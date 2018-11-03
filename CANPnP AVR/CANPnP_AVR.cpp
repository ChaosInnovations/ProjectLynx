/*
 Name:		CANPnP_AVR.cpp
 Created:	10/24/2018 8:22:35 AM
 Author:	Thomas Boland
 Editor:	http://www.visualmicro.com
*/

#include <EEPROM.h>
#include "mcp2515.h"
#include "CANPnP_AVR.h"

// SPI helper functions
// ====================
void spi_begin() {
	// init pins
	// set timing and mode
}

uint8_t spi_transfer(uint8_t value) {
	SPDR = value;
	//insert nop here
	while (0/*not sent yet*/) {}
	return SPDR;
}

#define spi_read() spi_transfer(0x00)

// AVRPin struct
// =============
void AVRPin::ApplyDirection(uint8_t val) {
	DDR = (DDR & ~(1 << Position)) | ((val & 1) << Position);
}

void AVRPin::SetPort(uint8_t val) {
	PORT = (PORT & ~(1 << Position)) | ((val & 1) << Position);
}

void AVRPin::ToggleOutput() {
	PIN |= 1 << Position;
}

uint8_t AVRPin::GetInputs() {
	return PIN & (1 << Position);
}

// CANPnP class
// ============
CANPnP::CANPnP(AVRPin cs, AVRPin pcInt) {
	// Flow
	// ========================
	// set CS
	_CS = cs;
	// Deselect
	_CS.ApplyDirection(AVRPIN_DIR_OUTPUT);
	_CS.SetPort(AVRPIN_HIGH);
	// setup PCINT
	// Open SPI
	spi_begin();
	// Need idmodeset, speedset, clockset
	// init ourselves with (idmodeset, speedset, clockset):
		// reset
		//		Send reset command
	mcp2515_reset();
		// enter config mode
	mcp2515_modifyRegister(MCP2515_REG_CANCTRL, MCP2515_MASK_CANCTRL_MODE, MCP2515_MODE_CONFIG);
		// set rate based on speedset and clockset
		// Whatever these do:
		//		mcp2515_initCANBuffers();
		//			Set filters & masks to 0 (& disable?)
		//			Clear & deactivate TX buffers
		//			Clear RX buffers
		//		mcp2515_setRegister(MCP_CANINTE, MCP_RX0IF | MCP_RX1IF);
		//			Send write command
		//			Send address
		//			Send value
		//		//Sets BF pins as GPO
		//		mcp2515_setRegister(MCP_BFPCTRL, MCP_BxBFS_MASK | MCP_BxBFE_MASK);
		//		//Sets RTS pins as GPI
		//		mcp2515_setRegister(MCP_TXRTSCTRL, 0x00);
		// Set mode:
		//		switch(canIDMode) {
		//			case (MCP_ANY):
		//				mcp2515_modifyRegister(MCP_RXB0CTRL, MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK, MCP_RXB_RX_ANY | MCP_RXB_BUKT_MASK);
		//					Send bitmod command
		//					Send address
		//					Send mask
		//					Send value
		//				mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK, MCP_RXB_RX_ANY);
		//				break;
		//			// The followingn two functions of the MCP2515 do not work, there is a bug in the silicon.
		//			case (MCP_STD):
		//				mcp2515_modifyRegister(MCP_RXB0CTRL, MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK, MCP_RXB_RX_STD | MCP_RXB_BUKT_MASK);
		//				mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK, MCP_RXB_RX_STD);
		//				break;
		//			case (MCP_EXT):
		//				mcp2515_modifyRegister(MCP_RXB0CTRL, MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK, MCP_RXB_RX_EXT | MCP_RXB_BUKT_MASK);
		//				mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK, MCP_RXB_RX_EXT);
		//				break;
		//			case (MCP_STDEXT):
		//				mcp2515_modifyRegister(MCP_RXB0CTRL, MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK, MCP_RXB_RX_STDEXT | MCP_RXB_BUKT_MASK);
		//				mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK, MCP_RXB_RX_STDEXT);
		//				break;
		//			default: // bad mode
		//				break;
		// leave config mode (go back to whatever we had before?)
	// =====================

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
	RegisterFunction(CANPnP_FUNCTION_SETCID, CANPnP::SetCID);
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
	// CID is auto-added
	SendMessage(true, ((uint64_t)_device_class << 6) | ((uint64_t)_device_version << 4) | ((uint32_t)_device_pid << 2) | _device_vid);
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
	node.SendMessage(node._statusFlags);
}

#define ever (;;)
void CANPnP::Reset(CANPnP node, uint8_t len, uint64_t data) {
	// Activate watchdog if not already active
	// Shorten watchdog time
	// Send acknowledgement
	node.SendMessage(CANPnP_MSG_ACK);
	// for(;;){} // Infinite loop to timeout and force reset
	for ever{}
}

void CANPnP::FirmwareOops(CANPnP node, uint8_t len, uint64_t data) {
	// This should be called during bootload, not here.
	// Send acknowledgement with an error
	node.SendMessage(CANPnP_MSG_BOOT_ONLY);
	// Do nothing else - leave it to the other node to reset us properly
}

void CANPnP::SetCID(CANPnP node, uint8_t len, uint64_t data) {
	if (len == 0) {
		// Bad, return error
		node.SendMessage(CANPnP_MSG_MISSING_ARGS);
		return;
	}
	EEPROM.write(CANPnP_EEPROM_CID, DataByte(data, 0));
	node.SendMessage(CANPnP_MSG_ACK);
}

uint8_t CANPnP::DataByte(uint64_t data, uint8_t position) {
	if (position > 7) {
		return 0x00;
	}
	return (data >> 4 * position) & 0xFF;
}

void CANPnP::SendMessage(uint8_t priority, bool heartbeat, uint8_t function, uint64_t data) {
	int addr = (((uint32_t)priority << 25) | ((uint32_t)heartbeat << 24) | _device_uid) & 0x1FFFFFFF;

	// Flow
	// ==================
	// Wait for free TX buffer
	// Write to TX buffer
	// Request to send
	// Wait to see that request sent?
	// ==================
}

// Various overflows for the SendMessage
void CANPnP::SendMessage(bool heartbeat, uint64_t data) {
	SendMessage(heartbeat ? CANPnP_HEARTBEAT_PRIORITY : _incomingPriority, heartbeat, heartbeat ? _device_cid : _incomingFunction, data);
}
void CANPnP::SendMessage(uint8_t priority, uint8_t function, uint64_t data) { SendMessage(priority, false, function, data); }
void CANPnP::SendMessage(uint8_t function, uint64_t data) { SendMessage(_incomingPriority, false, function, data); }
void CANPnP::SendMessage(uint64_t data) { SendMessage(_incomingPriority, false, _incomingFunction, data); }

// low-level functions
void CANPnP::mcp2515_reset() {
	_CS.SetPort(AVRPIN_LOW);
	spi_transfer(MCP2515_CMD_RESET);
	_CS.SetPort(AVRPIN_HIGH);
}

uint8_t CANPnP::mcp2515_readRegister(uint8_t address) {
	uint8_t result;

	_CS.SetPort(AVRPIN_LOW);
	spi_transfer(MCP2515_CMD_READ);
	spi_transfer(address);
	result = spi_read();
	_CS.SetPort(AVRPIN_HIGH);

	return result;
}

void CANPnP::mcp2515_writeRegister(uint8_t address, uint8_t value) {
	_CS.SetPort(AVRPIN_LOW);
	spi_transfer(MCP2515_CMD_WRITE);
	spi_transfer(address);
	spi_transfer(value);
	_CS.SetPort(AVRPIN_HIGH);
}

void CANPnP::mcp2515_modifyRegister(uint8_t address, uint8_t mask, uint8_t value) {
	_CS.SetPort(AVRPIN_LOW);
	spi_transfer(MCP2515_CMD_BITMOD);
	spi_transfer(address);
	spi_transfer(mask);
	spi_transfer(value);
}