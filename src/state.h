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
