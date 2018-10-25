/*
 Name:		CANPnP_AVR.cpp
 Created:	10/24/2018 8:22:35 AM
 Author:	Thomas Boland
 Editor:	http://www.visualmicro.com
*/

#include "CANPnP_AVR.h"

CANPnP_AVR::CANPnP_AVR() {
	// Read and initialize our UID, CID, VID, PID, and Class. Write if not set yet.
	// Initialize peripherals
	// Initialize device table?
	// Initialize function table
	// Register builtin function
}

void CANPnP_AVR::RegisterFunction(uint8_t funcNum, void(*funcPtr)(uint8_t, uint8_t*)) {
	_functionTable[funcNum] = funcPtr;
}

void CANPnP_AVR::UnregisterFunction(uint8_t funcNum) {
	_functionTable[funcNum] = 0;
}

bool CANPnP_AVR::FunctionRegistered(uint8_t funcNum) {
	return _functionTable[funcNum] != 0;
}

bool CANPnP_AVR::CallFunctionIfRegistered(uint8_t funcNum, uint8_t len, uint8_t data[9]) {
	if (!FunctionRegistered(funcNum)) {
		return false;
	}
	_functionTable[funcNum](len, data);
	return true;
}

