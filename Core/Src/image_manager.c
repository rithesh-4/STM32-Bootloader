/*
 * image_manager.c
 *
 *  Created on: 22-Jul-2026
 *      Author: ramri
 */

#include "image_manager.h"
#include "metadata.h"

static bool Image_IsStackPointerValid(uint32_t stackPointer);
static bool Image_IsResetHandlerValid(uint32_t resetHandler);



static bool Image_IsStackPointerValid(uint32_t stackPointer)
{
    return ((stackPointer >= RAM_START_ADDRESS) &&
            (stackPointer <= RAM_END_ADDRESS));
}

static bool Image_IsResetHandlerValid(uint32_t resetHandler)
{
    return ((resetHandler >= APP_START_ADDRESS) &&
            (resetHandler <= APP_END_ADDRESS));
}



bool Image_IsBootable(void)
{
    uint32_t appStack;
    uint32_t appResetHandler;

    if (!Metadata_IsValid())
    {
        return false;
    }

    appStack = *(uint32_t *)APP_START_ADDRESS;
    appResetHandler = *(uint32_t *)(APP_START_ADDRESS + 4U);

    if (!Image_IsStackPointerValid(appStack))
    {
        return false;
    }

    if (!Image_IsResetHandlerValid(appResetHandler))
    {
        return false;
    }

    return true;
}
