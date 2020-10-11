#include "states.h"

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
