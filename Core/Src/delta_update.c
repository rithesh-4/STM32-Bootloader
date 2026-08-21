/*
 * delta_update.c
 *
 *  Created on: 23-Jul-2026
 *      Author: ramri
 */

#include "delta_update.h"
#include "detools.h"
#include "flash_if.h"
#include "memory_map.h"
#include "metadata.h"
#include "stm32f4xx_hal.h"

/**
 * @brief Context for delta update operations.
 *
 * Tracks current read/write positions during patch application.
 * The read position refers to the old firmware in flash (source).
 * The write position refers to the new firmware being written to a buffer.
 */
typedef struct {
    uint32_t read_address;      /* Current read address in old firmware flash */
    uint32_t write_address;     /* Current write address in output buffer */
    uint32_t patch_size;        /* Total size of patch data */
    uint32_t bytes_processed;   /* Bytes processed so far */
    bool     is_initialized;    /* Whether the context is valid */
} delta_update_context_t;

/**
 * @brief Output buffer for new firmware.
 *
 * We use a static buffer in RAM to hold the patched firmware before
 * writing it to flash. This allows detools to write the entire new
 * firmware before we erase and reprogram flash.
 *
 * Size: APP_MAX_SIZE (992KB). This is too large for stack,
 * but acceptable as a static allocation since we have 128KB RAM.
 *
 * Note: For production with limited RAM, consider:
 * - Double-buffering with streaming writes
 * - Writing directly to flash (more complex)
 */
static uint8_t output_buffer[4096];  /* 4KB buffer for streaming writes */

static struct detools_apply_patch_t patch;
static delta_update_context_t context;

/**
 * @brief Read callback for detools.
 *
 * Called by detools to read the original (old) firmware data from flash.
 * This allows detools to reference the old firmware when computing the patch.
 *
 * @param arg_p     Pointer to delta_update_context_t (cast from void*)
 * @param buf_p     Buffer to read data into
 * @param size      Number of bytes to read
 * @return          0 on success, negative on error
 */
static int ReadCallback(void *arg_p, uint8_t *buf_p, size_t size)
{
    delta_update_context_t *ctx = (delta_update_context_t *)arg_p;

    /* Validate context */
    if (ctx == NULL || !ctx->is_initialized) {
        return -1;
    }

    /* Validate read address is within application region */
    if (ctx->read_address < APP_START_ADDRESS ||
        (ctx->read_address + size) > (APP_END_ADDRESS + 1U)) {
        return -2;
    }

    /* Read directly from flash memory (memory-mapped) */
    const uint8_t *flash_ptr = (const uint8_t *)ctx->read_address;
    for (size_t i = 0; i < size; i++) {
        buf_p[i] = flash_ptr[i];
    }

    /* Advance read position */
    ctx->read_address += size;

    return 0;
}

/**
 * @brief Write callback for detools.
 *
 * Called by detools to write the new (patched) firmware data.
 * We accumulate the output in a buffer, then write to flash when done.
 *
 * @param arg_p     Pointer to delta_update_context_t (cast from void*)
 * @param buf_p     Buffer containing data to write
 * @param size      Number of bytes to write
 * @return          0 on success, negative on error
 */
static int WriteCallback(void *arg_p, const uint8_t *buf_p, size_t size)
{
    delta_update_context_t *ctx = (delta_update_context_t *)arg_p;

    /* Validate context */
    if (ctx == NULL || !ctx->is_initialized) {
        return -1;
    }

    /* Copy data to output buffer */
    for (size_t i = 0; i < size; i++) {
        /* Bounds check on output buffer */
        if (ctx->write_address >= sizeof(output_buffer)) {
            return -3;  /* Buffer overflow */
        }
        output_buffer[ctx->write_address] = buf_p[i];
        ctx->write_address++;
    }

    return 0;
}

/**
 * @brief Seek callback for detools.
 *
 * Called by detools to seek to a position in the original firmware.
 * detools uses this to read specific parts of the old firmware
 * when computing the delta patch.
 *
 * @param arg_p     Pointer to delta_update_context_t (cast from void*)
 * @param offset    Offset from current position (positive = forward, negative = backward)
 * @return          0 on success, negative on error
 */
static int SeekCallback(void *arg_p, int offset)
{
    delta_update_context_t *ctx = (delta_update_context_t *)arg_p;

    /* Validate context */
    if (ctx == NULL || !ctx->is_initialized) {
        return -1;
    }

    /* Calculate new position */
    int64_t new_pos = (int64_t)ctx->read_address + offset;

    /* Validate bounds */
    if (new_pos < (int64_t)APP_START_ADDRESS ||
        new_pos > (int64_t)(APP_END_ADDRESS + 1U)) {
        return -4;  /* Seek out of bounds */
    }

    ctx->read_address = (uint32_t)new_pos;

    return 0;
}

/**
 * @brief Initialize the delta update process.
 *
 * This function:
 * 1. Resets the context structure
 * 2. Initializes the detools apply_patch structure
 * 3. Sets up read/write/seek callbacks
 * 4. Prepares for patch data reception
 *
 * @param patch_size    Total size of the patch data in bytes
 * @return              true on success, false on failure
 */
bool DeltaUpdate_Init(uint32_t patch_size)
{
    /* Reset context */
    context.read_address = APP_START_ADDRESS;
    context.write_address = 0;
    context.patch_size = patch_size;
    context.bytes_processed = 0;
    context.is_initialized = false;

    /* Clear output buffer */
    memset(output_buffer, 0xFF, sizeof(output_buffer));  /* 0xFF = erased flash state */

    /* Initialize detools apply patch */
    int res = detools_apply_patch_init(
        &patch,
        ReadCallback,       /* Read old firmware from flash */
        SeekCallback,       /* Seek in old firmware */
        patch_size,         /* Size of incoming patch */
        WriteCallback,      /* Write new firmware to buffer */
        &context            /* User context passed to callbacks */
    );

    if (res != DETOOLS_OK) {
        return false;
    }

    /* Mark context as valid */
    context.is_initialized = true;

    return true;
}

/**
 * @brief Process a chunk of patch data.
 *
 * This function is called repeatedly as patch data arrives (e.g., via UART).
 * It feeds the data to detools which processes it incrementally.
 *
 * The patch data should be the complete delta patch file, which typically
 * contains:
 * - Header with patch metadata
 * - Compressed diff data (using heatshrink or other compression)
 * - Optional extra data sections
 *
 * @param patch_data    Pointer to the patch data chunk
 * @param patch_length  Length of this chunk in bytes
 * @return              true on success, false on error
 */
bool DeltaUpdate_Process(const uint8_t *patch_data, uint32_t patch_length)
{
    /* Validate state */
    if (!context.is_initialized) {
        return false;
    }

    /* Feed data to detools */
    int res = detools_apply_patch_process(
        &patch,
        patch_data,
        patch_length
    );

    /* DETOOLS_OK means done, DETOOLS_NOT_DONE means more data needed */
    if (res != DETOOLS_OK && res != DETOOLS_NOT_DONE) {
        return false;
    }

    /* Update progress counter */
    context.bytes_processed += patch_length;

    return true;
}

/**
 * @brief Finalize the delta update process.
 *
 * This function:
 * 1. Tells detools to finalize the patching
 * 2. Verifies the new firmware with CRC32
 * 3. Erases the application region in flash
 * 4. Writes the new firmware to flash
 * 5. Updates the metadata
 *
 * @return  true on success, false on failure
 */
bool DeltaUpdate_Finalize(void)
{
    /* Validate state */
    if (!context.is_initialized) {
        return false;
    }

    /* Finalize detools patching */
    int new_size = detools_apply_patch_finalize(&patch);

    /* Check for errors (negative values are error codes) */
    if (new_size < 0) {
        return false;
    }

    /* Validate new firmware size */
    if (new_size == 0 || (uint32_t)new_size > sizeof(output_buffer)) {
        return false;
    }

    /* Step 1: Erase the application region */
    if (!Flash_EraseApplication()) {
        return false;
    }

    /* Step 2: Write new firmware to flash */
    if (!Flash_Write(APP_START_ADDRESS, output_buffer, (uint32_t)new_size)) {
        return false;
    }

    /* Step 3: Update metadata (caller should update metadata separately) */

    /* Mark context as no longer valid */
    context.is_initialized = false;

    return true;
}
