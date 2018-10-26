// Connected Devices                                 | Dev Status
// ================================================================
// PORTD2   | LED (low = on)                         | Not Started
// SPI      | CAN ctrl (via library)                | Not Started
//          | * Also uses PCINT1 (PCI0, PORTB1).     |

#include "CANPnP_AVR.h"

CANPnP canNode;

void SetMode(CANPnP node, uint8_t len, uint64_t data);
uint8_t _currentMode = 0; // 0 = flash, 1 = on, 2+ = off
uint8_t state = 0;

int main() {
	// Arduino bootloader uses UART. Disconnect so it can be reconnected in Serial.begin.
	// Write our own init() when we start using CANPnP bootloader...
	init();
	DDRD = 0xFF;
	Serial.begin(9600);
	Serial.println("Start. Default mode set to flash");
	canNode.RegisterFunction(0x10, SetMode);
	Serial.println("Registered SetMode function at 0x10");
	for (;;) {
		delay(500);
		if (_currentMode == 0) {
			state = ~state;
		}
		else {
			state = _currentMode & 1;
		}
		PORTD = PORTD & ~(1 << PORTD2) | ((state & 1) << PORTD2);
	}
}

void SetMode(CANPnP node, uint8_t len, uint64_t data) {
	// [D0:{0:flash;1:on;x:off]
	if (len != 1) {
		// send an error
		return;
	}
	_currentMode = data & 0xFF;
	// send acknowledgement
}