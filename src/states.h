#ifndef STATES_H_
#define STATES_H_

#include <stdbool.h>
#include "config.h"

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

extern ModStates modStates;
extern bool shiftLockActive;
extern bool capsLockActive;
extern bool bypassMode; // true if no mapping should be done
extern LastKey lastKey;

#endif /* STATES_H_ */
