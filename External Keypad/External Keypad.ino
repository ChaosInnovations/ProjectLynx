/*
 Name:		External_Keypad.ino
 Created:	10/24/2018 8:10:06 AM
 Author:	Thomas Boland
*/

// Connected Devices                                 | Dev Status
// ================================================================
// PORTD0:7 | 4x4 keypad - cols 0:3, rows 4:7        | Done, but some keys (cols 0 and 1) won't work while UART in use
// I2C@0x?? | 16x2 LCD display (HD44780) via PCF8574 | Not Started
// I2C@0x?? | RFID/NFC reader                        | Not Started
// PORTB0   | Buzzer?                                | Not Started
// SPI      | CAN ctrl (via library?)                | Not Started
//          | * Also uses PCINT1 (PCI0, PORTB1).     |

#include "Keypad.cpp"
#include "CANPnP_AVR.cpp"

Keypad keypad;
//HD44780 lcd([I2C_ADDR], [pins,...]);
//Buzzer buzz; // Always at PORTB.0
CANPnP_AVR canNode;

void setup() {
	Serial.begin(9600);
	// Pass in our functions to canNode as funcNum, void(*funcPtr)(uint8_t, uint8_t*) pairs that
	// accept uint8_t[9] where uint8_t[0] = 0>=len<=8 and uint8_t[1:<=8] are data
	canNode.RegisterFunction(0x10, sampleCanNodeFunction);
}

void loop() {
	delay(100);
	keypad.UpdateStates();
	Serial.print(keypad.GetKeyStates(), HEX);
	if (keypad.IsKeyPressed(KEY_OK)) {
		Serial.println("\tKey OK Pressed");
	}
	else {
		Serial.println();
	}
}

void sampleCanNodeFunction(uint8_t len, uint8_t data[9]) {
	return;
}