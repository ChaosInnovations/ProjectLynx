/*
 Name:		CANPnP Bootloader.cpp
 Created:	10/24/2018 8:09:11 AM
 Author:	Thomas Boland
*/

// CANPnP Bootloader
// Allows firmware updates over CANPnP bus
// Super skeletal, non-extensible version of CANPnP AVR library

// Connected Devices                                 | Dev Status
// ================================================================
// SPI      | CAN ctrl                               | Not Started
//          | * Also uses PCINT1 (PCI0, PORTB1).     |

#include <avr/boot.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define ever (;;)

/* Watchdog settings */
#define WATCHDOG_OFF    (0)
#define WATCHDOG_16MS   (_BV(WDE))
#define WATCHDOG_32MS   (_BV(WDP0) | _BV(WDE))
#define WATCHDOG_64MS   (_BV(WDP1) | _BV(WDE))
#define WATCHDOG_125MS  (_BV(WDP1) | _BV(WDP0) | _BV(WDE))
#define WATCHDOG_250MS  (_BV(WDP2) | _BV(WDE))
#define WATCHDOG_500MS  (_BV(WDP2) | _BV(WDP0) | _BV(WDE))
#define WATCHDOG_1S     (_BV(WDP2) | _BV(WDP1) | _BV(WDE))
#define WATCHDOG_2S     (_BV(WDP2) | _BV(WDP1) | _BV(WDP0) | _BV(WDE))
#ifndef __AVR_ATmega8__
#define WATCHDOG_4S     (_BV(WDP3) | _BV(WDE))
#define WATCHDOG_8S     (_BV(WDP3) | _BV(WDP0) | _BV(WDE))
#endif

#define EEPROM_UIDH 0x00
#define EEPROM_UIDM 0x01
#define EEPROM_UIDL 0x02
#define EEPROM_CID 0x03
#define EEPROM_VIDH 0x04
#define EEPROM_VIDL 0x05
#define EEPROM_PIDH 0x06
#define EEPROM_PIDL 0x07
#define EEPROM_CLASS 0x08
#define EEPROM_VERH 0x09
#define EEPROM_VERL 0x0A

#define FUNCTION_STATUS 0x00
#define FUNCTION_RESET 0x01
#define FUNCTION_PAGESTART 0x02
#define FUNCTION_PAGEDATA 0x03

#define HEARTBEAT_PRIORITY 0x04

int main();
void SendMessage(uint8_t priority, bool heartbeat, uint8_t len, uint64_t data);
void GetMessage();
void WatchdogConfig(uint8_t x);
void WatchdogReset();
void(*StartApplication)(void) = 0x0000;
void DoPageWrite(uint16_t size);
void DoPageRead(uint16_t size);
bool isWaiting = false;
uint64_t _messageOut = 0;
uint64_t _messageIn = 0;
uint8_t _incomingPriority = 0;
uint8_t _incomingLen = 0;
uint16_t _firmwareVersion = 0;
uint32_t _nodeUID = 0;
uint16_t _nodePID = 0;
uint16_t _nodeVID = 0;
uint8_t _nodeCID = 0;
uint8_t _nodeClass = 0;
uint16_t address = 0;

int main() {
	// Was this a hard or soft reset?
	//  Skip bootloader if hard
	uint8_t mcusr_copy = MCUSR;
	if (mcusr_copy & (1 << EXTRF)) StartApplication();

	cli();

	// Figure out our address, version, etc
	_nodeUID = (eeprom_read_dword((uint32_t*)EEPROM_UIDH) >> 8) & 0xFFFFFF;
	_nodeCID = eeprom_read_byte((uint8_t*)EEPROM_CID);
	_nodeVID = eeprom_read_word((uint16_t*)EEPROM_VIDH);
	_nodePID = eeprom_read_word((uint16_t*)EEPROM_PIDH);
	_nodeClass = eeprom_read_byte((uint8_t*)EEPROM_CLASS);
	_firmwareVersion = eeprom_read_word((uint16_t*)EEPROM_VERH);
	// Initialize CAN MCP chip
	//  We don't need to know about other devices, so
	//  set masks to only accept our address, no heartbeats, and (any?) priority

	// Send out a heartbeat. We'll only have time to send one.
	// [CID][VIDH][VIDL][PIDH][PIDL][VERH][VERM][VERL]
	_messageOut = ((uint64_t)_firmwareVersion << 5) | ((uint64_t)_nodePID << 3) | ((uint64_t)_nodeVID << 1) | _nodeCID;
	SendMessage(HEARTBEAT_PRIORITY, true, 8, _messageOut);

	TCCR0A |= (1 << WGM12) | (1 << CS11) | (1 << CS10); // Setup boot timeout
	TCNT0 = 0; // initialize counter
	// This should be checked:
	OCR0A = 62499; // initialize compare value @ 250ms
	TIMSK0 |= (1 << OCIE1A); // enable compare interrupt
	sei();

	// Watchdog setup
	WatchdogConfig(WATCHDOG_500MS);
	WatchdogReset();

	uint16_t pageSize;

	for ever {
		isWaiting = true; // Allow timeout to switch to app if we don't get a message.
		// Wait for a CAN interrupt
		GetMessage();
		// Read message
		WatchdogReset();
		isWaiting = false;
		// Check function
			// 0x00: status
			// 0x01: restart, handled by default
			// 0x02: start firmware upload
			// 0x03: firmware data preface
			// 0x04: formware data, 0-7 bytes
			// 0x05: finish firmware upload
			// dflt: start app
		switch ((uint8_t)(_messageIn & 0xFF)) {
			case FUNCTION_STATUS:
				// Send our status
				// [S0][S1]
				// 
				// S0.7 S0.6 S0.5 S0.4 S0.3 S0.2 S0.1 S0.0
				// N/A  N/A  N/A  N/A  N/A  N/A  BOOT APP
				//
				// S1.7 S1.6 S1.5 S1.4 S1.3 S1.2 S1.1 S1.0
				// N/A  N/A  N/A  N/A  N/A  N/A  N/A  N/A
				_messageOut = 0b000000000000001 << 8 | FUNCTION_STATUS;
				SendMessage(_incomingPriority, false, 8, _messageOut);
				break;
			case FUNCTION_PAGESTART:
				// Start firmware upload
				// [D0:1=address][D2:3=size][D4=R/W]
				address = _messageIn >> 010 & 0xFFFF;
				pageSize = _messageIn >> 030 & 0xFFFF;
				if (_messageIn >> 050 & 1) {
					// Acknowledge (only if Write)
					SendMessage(_incomingPriority, false, 2, 1 << 8 | FUNCTION_PAGESTART);
					DoPageWrite(pageSize);
				}
				else {
					DoPageRead(pageSize);
				}
				break;
			case FUNCTION_PAGEDATA:
				// This shouldn't be called here. Reply with an error
				// Acknowledge
				SendMessage(_incomingPriority, false, 2, 1 << 8 | FUNCTION_PAGEDATA);
				break;
			default:
				// Try to start application. Will come back to bootloader anyways
				//  if it's a bad app and doesn't feed the WDT.
				// Acknowledge
				SendMessage(_incomingPriority, false, 2, 1 << 8 | FUNCTION_RESET);
				StartApplication();
				break;
		}
	}
}

void SendMessage(uint8_t priority, bool heartbeat, uint8_t len, uint64_t data) {

}

void GetMessage() {
	// Wait for PORTB1/PCIN1(PCI0) to trigger
	// for (;;) {}
	// Get msg from CAN ctrl
	_incomingPriority = 0;
	_incomingLen = 0;
	_messageIn = 0;
}

void WatchdogConfig(uint8_t x) {
	WDTCSR = _BV(WDCE) | _BV(WDE);
	WDTCSR = x;
}

void WatchdogReset() {
	asm("wdr");
}

ISR(TIMER0_COMPA_vect) {
	if (isWaiting) {
		// Bootloader is out of time and not busy. Feed WDT and start application.
		WatchdogReset();
		StartApplication();
	}
}

void DoPageWrite(uint16_t size) {
	uint16_t tempAddress = address;
	uint16_t tempWord;
	boot_page_erase(address);
	boot_spm_busy_wait();
	uint8_t lonelyByte = 0;
	bool hasLonelyByte = false;
	while (size) {
		isWaiting = true; // Allow timeout to switch to app if we don't get a message.
		GetMessage();
		WatchdogReset();
		isWaiting = false;
		if (!(uint8_t)(_messageIn & 0xFF) & FUNCTION_PAGEDATA) {
			// Ignore any other data.
			continue;
		}
		
		// Since we don't necessarily always have the same length of data
		// incoming, we use a separate byte to hold the big end of a word
		while (_incomingLen--) {
			_messageIn >>= 8;
			if (!hasLonelyByte) {
				hasLonelyByte = true;
				lonelyByte = _messageIn & 0xFF;
				continue;
			}
			tempWord = lonelyByte << 8 | _messageIn & 0xFF;
			hasLonelyByte = false;
			boot_page_fill(address, tempWord);
			size--;
			address += 2;
		}
	}
	boot_page_write(tempAddress);
	boot_spm_busy_wait();
	boot_rww_enable();
}

void DoPageRead(uint16_t size) {
	uint64_t data;
	uint8_t count = 0;
	do {
		data = (data >> 8 & 0x00FFFFFFFFFFFFFF) | ((uint64_t)pgm_read_byte_near(address++) << 56);
		count++;
		if (count == 7) {
			SendMessage(_incomingPriority, false, 8, data | FUNCTION_PAGEDATA);
			count = 0;
		}
		size--;

	} while (size);
	SendMessage(_incomingPriority, false, count + 1, (data >> (7 - count)) | FUNCTION_PAGEDATA);
}