// 
// 
// 

#include "Keypad.h"

using namespace std;

// Uses PORTD0:7
// 4x4 MUX Keypad
class Keypad
{
public:
	uint16_t keyStates = 0;

	Keypad() {
		// Configure IO
		DDRD = 0x0F;
		PORTD = 0xF0;
	}

	void UpdateStates() {
		for (size_t i = 0; i < 4; i++)
		{
			// Turn off all columns and turn this one on
			PORTD = (PORTD & 0xF0) | ~(1 << i);
			// Read rows into keypadState
			delay(1);
			keyStates = (keyStates & 0xFFFF & ~(0xF << i * 4)) | ((~PIND >> 4 & 0xF) << i * 4);
		}
	}

	bool IsKeyPressed(int key) {
		return keyPressed(keyStates, key);
	}
};

