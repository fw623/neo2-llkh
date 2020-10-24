#ifndef CONFIG_H_
#define CONFIG_H_

#include <windows.h>
#include <stdbool.h>
#include "scancode.h"

#define LEN 103

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
    DOUBLETAPPED_HELD_FIRST,
    DOUBLETAPPED_HELD,
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
char layout[100];                   // keyboard layout (default: neo)
extern bool debugWindow;            // show debug output in a separate console window
extern bool capsLockEnabled;        // enable (allow) caps lock
extern bool shiftLockEnabled;       // enable (allow) shift lock (disabled if capsLockEnabled is true)
extern bool qwertzForShortcuts;     // use QWERTZ when Ctrl, Alt or Win is involved
extern bool supportLevels5and6;     // support levels five and six (greek letters and mathematical symbols)

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

extern DfkConfig dfkConfig;
extern SystemKey systemKey;
extern ModConfig modConfig;

#endif /* CONFIG_H_ */
