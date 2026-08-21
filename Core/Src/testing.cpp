/*
 * testing.cpp
 *
 *  Created on: 28-Jul-2026
 *      Author: ramri
 */

#include <stdint.h>

class Vehicle{

public :

	int speed;

	void start_code();

	void stop_code();

	uint8_t speed;

};


class car : public Vehicle{

public:
	void AC_feature();

	uint8_t AC_Temp;

};

int main(){

	car C;

	C.AC_Temp();

}
