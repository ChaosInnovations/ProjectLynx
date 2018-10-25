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

#define CANPnP_EEPROM_UIDH 0x00
#define CANPnP_EEPROM_UIDM 0x01
#define CANPnP_EEPROM_UIDL 0x02
#define CANPnP_EEPROM_CID 0x03
#define CANPnP_EEPROM_VIDH 0x04
#define CANPnP_EEPROM_VIDL 0x05
#define CANPnP_EEPROM_PIDH 0x06
#define CANPnP_EEPROM_PIDL 0x07
#define CANPnP_EEPROM_CLASS 0x08
#define CANPnP_HEARTBEAT_PRIORITY 0x04

#define CANPnP_FUNCTION_STATUS 0x00
#define CANPnP_FUNCTION_RESET 0x01
#define CANPnP_FUNCTION_FIRMWARESTART 0x02
#define CANPnP_FUNCTION_FIRMWAREPREFACE 0x03
#define CANPnP_FUNCTION_FIRMWAREPAYLOAD 0x04
#define CANPnP_FUNCTION_FIRMWAREEND 0x05

#define CANPnP_STATUS_CLEAR_FLAGS 0x01

class CANPnP {
public:
	explicit CANPnP();
	bool RegisterFunction(uint8_t funcNum, void(*funcPtr)(CANPnP node, uint8_t len, uint8_t*data));
	//bool RegisterFunction(uint8_t funcNum, void(CANPnP::*funcPtr)(uint8_t len, uint8_t*data));
	bool UnregisterFunction(uint8_t funcNum);
	bool FunctionRegistered(uint8_t funcNum);
	bool CallFunctionIfRegistered(uint8_t funcNum, uint8_t len, uint8_t data[9]);
	uint32_t GetUID();
	uint8_t GetCID();
	uint16_t GetVID();
	uint16_t GetPID();
	uint8_t GetClass();
private:
	void(*_functionTable[256])(CANPnP, uint8_t, uint8_t*);
	uint32_t _device_uid;
	uint8_t _device_cid;
	uint16_t _device_vid;
	uint16_t _device_pid;
	uint8_t _device_class;
	uint32_t MakeAddress(uint8_t priority, bool heartbeat);
	void SendHeartbeat();
	uint64_t _statusFlags;
	// Default functions
	static void GetStatus(CANPnP node, uint8_t len, uint8_t data[8]);
	static void Reset(CANPnP node, uint8_t len, uint8_t data[8]);
	static void FirmwareOops(CANPnP node, uint8_t len, uint8_t data[8]);
};

#endif
