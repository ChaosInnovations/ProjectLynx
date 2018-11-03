// Connected Devices                                 | Dev Status
// ================================================================
// PORTD2   | LED (low = on)                         | Not Started
// SPI      | CAN ctrl (via library)                 | Not Started
//          | * Also uses PCINT1 (PCI0, PORTB1).     |

//#define NUM_CAN_FUNCTIONS 1

#include "CANPnP_AVR.h"

#define VERSION 0x0001

AVRPin canCSPin{ DDRB, PORTB, PINB, PORTB2 };
AVRPin canIntPin{ DDRB, PORTB, PINB, PORTB1 };

CANPnP canNode(canCSPin, canIntPin);

AVRPin ledPin{ DDRD, PORTD, PIND, PORTD2 };

void SetMode(CANPnP node, uint8_t len, uint64_t data);
uint8_t _currentMode = 0; // 0 = flash, 1 = on, 2+ = off

int main() {
	// Arduino bootloader uses UART. Disconnect so it can be reconnected in Serial.begin.
	// Write our own init() when we start using CANPnP bootloader...
	init();
	ledPin.ApplyDirection(AVRPIN_DIR_OUTPUT);
	Serial.begin(9600);
	Serial.println("Start. Default mode set to flash");
	Serial.print("Node UID:\t");
	Serial.println(canNode.GetUID(), HEX);
	if (canNode.GetVersion() != VERSION) {
		canNode.SetVersion(VERSION);
	}
	canNode.RegisterFunction(0x10, SetMode);
	Serial.println("Registered SetMode function at 0x10");
	for (;;) {
		delay(500);
		if (_currentMode == 0) {
			ledPin.ToggleOutput();
		}
		else {
			ledPin.SetPort(_currentMode & 1);
		}
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