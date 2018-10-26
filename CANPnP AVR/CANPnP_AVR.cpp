/*
 Name:		CANPnP_AVR.cpp
 Created:	10/24/2018 8:22:35 AM
 Author:	Thomas Boland
 Editor:	http://www.visualmicro.com
*/

#include <EEPROM.h>
#include "CANPnP_AVR.h"

CANPnP::CANPnP() {
	// Read and initialize our UID, CID, VID, PID, and Class. Write if not set yet.
	_device_uid = ((long)EEPROM.read(CANPnP_EEPROM_UIDH) << 16) | (EEPROM.read(CANPnP_EEPROM_UIDM) << 8) | EEPROM.read(CANPnP_EEPROM_UIDL);
	_device_cid = EEPROM.read(CANPnP_EEPROM_CID);
	_device_vid = (EEPROM.read(CANPnP_EEPROM_VIDH) << 8) | EEPROM.read(CANPnP_EEPROM_VIDL);
	_device_pid = (EEPROM.read(CANPnP_EEPROM_PIDH) << 8) | EEPROM.read(CANPnP_EEPROM_PIDL);
	_device_class = EEPROM.read(CANPnP_EEPROM_CLASS);
	_device_version = (EEPROM.read(CANPnP_EEPROM_VERH) << 8) | EEPROM.read(CANPnP_EEPROM_VERL);
	// Initialize peripherals
	// Initialize device table?
	// Register builtin functions
	RegisterFunction(CANPnP_FUNCTION_STATUS, CANPnP::GetStatus);
	RegisterFunction(CANPnP_FUNCTION_RESET, CANPnP::Reset);
	// These functions shouldn't be called here. Respond with an error/warning
	RegisterFunction(CANPnP_FUNCTION_PAGESTART, CANPnP::FirmwareOops);
	RegisterFunction(CANPnP_FUNCTION_PAGEDATA, CANPnP::FirmwareOops);
}

uint16_t CANPnP::GetVersion() {
	return _device_version;
}

void CANPnP::SetVersion(uint16_t version) {
	_device_version = version;
	EEPROM.write(CANPnP_EEPROM_VERH, _device_version >> 8);
	EEPROM.write(CANPnP_EEPROM_VERL, _device_version);
}

bool CANPnP::RegisterFunction(uint8_t funcNum, void(*funcPtr)(CANPnP node, uint8_t len, uint64_t)) {
	if (FunctionRegistered(funcNum)) {
		return false;
	}
	_functionTable[funcNum] = funcPtr;
	return true;
}

bool CANPnP::UnregisterFunction(uint8_t funcNum) {
	if (FunctionRegistered(funcNum)) {
		return false;
	}
	_functionTable[funcNum] = 0;
	return true;
}

bool CANPnP::FunctionRegistered(uint8_t funcNum) {
	return _functionTable[funcNum] != 0;
}

bool CANPnP::CallFunctionIfRegistered(uint8_t funcNum, uint8_t len, uint64_t data) {
	if (!FunctionRegistered(funcNum)) {
		return false;
	}
	_functionTable[funcNum](*this, len, data);
	return true;
}

void CANPnP::SendHeartbeat() {
	SendMessage(true, ((uint64_t)_device_version << 4) | ((uint32_t)_device_pid << 2) | _device_vid);
}

uint32_t CANPnP::GetUID() {
	return _device_uid;
}

uint8_t CANPnP::GetCID() {
	return _device_cid;
}

uint16_t CANPnP::GetVID() {
	return _device_vid;
}

uint16_t CANPnP::GetPID() {
	return _device_pid;
}

uint8_t CANPnP::GetClass() {
	return _device_class;
}

// Default functions (application mode)
void CANPnP::GetStatus(CANPnP node, uint8_t len, uint64_t data) {
	if (len > 0 && DataByte(data, 0) == CANPnP_STATUS_CLEAR_FLAGS) {
		node._statusFlags = 0;
	}
	// reply with lowest 7 bytes of _statusFlags
}

void CANPnP::Reset(CANPnP node, uint8_t len, uint64_t data) {
	// Activate watchdog if not already active
	// Shorten watchdog time
	// Send acknowledgement
	// for(;;){} // Infinite loop to timeout and force reset
}

void CANPnP::FirmwareOops(CANPnP node, uint8_t len, uint64_t data) {
	// This should be called during bootload, not here.
	// Send acknowledgement with an error
	// Do nothing else - leave it to the other node to reset us properly
}

uint8_t CANPnP::DataByte(uint64_t data, uint8_t position) {
	if (position > 7) {
		return 0x00;
	}
	return (data >> 4 * position) & 0xFF;
}

void CANPnP::SendMessage(uint8_t priority, bool heartbeat, uint8_t function, uint64_t data) {
	int addr = (((uint32_t)priority << 25) | ((uint32_t)heartbeat << 24) | _device_uid) & 0x1FFFFFFF;
}

// Various overflows for the SendMessage
void CANPnP::SendMessage(bool heartbeat, uint64_t data) {
	SendMessage(heartbeat ? CANPnP_HEARTBEAT_PRIORITY : _incomingPriority, heartbeat, heartbeat ? _device_cid : _incomingFunction, data);
}
void CANPnP::SendMessage(uint8_t priority, uint8_t function, uint64_t data) { SendMessage(priority, false, function, data); }
void CANPnP::SendMessage(uint8_t function, uint64_t data) { SendMessage(_incomingPriority, false, function, data); }
void CANPnP::SendMessage(uint64_t data) { SendMessage(_incomingPriority, false, _incomingFunction, data); }