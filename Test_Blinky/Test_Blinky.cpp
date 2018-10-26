// Connected Devices                                 | Dev Status
// ================================================================
// PORTD2   | LED (low = on)                         | Not Started
// SPI      | CAN ctrl (via library)                | Not Started
//          | * Also uses PCINT1 (PCI0, PORTB1).     |

#include "CANPnP_AVR.h"

CANPnP canNode;

void SetMode(CANPnP node, uint8_t len, uint64_t data);
uint8_t _currentMode = 0; // 0 = flash, 1 = on, 2+ = off

int main() {
	// Arduino bootloader uses UART. Disconnect so it can be reconnected in Serial.begin.
	// Write our own init() when we start using CANPnP bootloader...
	init();
	Serial.begin(9600);
	Serial.println("Start. Default mode set to flash");
	canNode.RegisterFunction(0x10, SetMode);
	Serial.println("Registered SetMode function at 0x10");
	for (;;) {
		delay(100);
		Serial.print("\t");
		Serial.print(canNode.GetUID(), HEX);
	}
}

void SetMode(CANPnP node, uint8_t len, uint64_t data) {
	// [D0:{0:flash;1:on;x:off]
	if (len != 1) {
		// send an error
		return;
	}
	_currentMode = data & 0xFF;
}