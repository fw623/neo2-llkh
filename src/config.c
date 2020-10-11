#include "config.h"

bool debugWindow = false;
bool capsLockEnabled = false;
bool shiftLockEnabled = false;
bool qwertzForShortcuts = false;
bool supportLevels5and6 = false;

Mapping dfkCtrlL = {
	.key = { VK_LCONTROL, 29 },
	.hold = { VK_LWIN, 91, true },
	.tap = NULL,
	.state = RELEASED,
	.changed = 0,
	.n = NULL
};

Mapping dfkWinL = {
	.key = { VK_LWIN, 91 },
	.hold = { VK_LMENU, 56, false },
	.tap = NULL,
	.state = RELEASED,
	.changed = 0,
	.n = &dfkCtrlL
};

Mapping dfkAltL = {
	.key = { VK_LMENU, 56 },
	.hold = { VK_LCONTROL, 29, false },
	.tap = &(Tap){ { VK_ESCAPE, 1, false }, NULL },
	.state = RELEASED,
	.changed = 0,
	.n = &dfkWinL
};

Mapping dfkAltGr = {
	.key = { VK_RMENU, 56 },
	.hold = { VK_RMENU, 56, true },
	.tap = &(Tap){ { VK_RETURN, 28, false }, NULL },
	.state = RELEASED,
	.changed = 0,
	.n = &dfkAltL
};

Mapping dfkOe = {
	.key = { 0xDE, 40 },
	.hold = { 0xBF, 43, false },
	.tap = &(Tap){ { 0xDE, 40, false }, NULL },
	.state = RELEASED,
	.changed = 0,
	.n = &dfkAltGr
};

Mapping dfkLess = {
	.key = { 0xE2, 86 },
	.hold = { 0xE2, 86, false },
	.tap = &(Tap){ { VK_DELETE, 83, true }, NULL },
	.state = RELEASED,
	.changed = 0,
	.n = &dfkOe
};

Mapping dfkCaps = {
	.key = { VK_CAPITAL, 58 },
	.hold = { VK_CAPITAL, 58, false },
	.tap = &(Tap){ { VK_BACK, 14, false }, NULL },
	.state = RELEASED,
	.changed = 0,
	.n = &dfkLess
};

DfkConfig dfkConfig = {
	.tap_millis = DEFAULT_TAP_MILLIS,
	.double_tap_millis = DEFAULT_DOUBLE_TAP_MILLIS,
	.m = &dfkCaps
};

SystemKey systemKey = {
	.lCtrl = { VK_LCONTROL, 29 },
	.lWin = { VK_LWIN, 91 },
	.lAlt = { VK_LMENU, 56 },
	.rAlt = { VK_RMENU, 56 },
	.rWin = { VK_RWIN, 92 },
	.rCtrl = { VK_RCONTROL, 29 }
};

ModConfig modConfig = {
	.shift = {
		.lock = NULL,
		.left = { VK_LSHIFT, 42 },
		.right = { VK_RSHIFT, 54 },
		.bothLock = true
	},
	.mod3 = {
		.lock = NULL,
		.left = { VK_CAPITAL, 58 },
		.right = { VK_RMENU, 56 },
		.bothLock = false
	},
	.mod4 = {
		.lock = &(InputKey){ VK_RCONTROL, 29 },
		.left = { 0xE2, 86 }, // > key
		.right = { 0xBF, 43 }, // # key
		.bothLock = false
	}
};
