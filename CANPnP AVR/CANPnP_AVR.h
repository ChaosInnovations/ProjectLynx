/*
 Name:		CANPnP_AVR.h
 Created:	10/24/2018 8:22:35 AM
 Author:	Thomas Boland
 Editor:	http://www.visualmicro.com
*/

#ifndef _CANPnP_AVR_h
#define _CANPnP_AVR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class CANPnP_AVR {
public:
	explicit CANPnP_AVR();
	void RegisterFunction(uint8_t funcNum, void(*funcPtr)(uint8_t, uint8_t*));
	void UnregisterFunction(uint8_t funcNum);
	bool FunctionRegistered(uint8_t funcNum);
	bool CallFunctionIfRegistered(uint8_t funcNum, uint8_t len, uint8_t data[9]);
private:
	void(*_functionTable[256])(uint8_t, uint8_t*);
};

#endif

