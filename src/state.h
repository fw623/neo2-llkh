typedef struct NeoModState {
	bool leftIsPressed,
		rightIsPressed,
		isLocked;
} NeoModState;

typedef struct ModKeyStates {
	NeoModState shift,
		mod3,
		mod4;
	bool lCtrlIsPressed,
		rCtrlIsPressed,
		lAltIsPressed,
		rAltIsPressed,
		lMetaIsPressed,
		rMetaIsPressed;
} ModKeyStates;

/* ============ VALUES ============ */

ModKeyStates modKeyStates = {
	.shift = { false, false, false },
	.mod3 = { false, false, false },
	.mod4 = { false, false, false },
	.lCtrlIsPressed = false,
	.rCtrlIsPressed = false,
	.lAltIsPressed = false,
	.rAltIsPressed = false,
	.lMetaIsPressed = false,
	.rMetaIsPressed = false,
};
