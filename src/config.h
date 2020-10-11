#ifndef CONFIG_H_
#define CONFIG_H_

#include <windows.h>

#define LEN 103
#define DEFAULT_TAP_MILLIS 200
#define DEFAULT_DOUBLE_TAP_MILLIS 150

typedef struct InputKey {
	int vk;
	int scan;
} InputKey;

typedef struct OutputKey {
	int vk;
	int scan;
	bool isExtended;
} OutputKey;

typedef enum DfkState {
    RELEASED,
    PRESSED,
    TAPPED,
    DOUBLETAPPED,
    CONSUMED,
} DfkState;

typedef struct Tap {
    OutputKey code;
    struct Tap *n;
} Tap;

typedef struct Mapping {
    InputKey key;
    OutputKey hold;
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
typedef struct ModTypeConfig {
	InputKey *lock, left, right;
	bool bothLock; // should left + right at same time activate lock?
} ModTypeConfig;

typedef struct ModConfig {
	ModTypeConfig shift, mod3, mod4;
} ModConfig;

typedef struct SystemKey {
	InputKey lCtrl, lWin, lAlt, rAlt, rWin, rCtrl;
} SystemKey;

/* ============ VALUES ============ */

/**
 * Some global settings.
 * These values can be set in a configuration file (settings.ini)
 */
char layout[100];                    // keyboard layout (default: neo)
bool debugWindow = false;            // show debug output in a separate console window
bool capsLockEnabled = false;        // enable (allow) caps lock
bool shiftLockEnabled = false;       // enable (allow) shift lock (disabled if capsLockEnabled is true)
bool qwertzForShortcuts = false;     // use QWERTZ when Ctrl, Alt or Win is involved
bool supportLevels5and6 = false;     // support levels five and six (greek letters and mathematical symbols)

/**
 * Mapping tables for four levels.
 * They will be defined in initLayout().
 */
TCHAR mappingTableLevel1[LEN];
TCHAR mappingTableLevel2[LEN];
TCHAR mappingTableLevel3[LEN];
TCHAR mappingTableLevel4[LEN];
TCHAR mappingTableLevel5[LEN];
TCHAR mappingTableLevel6[LEN];
CHAR mappingTableLevel4Special[LEN];

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

static SystemKey systemKey = {
	.lCtrl = { VK_LCONTROL, 29 },
	.lWin = { VK_LWIN, 91 },
	.lAlt = { VK_LMENU, 56 },
	.rAlt = { VK_RMENU, 56 },
	.rWin = { VK_RWIN, 92 },
	.rCtrl = { VK_RCONTROL, 29 }
};

static ModConfig modConfig = {
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

#endif /* CONFIG_H_ */
