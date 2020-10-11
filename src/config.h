#ifndef _GUARD_WINDOWS
  #define _GUARD_WINDOWS
  #include <windows.h>
#endif

#define DEFAULT_TAP_MILLIS 200
#define DEFAULT_DOUBLE_TAP_MILLIS 150

typedef struct InputKey {
	int vk;
	int scan;
} InputKey;

typedef struct SendKey {
	int vk;
	int scan;
	bool isExtended;
} SendKey;

typedef enum DfkState {
    RELEASED,
    PRESSED,
    TAPPED,
    DOUBLETAPPED,
    CONSUMED,
} DfkState;

typedef struct Tap {
    SendKey code;
    struct Tap *n;
} Tap;

typedef struct Mapping {
    InputKey key;
    SendKey hold;
    Tap *tap;
    DfkState state;
    DWORD changed;
    struct Mapping *n;
} Mapping;

typedef struct DfkConfig {
    int tap_millis;
    int double_tap_millis;
    Mapping *m;
} DfkConfig;

// type to define neo modifier keys (i.e. mod3, mod4) and shift
typedef struct ModKeyConfig {
	InputKey *lock, left, right;
	bool bothLock; // should left + right at same time activate lock?
} ModKeyConfig;

typedef struct ModKeyConfigs {
	ModKeyConfig shift, mod3, mod4;
} ModKeyConfigs;

/* ============ VALUES ============ */

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

static DfkConfig dfkConfig = {
	.tap_millis = DEFAULT_TAP_MILLIS,
	.double_tap_millis = DEFAULT_DOUBLE_TAP_MILLIS,
	.m = &dfkCaps
};

static ModKeyConfigs modKeyConfigs = {
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
