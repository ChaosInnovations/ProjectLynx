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

#include <EEPROM.h>

#define CANPnP_EEPROM_UIDH 0x00
#define CANPnP_EEPROM_UIDM 0x01
#define CANPnP_EEPROM_UIDL 0x02
#define CANPnP_EEPROM_CID 0x03
#define CANPnP_EEPROM_VIDH 0x04
#define CANPnP_EEPROM_VIDL 0x05
#define CANPnP_EEPROM_PIDH 0x06
#define CANPnP_EEPROM_PIDL 0x07
#define CANPnP_EEPROM_CLASS 0x08
#define CANPnP_EEPROM_VERH 0x09
#define CANPnP_EEPROM_VERL 0x0A
#define CANPnP_HEARTBEAT_PRIORITY 0x04

#define CANPnP_FUNCTION_STATUS 0x00
#define CANPnP_FUNCTION_RESET 0x01
#define CANPnP_FUNCTION_PAGESTART 0x02
#define CANPnP_FUNCTION_PAGEDATA 0x03
#define CANPnP_FUNCTION_SETCID 0x04

#define CANPnP_STATUS_CLEAR_FLAGS 0x01

#define CANPnP_MSG_ACK          0b00000001
#define CANPnP_MSG_BOOT_ONLY    0b00000010
#define CANPnP_MSG_APP_ONLY     0b00000100 // Don't really need this because we aren't the bootloader
#define CANPnP_MSG_MISSING_ARGS 0b00001000

#define AVRPIN_DIR_OUTPUT 1
#define AVRPIN_DIR_INPUT 0
#define AVRPIN_HIGH 1
#define AVRPIN_LOW 0
#define AVRPIN_PULLUP_ENABLE 1
#define AVRPIN_PULLUP_DISABLE 0


//#ifndef NUM_CAN_FUNCTIONS
//#define NUM_CAN_FUNCTIONS 0
//#endif

struct AVRPin {
	uint8_t DDR;  // Direction register
	uint8_t PORT; // Port register
	uint8_t PIN;  // Pin register
	uint8_t Position; // Select our pin
	void ApplyDirection(uint8_t val);
	void SetPort(uint8_t val);
	void ToggleOutput();
	uint8_t GetInputs();
};

class CANPnP {
public:
	explicit CANPnP(AVRPin cs, AVRPin pcInt);
	bool RegisterFunction(uint8_t funcNum, void(*funcPtr)(CANPnP node, uint8_t len, uint64_t));
	bool UnregisterFunction(uint8_t funcNum);
	bool FunctionRegistered(uint8_t funcNum);
	bool CallFunctionIfRegistered(uint8_t funcNum, uint8_t len, uint64_t data);
	uint32_t GetUID();
	uint8_t GetCID();
	uint16_t GetVID();
	uint16_t GetPID();
	uint8_t GetClass();
	uint16_t GetVersion();
	void SetVersion(uint16_t version);
	void SendMessage(bool heartbeat, uint64_t data);
	void SendMessage(uint8_t priority, uint8_t function, uint64_t data);
	void SendMessage(uint8_t function, uint64_t data);
	void SendMessage(uint64_t data);
	void SendMessage(uint8_t priority, bool heartbeat, uint8_t function, uint64_t data);
private:
	// Function Table, up to 256 functions (0:5 reserved)
	// This takes up lots of space - can it be any smaller?
    void(*_functionTable[256])(CANPnP, uint8_t, uint64_t);
	//void(*_functionTable[NUM_CAN_FUNCTIONS + 4])(CANPnP, uint8_t, uint64_t);
	uint8_t _incomingPriority;
	uint8_t _incomingFunction;
	uint32_t _device_uid;
	uint8_t _device_cid;
	uint16_t _device_vid;
	uint16_t _device_pid;
	uint8_t _device_class;
	uint16_t _device_version;
	void SendHeartbeat();
	uint64_t _statusFlags;
	// low-level SPI/register functions
	AVRPin _CS;
	AVRPin _PCINT;
	void mcp2515_reset();
	uint8_t mcp2515_readRegister(uint8_t address);
	void mcp2515_writeRegister(uint8_t address, uint8_t value);
	void mcp2515_modifyRegister(uint8_t address, uint8_t mask, uint8_t value);
	// Default functions
	static void GetStatus(CANPnP node, uint8_t len, uint64_t data);
	static void Reset(CANPnP node, uint8_t len, uint64_t data);
	static void FirmwareOops(CANPnP node, uint8_t len, uint64_t data);
	static void SetCID(CANPnP node, uint8_t len, uint64_t data);
	static uint8_t DataByte(uint64_t data, uint8_t position);
};

#endif
