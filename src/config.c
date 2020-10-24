#include "config.h"

bool debugWindow = false;
bool capsLockEnabled = false;
bool shiftLockEnabled = false;
bool qwertzForShortcuts = false;
bool supportLevels5and6 = false;

Mapping dfkCtrlL = {
	.key = { VK_LCONTROL, SCAN_CTRL },
	.hold = { VK_LWIN, SCAN_LWIN, true },
	.tap = NULL,
	.state = RELEASED,
	.changed = 0,
	.n = NULL
};

Mapping dfkWinL = {
	.key = { VK_LWIN, SCAN_LWIN },
	.hold = { VK_LMENU, SCAN_ALT, false },
	.tap = NULL,
	.state = RELEASED,
	.changed = 0,
	.n = &dfkCtrlL
};

Mapping dfkAltL = {
	.key = { VK_LMENU, SCAN_ALT },
	.hold = { VK_LCONTROL, SCAN_CTRL, false },
	.tap = &(Tap){ { VK_ESCAPE, SCAN_ESC, false }, NULL },
	.state = RELEASED,
	.changed = 0,
	.n = &dfkWinL
};

Mapping dfkAltGr = {
	.key = { VK_RMENU, SCAN_ALT },
	.hold = { VK_RMENU, SCAN_ALT, true },
	.tap = &(Tap){ { VK_RETURN, SCAN_RETURN, false }, NULL },
	.state = RELEASED,
	.changed = 0,
	.n = &dfkAltL
};

Mapping dfkAe = {
	.key = { 0xDE, SCAN_Ae },
	.hold = { 0xBF, SCAN_Hash, false },
	.tap = &(Tap){ { 0xDE, SCAN_Ae, false }, NULL },
	.state = RELEASED,
	.changed = 0,
	.n = &dfkAltGr
};

Mapping dfkLess = {
	.key = { 0xE2, SCAN_Less },
	.hold = { 0xE2, SCAN_Less, false },
	.tap = &(Tap){ { VK_DELETE, 83, true }, NULL },
	.state = RELEASED,
	.changed = 0,
	.n = &dfkAe
};

Mapping dfkCaps = {
	.key = { VK_CAPITAL, SCAN_CAPS },
	.hold = { VK_CAPITAL, SCAN_CAPS, false },
	.tap = &(Tap){ { VK_BACK, SCAN_BACKSPACE, false }, NULL },
	.state = RELEASED,
	.changed = 0,
	.n = &dfkLess
};

DfkConfig dfkConfig = {
	.tap_millis = 0, // set via settings.ini
	.double_tap_millis = 0, // set via settings.ini
	.m = &dfkCaps
};

SystemKey systemKey = {
	.lCtrl = { VK_LCONTROL, SCAN_CTRL },
	.lWin = { VK_LWIN, SCAN_LWIN },
	.lAlt = { VK_LMENU, SCAN_ALT },
	.rAlt = { VK_RMENU, SCAN_ALT },
	.rWin = { VK_RWIN, SCAN_RWIN },
	.rCtrl = { VK_RCONTROL, SCAN_CTRL }
};

ModConfig modConfig = {
	.shift = {
		.lock = NULL,
		.left = { VK_LSHIFT, SCAN_LSHIFT },
		.right = { VK_RSHIFT, SCAN_RSHIFT },
		.bothLock = true
	},
	.mod3 = {
		.lock = NULL,
		.left = { VK_CAPITAL, SCAN_CAPS },
		.right = { VK_RMENU, SCAN_ALT },
		.bothLock = false
	},
	.mod4 = {
		.lock = &(InputKey){ VK_RCONTROL, SCAN_CTRL },
		.left = { 0xE2, SCAN_Less }, // > key
		.right = { 0xBF, SCAN_Hash }, // # key
		.bothLock = false
	}
};
