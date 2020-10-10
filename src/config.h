#ifndef _GUARD_WINDOWS
  #define _GUARD_WINDOWS
  #include <windows.h>
#endif

#define DEFAULT_TAP_MILLIS 200
#define DEFAULT_DOUBLE_TAP_MILLIS 150

typedef struct SendKey {
	int vk;
	int scan;
	bool isExtended;
} SendKey;

// dualFunctionKeys stuff

typedef enum State {
    RELEASED,
    PRESSED,
    TAPPED,
    DOUBLETAPPED,
    CONSUMED,
} State;

typedef struct Tap {
    SendKey code;
    struct Tap *n;
} Tap;

typedef struct Mapping {
    int key;
    SendKey hold;
    Tap *tap;
    State state;
    DWORD changed;
    struct Mapping *n;
} Mapping;

typedef struct Cfg {
    int tap_millis;
    int double_tap_millis;
    Mapping *m;
} Cfg;

// neo stuff

typedef struct LevelSendKey {
	SendKey	lvl1,
		lvl2,
		lvl3,
		lvl4,
		lvl5,
		lvl6;
} LevelSendKey;

typedef struct InputKey {
	int vk;
	int scan;
} InputKey;

typedef struct ModKeyConfig {
	InputKey key;
	SendKey	hold;
	LevelSendKey *tap;
} ModKeyConfig;

typedef struct NeoModKeyConfig {
	InputKey key;
	LevelSendKey *tap;
} NeoModKeyConfig;

// type to define neo modifier keys (i.e. mod3, mod4)
typedef struct NeoModConfig {
	InputKey *lock;
	NeoModKeyConfig left, right;
	bool bothLock; // should left + right at same time activate lock?
} NeoModConfig;

// type to define shift settings
typedef struct ShiftModConfig {
	ModKeyConfig *lock, left, right;
	bool bothLock; // should left + right at same time activate lock?
} ShiftModConfig;

typedef struct ModKeyConfigs {
	ShiftModConfig shift;
	NeoModConfig mod3, mod4;
	ModKeyConfig lCtrl,
		rCtrl,
		lAlt,
		rAlt,
		lMeta,
		rMeta;
} ModKeyConfigs;

/* ============ VALUES ============ */

LevelSendKey mod3LTap = {
	{ VK_BACK, 14, false },
	{ VK_BACK, 14, false },
	{ VK_BACK, 14, false },
	{ VK_BACK, 14, false },
	{ VK_BACK, 14, false },
	{ VK_BACK, 14, false },
};

LevelSendKey mod3RTap = {
	{ VK_RETURN, 28, false },
	{ VK_RETURN, 28, false },
	{ VK_RETURN, 28, false },
	{ VK_RETURN, 28, false },
	{ VK_RETURN, 28, false },
	{ VK_RETURN, 28, false },
};

InputKey mod4Lock = { VK_RCONTROL, 29 };

LevelSendKey mod4LTap = {
	{ VK_DELETE, 83, true },
	{ VK_DELETE, 83, true },
	{ VK_DELETE, 83, true },
	{ VK_DELETE, 83, true },
	{ VK_DELETE, 83, true },
	{ VK_DELETE, 83, true },
};

// @TODO: change to proper stuff
LevelSendKey mod4RTap = {
	{ 0x44, 32, false },
	{ 0x44, 32, false },
	{ 0x44, 32, false },
	{ 0x44, 32, false },
	{ 0x44, 32, false },
	{ 0x44, 32, false },
};

ModKeyConfigs modKeyConfigs = {
	.shift = {
		.lock = NULL,
		.left = { .key = { VK_LSHIFT, 42 }, .hold = { VK_LSHIFT, 42, false }, .tap = NULL },
		.right = { .key = { VK_RSHIFT, 54 }, .hold = { VK_RSHIFT, 54, true }, .tap = NULL },
		.bothLock = true
	},
	.mod3 = {
		.lock = NULL,
		// .left = { .key = { VK_CAPITAL, 58 }, .tap = &mod3LTap },
		// .right = { .key = { VK_RMENU, 56 }, .tap = &mod3RTap },
		.left = { .key = { VK_CAPITAL, 58 }, .tap = NULL },
		.right = { .key = { VK_RMENU, 56 }, .tap = NULL },
		.bothLock = false
	},
	.mod4 = {
		.lock = &mod4Lock,
		// .left = { .key = { 0xE2, 86 }, .tap = &mod4LTap }, // > key
		// .right = { .key = { 0xDE, 40 }, .tap = &mod4RTap }, // Ä key
		.left = { .key = { 0xE2, 86 }, .tap = NULL }, // > key
		.right = { .key = { 0xBF, 43 }, .tap = NULL }, // # key
		.bothLock = true
	},
	.lCtrl = {},
	.rCtrl = {},
	.lAlt = {},
	.rAlt = {},
	.lMeta = {},
	.rMeta = {},
};


// new way below

Mapping mod3R = {
	.key = 40,
	.hold = { 0xBF, 43, false },
	.tap = &(Tap){ { 0xDE, 40, false }, NULL },
	.state = RELEASED,
	.changed = 0,
	.n = NULL
};

Mapping mod4L = {
	.key = 86,
	.hold = { 0xE2, 86, false },
	.tap = &(Tap){ { VK_DELETE, 83, true }, NULL },
	.state = RELEASED,
	.changed = 0,
	.n = &mod3R
};

Mapping mod3L = {
	.key = 58,
	.hold = { VK_CAPITAL, 58, false },
	.tap = &(Tap){ { VK_BACK, 14, false }, NULL },
	.state = RELEASED,
	.changed = 0,
	.n = &mod4L
};

static Cfg cfg = {
	.tap_millis = DEFAULT_TAP_MILLIS,
	.double_tap_millis = DEFAULT_DOUBLE_TAP_MILLIS,
	.m = &mod3L
};
