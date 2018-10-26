// Connected Devices                                 | Dev Status
// ================================================================
// PORTD0:7 | 4x4 keypad - cols 0:3, rows 4:7        | Done, but some keys (cols 0 and 1) won't work while UART in use
// I2C@0x?? | 16x2 LCD display (HD44780) via PCF8574 | Not Started
// I2C@0x?? | RFID/NFC reader                        | Not Started
// PORTB0   | Buzzer?                                | Not Started
// SPI      | CAN ctrl (via library?)                | Not Started
//          | * Also uses PCINT1 (PCI0, PORTB1).     |

#include "Keypad.h"
#include "CANPnP_AVR.h"

Keypad keypad;
//HD44780 lcd([I2C_ADDR], [pins,...]);
//Buzzer buzz; // Always at PORTB.0
CANPnP canNode;

void sampleCanNodeFunction(CANPnP node, uint8_t len, uint64_t data);

int main() {
	// Arduino bootloader uses UART. Disconnect so it can be reconnected in Serial.begin.
	// Write our own init() when we start using CANPnP bootloader...
	init();
	Serial.begin(9600);
	Serial.println("start");
	canNode.RegisterFunction(0x10, sampleCanNodeFunction);
	for (;;) {
		delay(100);
		keypad.UpdateStates();
		Serial.print(keypad.GetKeyStates(), HEX);
		Serial.print("\t");
		Serial.print(canNode.GetUID(), HEX);
		if (keypad.IsKeyPressed(KEY_OK)) {
			Serial.println("\tKey OK Pressed");
		}
		else {
			Serial.println();
		}
	}
}

void sampleCanNodeFunction(CANPnP node, uint8_t len, uint64_t data) {
	return;
}