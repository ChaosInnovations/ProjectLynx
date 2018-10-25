#include "Keypad.h"

//using namespace std;

// Uses PORTD0:7
// 4x4 MUX Keypad
Keypad::Keypad() {
	// Configure IO
	DDRD = 0x0F;
	PORTD = 0xF0;
	_keyStates = 0;
}

void Keypad::UpdateStates() {
	for (size_t i = 0; i < 4; i++)
	{
		// Turn off all columns and turn this one on
		PORTD = (PORTD & 0xF0) | ~(1 << i);
		// Read rows into keypadState
		delay(1);
		_keyStates = (_keyStates & 0xFFFF & ~(0xF << i * 4)) | ((~PIND >> 4 & 0xF) << i * 4);
	}
}

bool Keypad::IsKeyPressed(int key) {
	return keyPressed(_keyStates, key);
}

uint16_t Keypad::GetKeyStates() {
	return _keyStates;
}
