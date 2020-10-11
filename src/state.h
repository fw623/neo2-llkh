#ifndef STATE_H_
#define STATE_H_

typedef struct NeoModState {
	bool leftIsPressed,
		rightIsPressed,
		isLocked;
} NeoModState;

typedef struct ModStates {
	NeoModState shift,
		mod3,
		mod4;
	bool lCtrlIsPressed,
		lWinIsPressed,
		lAltIsPressed,
		rAltIsPressed,
		rWinIsPressed,
		rCtrlIsPressed;
} ModStates;

typedef struct LastKey {
	InputKey key;
	DWORD time;
} LastKey;

/* ============ VALUES ============ */

ModStates modStates = {
	.shift = { false, false, false },
	.mod3 = { false, false, false },
	.mod4 = { false, false, false },
	.lCtrlIsPressed = false,
	.lWinIsPressed = false,
	.lAltIsPressed = false,
	.rAltIsPressed = false,
	.rWinIsPressed = false,
	.rCtrlIsPressed = false
};

bool shiftLockActive = false;
bool capsLockActive = false;
bool bypassMode = false; // true if no mapping should be done
LastKey lastKey;

#endif /* STATE_H_ */
