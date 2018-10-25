#ifndef _KEYPAD_h
#define _KEYPAD_h
#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

// Key masks
//  1 2 3 START
//  4 5 6 LOCK
//  7 8 9 END
//  X < 0 OK
#define KEY_1      0x0001
#define KEY_2      0x0010
#define KEY_3      0x0100
#define KEY_4      0x0002
#define KEY_5      0x0020
#define KEY_6      0x0200
#define KEY_7      0x0004
#define KEY_8      0x0040
#define KEY_9      0x0400
#define KEY_0      0x0800
#define KEY_OK     0x8000
#define KEY_BACK   0x0080
#define KEY_CANCEL 0x0008
#define KEY_START  0x1000
#define KEY_LOCK   0x2000
#define KEY_END    0x4000

#define keyPressed(state, key) (state&key)==key

class Keypad
{
public:
	uint16_t keyStates = 0;

	explicit Keypad();

	void UpdateStates();

	bool IsKeyPressed(int key);
	uint16_t GetKeyStates();

private:
	uint16_t _keyStates;
};

#endif