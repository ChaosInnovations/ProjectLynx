/*
 Name:		External_Keypad.ino
 Created:	10/24/2018 8:10:06 AM
 Author:	Thomas Boland
*/

// Connected Devices                                         | Dev Status
// ======================================================================
// PORTD0:7 | 4x4 keypad - cols 0:3, rows 4:7                | Done, but some keys won't work while UART in use
// I2C@0x27 | 16x2 LCD display (P/N) via 8-bit GPIO expander | Not Started
// I2C@0x?? | RFID/NFC reader                                | Not Started
// PORTB0   | Buzzer?                                        | Not Started
// SPI      | CAN ctrl (via library?)                        | Not Started
//          | * Also uses PCINT1 (PCI0, PORTB1).             |

#include "Keypad.cpp"

Keypad keypad;

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600);
}

// the loop function runs over and over again until power down or reset
void loop() {
	delay(100);
	keypad.UpdateStates();
	Serial.print(keypad.keyStates, HEX);
	if (keypad.IsKeyPressed(KEY_OK)) {
		Serial.println("\tKey OK Pressed");
	}
	else {
		Serial.println();
	}
}