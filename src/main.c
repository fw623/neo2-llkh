#define UNICODE
/**
 * Alternative Windows driver for the Neo2 based keyboard layouts:
 * Neo2, (www.neo-layout.org)
 * AdNW, AdNWzjßf, KOY (www.adnw.de)
 * bone (https://web.archive.org/web/20180721192908/http://wiki.neo-layout.org/wiki/Bone)
 * qwertz (https://de.wikipedia.org/wiki/QWERTZ-Tastaturbelegung)
 */

#ifndef _GUARD_WINDOWS
	#define _GUARD_WINDOWS
	#include <windows.h>
#endif

#include <stdio.h>
#include <wchar.h>
#include <stdbool.h>
#include "trayicon.h"
#include "resources.h"
#include <io.h>
#include <time.h>
#include "config.h"
#include "state.h"

typedef struct SimpleModState {
	bool shift, mod3, mod4;
} SimpleModState;

typedef struct LastKey {
	InputKey key;
	time_t time;
} LastKey;

HHOOK keyhook = NULL;
#define APPNAME "neo-llkh"
#define LEN 103
#define SCANCODE_TAB_KEY 15
#define SCANCODE_CAPSLOCK_KEY 58
#define SCANCODE_LOWER_THAN_KEY 86 // <
#define SCANCODE_QUOTE_KEY 40      // Ä
#define SCANCODE_HASH_KEY 43       // #
#define SCANCODE_RETURN_KEY 28
#define SCANCODE_ANY_ALT_KEY 56        // Alt or AltGr

/**
 * Some global settings.
 * These values can be set in a configuration file (settings.ini)
 */
char layout[100];                    // keyboard layout (default: neo)
bool debugWindow = false;            // show debug output in a separate console window
bool quoteAsMod3R = false;           // use quote/ä as right level 3 modifier
bool returnAsMod3R = false;          // use return as right level 3 modifier
bool tabAsMod4L = false;             // use tab as left level 4 modifier
DWORD scanCodeMod3L = SCANCODE_CAPSLOCK_KEY;
DWORD scanCodeMod4R = SCANCODE_HASH_KEY;       // depends on quoteAsMod3R and returnAsMod3R
DWORD scanCodeMod4L = SCANCODE_LOWER_THAN_KEY; // depends on tabAsMod4L
bool capsLockEnabled = false;        // enable (allow) caps lock
bool shiftLockEnabled = false;       // enable (allow) shift lock (disabled if capsLockEnabled is true)
bool level4LockEnabled = false;      // enable (allow) level 4 lock (toggle by pressing both Mod4 keys at the same time)
bool qwertzForShortcuts = false;     // use QWERTZ when Ctrl, Alt or Win is involved
bool swapLeftCtrlAndLeftAlt = false; // swap left Ctrl and left Alt key
bool swapLeftCtrlLeftAltAndLeftWin = false;  // swap left Ctrl, left Alt key and left Win key. Resulting order: Win, Alt, Ctrl (on a standard Windows keyboard)
bool supportLevels5and6 = false;     // support levels five and six (greek letters and mathematical symbols)
bool capsLockAsEscape = false;       // if true, hitting CapsLock alone sends Esc
bool mod3RAsReturn = false;          // if true, hitting Mod3R alone sends Return
bool mod4LAsTab = false;             // if true, hitting Mod4L alone sends Tab
SendKey mod4LTapOld = { VK_DELETE, 83, true };

// @TODO: implement proper modifier tapping and key remapping (for ctrl, alt, ...)


LastKey lastKey; // this saves the last keypress and is used for taps
int maxTapMillisec = 150;

/**
 * True if no mapping should be done
 */
bool bypassMode = false;

/**
 * States of some keys and shift lock.
 */
bool shiftLeftPressed = false;
bool shiftRightPressed = false;
bool shiftLockActive = false;
bool capsLockActive = false;

bool level3modLeftPressed = false;
bool level3modRightPressed = false;
bool level3modLeftAndNoOtherKeyPressed = false;
bool level3modRightAndNoOtherKeyPressed = false;
bool level4modLeftAndNoOtherKeyPressed = false;
bool level4modRightAndNoOtherKeyPressed = false;

bool level4modLeftPressed = false;
bool level4modRightPressed = false;
bool level4LockActive = false;

bool ctrlLeftPressed = false;
bool ctrlRightPressed = false;
bool altLeftPressed = false;
bool winLeftPressed = false;
bool winRightPressed = false;

SimpleModState modState = { false, false, false };

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

void SetStdOutToNewConsole()
{
	// allocate a console for this app
	AllocConsole();
	// redirect unbuffered STDOUT to the console
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	int fileDescriptor = _open_osfhandle((intptr_t)consoleHandle, _A_SYSTEM);
	FILE *fp = _fdopen(fileDescriptor, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);
	// give the console window a nicer title
	SetConsoleTitle(L"neo-llkh Debug Output");
	// give the console window a bigger buffer size
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(consoleHandle, &csbi)) {
		COORD bufferSize;
		bufferSize.X = csbi.dwSize.X;
		bufferSize.Y = 9999;
		SetConsoleScreenBufferSize(consoleHandle, bufferSize);
	}
}

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType) {
		// Handle the Ctrl-c signal.
		case CTRL_C_EVENT:
			printf("\nCtrl-c detected!\n");
			printf("Please quit by using the tray icon!\n\n");
			return TRUE;

		default:
			return FALSE;
	}
}

void mapLevels_2_5_6(TCHAR * mappingTableOutput, TCHAR * newChars)
{
	TCHAR * l1_lowercase = L"abcdefghijklmnopqrstuvwxyzäöüß.,";

	TCHAR *ptr;
	for (int i = 0; i < LEN; i++) {
		ptr = wcschr(l1_lowercase, mappingTableLevel1[i]);
		if (ptr != NULL && ptr < &l1_lowercase[32]) {
			//printf("i = %d: mappingTableLevel1[i] = %c; ptr = %d; ptr = %s; index = %d\n", i, mappingTableLevel1[i], ptr, ptr, ptr-l1_lowercase+1);
			mappingTableOutput[i] = newChars[ptr-l1_lowercase];
		}
	}
}

void initLevel4SpecialCases() {
	for (int i = 0; i < LEN; i++)
		mappingTableLevel4Special[i] = 0;

	mappingTableLevel4Special[16] = VK_PRIOR;
	if (strcmp(layout, "kou-fw623") == 0) {
		mappingTableLevel4Special[18] = VK_UP;
		mappingTableLevel4Special[20] = VK_NEXT;
		mappingTableLevel4Special[43] = VK_RETURN;
	} else if (strcmp(layout, "kou") == 0 || strcmp(layout, "vou") == 0) {
		mappingTableLevel4Special[17] = VK_NEXT;
		mappingTableLevel4Special[18] = VK_UP;
		mappingTableLevel4Special[19] = VK_BACK;
		mappingTableLevel4Special[20] = VK_DELETE;
	} else {
		mappingTableLevel4Special[17] = VK_BACK;
		mappingTableLevel4Special[18] = VK_UP;
		mappingTableLevel4Special[19] = VK_DELETE;
		mappingTableLevel4Special[20] = VK_NEXT;
	}
	mappingTableLevel4Special[30] = VK_HOME;
	mappingTableLevel4Special[31] = VK_LEFT;
	mappingTableLevel4Special[32] = VK_DOWN;
	mappingTableLevel4Special[33] = VK_RIGHT;
	mappingTableLevel4Special[34] = VK_END;
	if (strcmp(layout, "kou") == 0 || strcmp(layout, "vou") == 0) {
		mappingTableLevel4Special[44] = VK_INSERT;
		mappingTableLevel4Special[45] = VK_TAB;
		mappingTableLevel4Special[46] = VK_RETURN;
		mappingTableLevel4Special[47] = VK_ESCAPE;
	} else if (strcmp(layout, "kou-fw623") == 0) {
		mappingTableLevel4Special[44] = VK_SPACE;
		mappingTableLevel4Special[45] = VK_SPACE;
		mappingTableLevel4Special[46] = VK_RETURN;
		mappingTableLevel4Special[47] = VK_SPACE;
		mappingTableLevel4Special[49] = VK_BACK; // new special case
	} else {
		mappingTableLevel4Special[44] = VK_ESCAPE;
		mappingTableLevel4Special[45] = VK_TAB;
		mappingTableLevel4Special[46] = VK_INSERT;
		mappingTableLevel4Special[47] = VK_RETURN;
	}

	if (strcmp(layout, "kou-fw623") != 0)
		mappingTableLevel4Special[57] = '0';
}

void initLayout()
{
	// initialize the mapping tables
	for (int i = 0; i < LEN; i++) {
		mappingTableLevel1[i] = 0;
		mappingTableLevel2[i] = 0;
		mappingTableLevel3[i] = 0;
		mappingTableLevel4[i] = 0;
		if (supportLevels5and6) {
			mappingTableLevel5[i] = 0;
			mappingTableLevel6[i] = 0;
		}
	}

	// same for all layouts
	wcscpy(mappingTableLevel1 +  2, L"1234567890-`");

	wcscpy(mappingTableLevel2 + 41, L"̌");  // key to the left of the "1" key
	wcscpy(mappingTableLevel2 + 2, strcmp(layout, "kou-fw623") == 0 ? L"°§ℓ»«¢€„“”–" : L"°§ℓ»«$€„“”—̧");

	wcscpy(mappingTableLevel3 + 41, L"^");
	wcscpy(mappingTableLevel3 + 2, strcmp(layout, "kou-fw623") == 0 ? L"¹²³›‹¥£‚‘’—" : L"¹²³›‹¢¥‚‘’—̊");
	wcscpy(mappingTableLevel3 + 16, L"…_[]^!<>=&ſ̷");
	wcscpy(mappingTableLevel3 + 30, L"\\/{}*?()-:@"); // TODO: fix right brace in vscode
	wcscpy(mappingTableLevel3 + 44, L"#$|~`+%\"';"); // TODO: fix backslash in vscode

	wcscpy(mappingTableLevel4 + 41, L"̇");
	wcscpy(mappingTableLevel4 +  2, L"ªº№⋮·£¤0/*-¨");
	wcscpy(mappingTableLevel4 + 21, L"¡789+−˝");
	wcscpy(mappingTableLevel4 + 35, L"¿456,.");
	wcscpy(mappingTableLevel4 + 49, L":123;");

	// layout dependent
	if (strcmp(layout, "adnw") == 0) {
		wcscpy(mappingTableLevel1 + 16, L"kuü.ävgcljf´");
		wcscpy(mappingTableLevel1 + 30, L"hieaodtrnsß");
		wcscpy(mappingTableLevel1 + 44, L"xyö,qbpwmz");

	} else if (strcmp(layout, "adnwzjf") == 0) {
		wcscpy(mappingTableLevel1 + 16, L"kuü.ävgclßz´");
		wcscpy(mappingTableLevel1 + 30, L"hieaodtrnsf");
		wcscpy(mappingTableLevel1 + 44, L"xyö,qbpwmj");

	} else if (strcmp(layout, "bone") == 0) {
		wcscpy(mappingTableLevel1 + 16, L"jduaxphlmwß´");
		wcscpy(mappingTableLevel1 + 30, L"ctieobnrsgq");
		wcscpy(mappingTableLevel1 + 44, L"fvüäöyz,.k");

	} else if (strcmp(layout, "koy") == 0) {
		wcscpy(mappingTableLevel1 + 16, L"k.o,yvgclßz´");
		wcscpy(mappingTableLevel1 + 30, L"haeiudtrnsf");
		wcscpy(mappingTableLevel1 + 44, L"xqäüöbpwmj");

	} else if (strcmp(layout, "kou") == 0
				  || strcmp(layout, "kou-fw623") == 0
				  || strcmp(layout, "vou") == 0) {
		if (strcmp(layout, "kou") == 0 || strcmp(layout, "kou-fw623") == 0) {
			wcscpy(mappingTableLevel1 + 16, L"k.ouäqgclfj´");
			wcscpy(mappingTableLevel1 + 30, L"haeiybtrnsß");
			wcscpy(mappingTableLevel1 + 44, L"zx,üöpdwmv");
		} else {  // vou
			wcscpy(mappingTableLevel1 + 16, L"v.ouäqglhfj´");
			wcscpy(mappingTableLevel1 + 30, L"caeiybtrnsß");
			wcscpy(mappingTableLevel1 + 44, L"zx,üöpdwmk");
		}

		if (strcmp(layout, "kou-fw623") == 0) {
			wcscpy(mappingTableLevel3 + 16, L"@=:^• #<>~→̷");
			wcscpy(mappingTableLevel3 + 30, L"`'-/*&{()}|");
			wcscpy(mappingTableLevel3 + 44, L"\\\"_+ ;[]$%");

			wcscpy(mappingTableLevel4 + 4, L"    =/*-−¨");
			wcscpy(mappingTableLevel4 + 21, L";789+\t−˝");
			wcscpy(mappingTableLevel4 + 35, L":456.\n");
			wcscpy(mappingTableLevel4 + 48, L"·");
			wcscpy(mappingTableLevel4 + 50, L"123,");
		} else { // kou or vou
			wcscpy(mappingTableLevel3 + 16, L"@%{}^!<>=&€̷");
			wcscpy(mappingTableLevel3 + 30, L"|`()*?/:-_→");
			wcscpy(mappingTableLevel3 + 44, L"#[]~$+\"'\\;");

			wcscpy(mappingTableLevel4 + 4, L"✔✘·£¤0/*-¨");
			wcscpy(mappingTableLevel4 + 21, L":789+−˝");
			wcscpy(mappingTableLevel4 + 35, L"-456,;");
			wcscpy(mappingTableLevel4 + 49, L"_123.");
		}
	} else if (strcmp(layout, "qwertz") == 0) {
		wcscpy(mappingTableLevel1 + 12, L"ß");
		wcscpy(mappingTableLevel1 + 16, L"qwertzuiopü+");
		wcscpy(mappingTableLevel1 + 30, L"asdfghjklöä");
		wcscpy(mappingTableLevel1 + 44, L"yxcvbnm,.-");

	} else { // neo
		wcscpy(mappingTableLevel1 + 16, L"xvlcwkhgfqß´");
		wcscpy(mappingTableLevel1 + 30, L"uiaeosnrtdy");
		wcscpy(mappingTableLevel1 + 44, L"üöäpzbm,.j");
	}

	// same for all layouts
	wcscpy(mappingTableLevel1 + 27, L"´");
	wcscpy(mappingTableLevel2 + 27, L"~");

	// map letters of level 2
	TCHAR * charsLevel2;
	charsLevel2 = strcmp(layout, "kou-fw623") == 0 ? L"ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÜẞ?!" : L"ABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÜẞ•–";
	mapLevels_2_5_6(mappingTableLevel2, charsLevel2);

	if (supportLevels5and6) {
		// map main block on levels 5 and 6
		TCHAR * charsLevel5 = L"αβχδεφγψιθκλμνοπϕρστuvωξυζηϵüςϑϱ";  // a-zäöüß.,
		mapLevels_2_5_6(mappingTableLevel5, charsLevel5);
		TCHAR * charsLevel6 = L"∀⇐ℂΔ∃ΦΓΨ∫Θ⨯Λ⇔ℕ∈ΠℚℝΣ∂⊂√ΩΞ∇ℤℵ∩∪∘↦⇒";  // a-zäöüß.,
		mapLevels_2_5_6(mappingTableLevel6, charsLevel6);

		// add number row and dead key in upper letter row
		wcscpy(mappingTableLevel5 + 41, L"̉");
		wcscpy(mappingTableLevel5 +  2, L"₁₂₃♂♀⚥ϰ⟨⟩₀?῾");
		wcscpy(mappingTableLevel5 + 27, L"᾿");
		mappingTableLevel5[57] = 0x00a0;  // space = no-break space
		wcscpy(mappingTableLevel6 + 41, L"̣");
		wcscpy(mappingTableLevel6 +  2, L"¬∨∧⊥∡∥→∞∝⌀?̄");
		wcscpy(mappingTableLevel6 + 27, L"˘");
		mappingTableLevel6[57] = 0x202f;  // space = narrow no-break space
	}

	// if quote/ä is the right level 3 modifier, copy symbol of quote/ä key to backslash/# key
	if (quoteAsMod3R) {
		mappingTableLevel1[43] = mappingTableLevel1[40];
		mappingTableLevel2[43] = mappingTableLevel2[40];
		mappingTableLevel3[43] = mappingTableLevel3[40];
		mappingTableLevel4[43] = mappingTableLevel4[40];
		if (supportLevels5and6) {
			mappingTableLevel5[43] = mappingTableLevel5[40];
			mappingTableLevel6[43] = mappingTableLevel6[40];
		}
	}

	mappingTableLevel2[8] = 0x20AC;  // €

	// level4 special cases
	initLevel4SpecialCases();
}

void toggleBypassMode()
{
	bypassMode = !bypassMode;

	HINSTANCE hInstance = GetModuleHandle(NULL);
	HICON icon = bypassMode
		? LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON_DISABLED))
		: LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));

	trayicon_change_icon(icon);
	printf("%i bypass mode \n", bypassMode);
}

KBDLLHOOKSTRUCT newKeyInfo (SendKey send, DWORD flags) {
	KBDLLHOOKSTRUCT keyInfo = {
		.vkCode = send.vk,
		.scanCode = send.scan,
		.flags = (send.isExtended ? LLKHF_EXTENDED : 0) | (flags & ~LLKHF_EXTENDED), // ignore extended flag of incoming key
		.time = 0,
		.dwExtraInfo = 0
	};
	return keyInfo;
}

/**
 * Map a key scancode to the char that should be displayed after typing
 **/
TCHAR mapScanCodeToChar(unsigned level, char in)
{
	switch (level) {
		case 2:
			return mappingTableLevel2[in];
		case 3:
			return mappingTableLevel3[in];
		case 4:
			return mappingTableLevel4[in];
		case 5:
			return mappingTableLevel5[in];
		case 6:
			return mappingTableLevel6[in];
		default: // level 1
			return mappingTableLevel1[in];
	}
}

/**
 * Maps keyInfo flags (https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-kbdllhookstruct)
 * to dwFlags for keybd_event (https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-keybd_event)
 **/
DWORD dwFlagsFromKeyInfo(KBDLLHOOKSTRUCT keyInfo)
{
	DWORD dwFlags = 0;
	if (keyInfo.flags & LLKHF_EXTENDED) dwFlags |= KEYEVENTF_EXTENDEDKEY;
	if (keyInfo.flags & LLKHF_UP) dwFlags |= KEYEVENTF_KEYUP;
	return dwFlags;
}

void sendDown(BYTE vkCode, BYTE scanCode, bool isExtendedKey) {
	keybd_event(vkCode, scanCode, (isExtendedKey ? KEYEVENTF_EXTENDEDKEY : 0), 0);
}

void sendDown2(SendKey key) {
	keybd_event(key.vk, key.scan, (key.isExtended ? KEYEVENTF_EXTENDEDKEY : 0), 0);
}

void sendUp(BYTE vkCode, BYTE scanCode, bool isExtendedKey) {
	keybd_event(vkCode, scanCode, (isExtendedKey ? KEYEVENTF_EXTENDEDKEY : 0) | KEYEVENTF_KEYUP, 0);
}

void sendUp2(SendKey key) {
	keybd_event(key.vk, key.scan, (key.isExtended ? KEYEVENTF_EXTENDEDKEY : 0) | KEYEVENTF_KEYUP, 0);
}

void sendDownUp(BYTE vkCode, BYTE scanCode, bool isExtendedKey) {
	sendDown(vkCode, scanCode, isExtendedKey);
	sendUp(vkCode, scanCode, isExtendedKey);
}

void sendDownUp2(SendKey key) {
	sendDown2(key);
	sendUp2(key);
}

void sendUnicodeChar(TCHAR key, KBDLLHOOKSTRUCT keyInfo)
{
	INPUT Input = {
		.type = INPUT_KEYBOARD,
		.ki = {
			.wScan = key,
			.dwFlags = KEYEVENTF_UNICODE | dwFlagsFromKeyInfo(keyInfo)
		}
	};
	SendInput(1, &Input, sizeof(Input));
}

/**
 * Sends a char using emulated keyboard input
 * This works for most cases, but not for dead keys etc
 **/
void sendChar(TCHAR key, KBDLLHOOKSTRUCT keyInfo)
{
	SHORT keyScanResult = VkKeyScanEx(key, GetKeyboardLayout(0));

	if (keyScanResult == -1 || shiftLockActive || capsLockActive || level4LockActive
		|| (keyInfo.vkCode >= 0x30 && keyInfo.vkCode <= 0x39)) {
		// key not found in the current keyboard layout or shift lock is active
		//
		// If shiftLockActive is true, a unicode letter will be sent. This implies
		// that shortcuts don't work in shift lock mode. That's good, because
		// people might not be aware that they would send Ctrl-S instead of
		// Ctrl-s. Sending a unicode letter makes it possible to undo shift
		// lock temporarily by holding one shift key because that way the
		// shift key won't be sent.
		//
		// Furthermore, use unicode for number keys.
		sendUnicodeChar(key, keyInfo);
	} else {
		keyInfo.vkCode = keyScanResult;
		char modifiers = keyScanResult >> 8;
		bool shift = ((modifiers & 1) != 0);
		bool alt = ((modifiers & 2) != 0);
		bool ctrl = ((modifiers & 4) != 0);
		bool altgr = alt && ctrl;
		if (altgr) {
			ctrl = false;
			alt = false;
		}

		if (altgr) sendDown(VK_RMENU, 56, false);
		if (ctrl) sendDown(VK_CONTROL, 29, false);
		if (alt) sendDown(VK_MENU, 56, false); // ALT
		if (shift) sendDown(VK_SHIFT, 42, false);

		keybd_event(keyInfo.vkCode, keyInfo.scanCode, dwFlagsFromKeyInfo(keyInfo), keyInfo.dwExtraInfo);

		if (altgr) sendUp(VK_RMENU, 56, false);
		if (ctrl) sendUp(VK_CONTROL, 29, false);
		if (alt) sendUp(VK_MENU, 56, false); // ALT
		if (shift) sendUp(VK_SHIFT, 42, false);
	}
}

/**
 * Send a usually dead key by injecting space after (on down).
 * This will add an actual space if actual dead key is followed by "dead" key with this
 **/
void commitDeadKey(KBDLLHOOKSTRUCT keyInfo)
{
	if (!(keyInfo.flags & LLKHF_UP)) sendDownUp(VK_SPACE, 57, false);
}

bool isInputKey(KBDLLHOOKSTRUCT actual, InputKey desired) {
	return actual.vkCode == desired.vk && actual.scanCode == desired.scan;
}

bool handleLayer2SpecialCases(KBDLLHOOKSTRUCT keyInfo)
{
	switch(keyInfo.scanCode) {
		case 27:
			sendChar(L'̃', keyInfo);  // perispomene (Tilde)
			return true;
		case 41:
			sendChar(L'̌', keyInfo);  // caron, wedge, háček (Hatschek)
			return true;
		default:
			return false;
	}
}

bool handleLayer3SpecialCases(KBDLLHOOKSTRUCT keyInfo)
{
	switch(keyInfo.scanCode) {
		case 13:
			sendChar(L'̊', keyInfo);  // overring
			return true;
		case 19:
			if (strcmp(layout, "kou-fw623") == 0) {
				sendChar(L'^', keyInfo);
				commitDeadKey(keyInfo);
				return true;
			}
			return false;
		case 20:
			if (strcmp(layout, "kou-fw623") != 0) {
				sendChar(L'^', keyInfo);
				commitDeadKey(keyInfo);
				return true;
			}
			return false;
		case 27:
			sendChar(L'̷', keyInfo);  // bar (diakritischer Schrägstrich)
			return true;
		case 30:
			if (strcmp(layout, "kou-fw623") == 0) {
				sendChar(L'`', keyInfo);
				commitDeadKey(keyInfo);
				return true;
			}
			return false;
		case 31:
			if (strcmp(layout, "kou") == 0 || strcmp(layout, "vou") == 0) {
				sendChar(L'`', keyInfo);
				commitDeadKey(keyInfo);
				return true;
			}
			return false;
		case 48:
			if (strcmp(layout, "kou") != 0 && strcmp(layout, "kou-fw623") != 0 && strcmp(layout, "vou") != 0) {
				sendChar(L'`', keyInfo);
				commitDeadKey(keyInfo);
				return true;
			}
			return false;
		default:
			return false;
	}
}

bool handleLayer4SpecialCases(KBDLLHOOKSTRUCT keyInfo)
{
	switch(keyInfo.scanCode) {
		case 13:
			sendChar(L'¨', keyInfo);  // diaeresis, umlaut
			return true;
		case 27:
			sendChar(L'˝', keyInfo);  // double acute (doppelter Akut)
			return true;
		case 41:
			sendChar(L'̇', keyInfo);  // dot above (Punkt, darüber)
			return true;
	}

	// handle repeated up/down on layer 4 of kou-fw623 layout
	if (strcmp(layout, "kou-fw623") == 0 && (keyInfo.scanCode == 17 || keyInfo.scanCode == 19))	{
		if (keyInfo.flags & LLKHF_UP) {
			if (keyInfo.scanCode == 17) {
				sendUp(VK_UP, 72, true);
			}	else if (keyInfo.scanCode == 19) {
				sendUp(VK_DOWN, 80, true);
			}
			return true;
		}

		if (keyInfo.scanCode == 17) {
			for (int i = 0; i < 7; i++)
				sendDownUp(VK_UP, 72, true);
			sendDown(VK_UP, 72, true);
			return true;
		}	else if (keyInfo.scanCode == 19) {
			for (int i = 0; i < 7; i++)
				sendDownUp(VK_DOWN, 80, true);
			sendDown(VK_DOWN, 80, true);
			return true;
		}
	}

	// A second level 4 mapping table for special (non-unicode) keys.
	// Maybe this could be included in the global TCHAR mapping table or level 4!?
	BYTE bScan = 0;

	if (mappingTableLevel4Special[keyInfo.scanCode] != 0) {
		if (mappingTableLevel4Special[keyInfo.scanCode] == VK_RETURN)
			bScan = 0x1c;
		else if (mappingTableLevel4Special[keyInfo.scanCode] == VK_INSERT)
			bScan = 0x52;

		// extended flag (bit 0) is necessary for selecting text with shift + arrow
		keybd_event(mappingTableLevel4Special[keyInfo.scanCode], bScan, dwFlagsFromKeyInfo(keyInfo) | KEYEVENTF_EXTENDEDKEY, 0);

		return true;
	}
	return false;
}

bool isShift(KBDLLHOOKSTRUCT keyInfo)
{
	return keyInfo.vkCode == VK_SHIFT
	    || keyInfo.vkCode == VK_LSHIFT
	    || keyInfo.vkCode == VK_RSHIFT;
}

bool isSystemKeyPressed()
{
	return ctrlLeftPressed || ctrlRightPressed
	    || altLeftPressed
	    || winLeftPressed || winRightPressed;
}

bool isLetter(TCHAR key)
{
	return (key >= 65 && key <= 90  // A-Z
	     || key >= 97 && key <= 122 // a-z
	     || key == L'ä' || key == L'ö'
	     || key == L'ü' || key == L'ß'
	     || key == L'Ä' || key == L'Ö'
	     || key == L'Ü' || key == L'ẞ');
}

void toggleShiftLock()
{
	shiftLockActive = !shiftLockActive;
	printf("Shift lock %s!\n", shiftLockActive ? "activated" : "deactivated");
}

void toggleCapsLock()
{
	capsLockActive = !capsLockActive;
	printf("Caps lock %s!\n", capsLockActive ? "activated" : "deactivated");
}

void logKeyEvent(char *desc, KBDLLHOOKSTRUCT keyInfo)
{
	char vkCodeLetter[4] = {'(', keyInfo.vkCode, ')', 0};
	char *keyName;
	switch (keyInfo.vkCode) {
		case VK_LSHIFT:
			keyName = "(Shift left)";
			break;
		case VK_RSHIFT:
			keyName = "(Shift right)";
			break;
		case VK_SHIFT:
			keyName = "(Shift)";
			break;
		case VK_CAPITAL:
			keyName = "(M3 left)";
			break;
		case 0xde:  // ä
			keyName = quoteAsMod3R ? "(M3 right)" : "";
			break;
		case 0xbf:  // #
			keyName = quoteAsMod3R ? "" : "(M3 right)";
			break;
		case VK_OEM_102:
			keyName = "(M4 left [<])";
			break;
		case VK_CONTROL:
			keyName = "(Ctrl)";
			break;
		case VK_LCONTROL:
			keyName = "(Ctrl left)";
			break;
		case VK_RCONTROL:
			keyName = "(Ctrl right)";
			break;
		case VK_MENU:
			keyName = "(Alt)";
			break;
		case VK_LMENU:
			keyName = "(Alt left)";
			break;
		case VK_RMENU:
			keyName = "(Alt right)";
			break;
		case VK_LWIN:
			keyName = "(Win left)";
			break;
		case VK_RWIN:
			keyName = "(Win right)";
			break;
		case VK_BACK:
			keyName = "(Backspace)";
			break;
		case VK_RETURN:
			keyName = "(Return)";
			break;
		case 0x41 ... 0x5A:
			keyName = vkCodeLetter;
			break;
		default:
			keyName = "";
			//keyName = MapVirtualKeyA(keyInfo.vkCode, MAPVK_VK_TO_CHAR);
	}
	char *shiftLockCapsLockInfo = shiftLockActive ? " [shift lock active]"
						: (capsLockActive ? " [caps lock active]" : "");
	char *level4LockInfo = level4LockActive ? " [level4 lock active]" : "";
	char *vkPacket = (desc=="injected" && keyInfo.vkCode == VK_PACKET) ? " (VK_PACKET)" : "";
	printf(
		"%-13s | sc:%03u vk:0x%02X flags:0x%02X extra:%d %s%s%s%s\n",
		desc, keyInfo.scanCode, keyInfo.vkCode, keyInfo.flags, keyInfo.dwExtraInfo,
		keyName, shiftLockCapsLockInfo, level4LockInfo, vkPacket
	);
}

bool modIsActive (NeoModState state) {
	return (state.leftIsPressed || state.rightIsPressed) != state.isLocked;
}

unsigned getLevel() {
	unsigned level = 1;

	if (modIsActive(modKeyStates.shift))
		level = 2;
	if (modIsActive(modKeyStates.mod3))
		level = (supportLevels5and6 && level == 2) ? 5 : 3;
	if (modIsActive(modKeyStates.mod4))
		level = (supportLevels5and6 && level == 3) ? 6 : 4;

	return level;
}

/**
 * returns `true` if no systemKey was pressed -> continue execution, `false` otherwise
 **/
boolean handleSystemKey(KBDLLHOOKSTRUCT keyInfo, bool isKeyUp) {
	bool newStateValue = !isKeyUp;
	DWORD dwFlags = isKeyUp ? KEYEVENTF_KEYUP : 0;

	// Check also the scan code because AltGr sends VK_LCONTROL with scanCode 541
	if (keyInfo.vkCode == VK_LCONTROL && keyInfo.scanCode == 29) {
		if (swapLeftCtrlAndLeftAlt) {
			altLeftPressed = newStateValue;
			keybd_event(VK_LMENU, 56, dwFlags, 0);
		} else if (swapLeftCtrlLeftAltAndLeftWin) {
			winLeftPressed = newStateValue;
			keybd_event(VK_LWIN, 91, dwFlags, 0);
		} else {
			ctrlLeftPressed = newStateValue;
			keybd_event(VK_LCONTROL, 29, dwFlags, 0);
		}
		return false;
	} else if (keyInfo.vkCode == VK_RCONTROL) {
		ctrlRightPressed = newStateValue;
		keybd_event(VK_RCONTROL, 29, dwFlags, 0);
	} else if (keyInfo.vkCode == VK_LMENU) {
		if (swapLeftCtrlAndLeftAlt || swapLeftCtrlLeftAltAndLeftWin) {
			ctrlLeftPressed = newStateValue;
			keybd_event(VK_LCONTROL, 29, dwFlags, 0);
		} else {
			altLeftPressed = newStateValue;
			keybd_event(VK_LMENU, 56, dwFlags, 0);
		}
		return false;
	} else if (keyInfo.vkCode == VK_LWIN) {
		if (swapLeftCtrlLeftAltAndLeftWin) {
			altLeftPressed = newStateValue;
			keybd_event(VK_LMENU, 56, dwFlags, 0);
		} else {
			winLeftPressed = newStateValue;
			keybd_event(VK_LWIN, 91, dwFlags, 0);
		}
		return false;
	} else if (keyInfo.vkCode == VK_RWIN) {
		winRightPressed = newStateValue;
		keybd_event(VK_RWIN, 92, dwFlags, 0);
		return false;
	}

	return true;
}

void toggleModLockConditionally(bool *isLocked, bool isKeyUp, bool ignoreLocking) {
	if (!isKeyUp && !ignoreLocking) {
		*isLocked = !(*isLocked);
	}
}

// also toggle CapsLock (for keyboard LED)
void toggleModLockConditionallyShift(bool *isLocked, bool isKeyUp, bool ignoreLocking) {
	if (!isKeyUp && !ignoreLocking) {
		// TODO: fix NumberRow characters during lock

		// NOTE: when sending CapsLock like this, we mustn't use the internal isLocked state
		// because these 2 interfere with each other
		// the probably cleaner solution is to use only the internal state and ignore the CapsLock LED

		sendDownUp(VK_CAPITAL, 58, false);
		toggleCapsLock();
	}
}

// ignoreLocking - is used for shift key during bypassMode
bool handleModKey(KBDLLHOOKSTRUCT keyInfo, bool isKeyUp, NeoModConfig mod, NeoModState *state, bool ignoreLocking, void (*toggleLockConditionally)(bool *, bool, bool)) {
	if (mod.lock && isInputKey(keyInfo, *mod.lock)) {
		toggleLockConditionally(&state->isLocked, isKeyUp, ignoreLocking);
		return true;
	} else if (isInputKey(keyInfo, mod.left)) {
		state->leftIsPressed = !isKeyUp;
		if (mod.bothLock && state->rightIsPressed) {
			toggleLockConditionally(&state->isLocked, isKeyUp, ignoreLocking);
		}
		return true;
	} else if (isInputKey(keyInfo, mod.right)) {
		state->rightIsPressed = !isKeyUp;
		if (mod.bothLock && state->leftIsPressed) {
			toggleLockConditionally(&state->isLocked, isKeyUp, ignoreLocking);
		}
		return true;
	}

	return false;
}

/**
 * updates system key and layerLock states; writes key
 * returns `true` if next hook should be called, `false` otherwise
 **/
bool updateStatesAndWriteKey(KBDLLHOOKSTRUCT keyInfo, bool isKeyUp)
{
	bool continueExecution = handleSystemKey(keyInfo, isKeyUp);
	if (!continueExecution) return false;

	unsigned level = getLevel();

	if (handleModKey(keyInfo, isKeyUp, modKeyConfigs.mod3, &modKeyStates.mod3, false, toggleModLockConditionally)) {
		return false;
	} else if (handleModKey(keyInfo, isKeyUp, modKeyConfigs.mod4, &modKeyStates.mod4, false, toggleModLockConditionally)) {
		return false;
	} else if (keyInfo.flags == 1) {
		return true;
	} else if (level == 2 && handleLayer2SpecialCases(keyInfo)) {
		return false;
	} else if (level == 3 && handleLayer3SpecialCases(keyInfo)) {
		return false;
	} else if (level == 4 && handleLayer4SpecialCases(keyInfo)) {
		return false;
	} else if (keyInfo.vkCode >= 0x60 && keyInfo.vkCode <= 0x6F) {
		// Numeric keypad -> don't remap
	} else if (level == 1 && keyInfo.vkCode >= 0x30 && keyInfo.vkCode <= 0x39) {
		// numbers 0 to 9 -> don't remap
	} else if (!(qwertzForShortcuts && isSystemKeyPressed())) {
		TCHAR key = mapScanCodeToChar(level, keyInfo.scanCode);
		if (capsLockActive && (level == 1 || level == 2) && isLetter(key)) {
			key = mapScanCodeToChar(level==1 ? 2 : 1, keyInfo.scanCode);
		} if (key != 0 && (keyInfo.flags & LLKHF_INJECTED) == 0) {
			// if key must be mapped
			int character = MapVirtualKeyA(keyInfo.vkCode, MAPVK_VK_TO_CHAR);
			printf("%-13s | sc:%03d %c->%c [0x%04X] (level %u)\n", "mapped", keyInfo.scanCode, character, key, key, level);
			sendChar(key, keyInfo);
			return false;
		}
	}

	return true;
}

bool write_event(const KBDLLHOOKSTRUCT keyInfo) {
	WPARAM wparam = (keyInfo.flags & LLKHF_UP) ? WM_KEYUP : WM_KEYDOWN;

	// handle shift here; necessary because we need to track it also in bypassMode
	if (handleModKey(keyInfo, keyInfo.flags & LLKHF_UP, modKeyConfigs.shift, &modKeyStates.shift, bypassMode, toggleModLockConditionallyShift)) {
		return false;
	}

	// Shift + Pause
	if (wparam == WM_KEYDOWN && keyInfo.vkCode == VK_PAUSE && (modKeyStates.shift.leftIsPressed || modKeyStates.shift.rightIsPressed)) {
		toggleBypassMode();
		return true;
	}

	if (bypassMode) {
		if (keyInfo.vkCode == VK_CAPITAL && !(keyInfo.flags & LLKHF_UP)) {
			// synchronize with capsLock state during bypass
			if (shiftLockEnabled) {
				toggleShiftLock();
			} else if (capsLockEnabled) {
				toggleCapsLock();
			}
		}
		keybd_event(keyInfo.vkCode, keyInfo.scanCode, dwFlagsFromKeyInfo(keyInfo), keyInfo.dwExtraInfo);
		return true;
	}

	// skip LCONTROL sent by pressing ALTGR
	if (keyInfo.scanCode == 541) return true;

	if (wparam == WM_SYSKEYUP || wparam == WM_KEYUP) {
		logKeyEvent("key up", keyInfo);

		bool callNext = updateStatesAndWriteKey(keyInfo, true);
		if (!callNext) return true;
	} else if (wparam == WM_SYSKEYDOWN || wparam == WM_KEYDOWN) {
		logKeyEvent("key down", keyInfo);

		level3modLeftAndNoOtherKeyPressed = false;
		level3modRightAndNoOtherKeyPressed = false;
		level4modLeftAndNoOtherKeyPressed = false;
		level4modRightAndNoOtherKeyPressed = false;

		bool callNext = updateStatesAndWriteKey(keyInfo, false);
		if (!callNext) return true;
	}

	// send the incoming key if nothing matches
	keybd_event(keyInfo.vkCode, keyInfo.scanCode, dwFlagsFromKeyInfo(keyInfo), keyInfo.dwExtraInfo);
	return true;
}

void tap(Mapping *m, DWORD flags) {
    Tap *t;
    for (t = m->tap; t; t = t->n) {
				printf("remapped tap\n");
        write_event(newKeyInfo(t->code, flags));
    }
}

void handle_press(Mapping *m, KBDLLHOOKSTRUCT *input) {
    // state
    switch (m->state) {
        case TAPPED:
        case DOUBLETAPPED:
            if (input->time - m->changed < cfg.double_tap_millis)
                m->state = DOUBLETAPPED;
            else
                m->state = PRESSED;
            break;
        case RELEASED:
        case CONSUMED:
            m->state = PRESSED;
            break;
        case PRESSED:
            break;
    }
    m->changed = input->time;

    // action
    switch (m->state) {
        case TAPPED:
        case DOUBLETAPPED:
            tap(m, 0);
            break;
        case RELEASED:
        case PRESSED:
        case CONSUMED:
						printf("remapped down\n");
            write_event(newKeyInfo(m->hold, input->flags));
            break;
    }
}

void handle_release(Mapping *m, KBDLLHOOKSTRUCT *input) {
    // state
    switch (m->state) {
        case PRESSED:
            if (input->time - m->changed < cfg.tap_millis)
                m->state = TAPPED;
            else
                m->state = RELEASED;
            break;
        case TAPPED:
        case DOUBLETAPPED:
            break;
        case CONSUMED:
            m->state = RELEASED;
            break;
        case RELEASED:
            break;
    }
    m->changed = input->time;

    // action
    switch (m->state) {
        case TAPPED:
						printf("remapped release\n");
            // release
            write_event(newKeyInfo(m->hold, input->flags));

            // synthesize tap
            tap(m, 0);
            tap(m, LLKHF_UP);
            break;
        case DOUBLETAPPED:
            tap(m, LLKHF_UP);
            break;
        case CONSUMED:
        case RELEASED:
        case PRESSED:
						printf("remapped up\n");
            write_event(newKeyInfo(m->hold, input->flags));
            break;
    }
}

void consume_pressed() {

    // state
    for (Mapping *m = cfg.m; m; m = m->n) {
        switch (m->state) {
            case PRESSED:
                m->state = CONSUMED;
                break;
            case TAPPED:
            case DOUBLETAPPED:
            case RELEASED:
            case CONSUMED:
                break;
        }
    }
}

bool
dualFunctionKeys(KBDLLHOOKSTRUCT *input) {
    Mapping *m;

    // consume all taps that are incomplete
    if (!(input->flags & LLKHF_UP))
        consume_pressed();

    // is this our key?
    for (m = cfg.m; m && !isInputKey(*input, m->key); m = m->n);

    // forward all other key events
    if (!m) {
        // write_event(*input);
        return false;
    }

    if (input->flags & LLKHF_UP) {
        handle_release(m, input);
    } else {
        handle_press(m, input);
    }
		return true;
}

__declspec(dllexport)
LRESULT CALLBACK keyevent(int code, WPARAM wparam, LPARAM lparam)
{
	LastKey currentKey;

	if (
		code != HC_ACTION
		|| !(wparam == WM_SYSKEYUP || wparam == WM_KEYUP || wparam == WM_SYSKEYDOWN || wparam == WM_KEYDOWN)
	) {
		/* Passes the hook information to the next hook procedure in the current hook chain.
		* 1st Parameter hhk - Optional
		* 2nd Parameter nCode - The next hook procedure uses this code to determine how to process the hook information.
		* 3rd Parameter wParam - The wParam value passed to the current hook procedure.
		* 4th Parameter lParam - The lParam value passed to the current hook procedure
		*/
		return CallNextHookEx(NULL, code, wparam, lparam);
	}

	KBDLLHOOKSTRUCT keyInfo = *((KBDLLHOOKSTRUCT *) lparam);

	if (keyInfo.flags & LLKHF_INJECTED) {
		// process injected events like normal, because most probably we are injecting them
		logKeyEvent((keyInfo.flags & LLKHF_UP) ? "injected up" : "injected down", keyInfo);
		return CallNextHookEx(NULL, code, wparam, lparam);
	}

	printf("\n");
	logKeyEvent((keyInfo.flags & LLKHF_UP) ? "actual up" : "actual down", keyInfo);


	// remap keys and handle tapping
	if (!bypassMode && dualFunctionKeys(&keyInfo)) return -1;

	// update lastKey and currentKey
	if (keyInfo.scanCode != 541) { // ignore LCONTROL that is sent when pressing ALTGR
		lastKey.key.vk = currentKey.key.vk;
		lastKey.key.scan = currentKey.key.scan;
		lastKey.time = currentKey.time;
		currentKey.key.vk = keyInfo.vkCode;
		currentKey.key.scan = keyInfo.scanCode;
		currentKey.time = time(NULL);
	}

	if (write_event(keyInfo)) return -1;

	return CallNextHookEx(NULL, code, wparam, lparam);
}

DWORD WINAPI hookThreadMain(void *user)
{
	HINSTANCE base = GetModuleHandle(NULL);
	MSG msg;

	if (!base) {
		if (!(base = LoadLibrary((wchar_t *) user))) {
			return 1;
		}
	}
	/* Installs an application-defined hook procedure into a hook chain
	 * 1st Parameter idHook: WH_KEYBOARD_LL - The type of hook procedure to be installed.
	 * Installs a hook procedure that monitors low-level keyboard input events.
	 * 2nd Parameter lpfn: LowLevelKeyboardProc - A pointer to the hook procedure.
	 * 3rd Parameter hMod: hExe - A handle to the DLL containing the hook procedure pointed to by the lpfn parameter.
	 * 4th Parameter dwThreadId: 0 - the hook procedure is associated with all existing threads running.
	 * If the function succeeds, the return value is the handle to the hook procedure.
	 * If the function fails, the return value is NULL.
	 */
	keyhook = SetWindowsHookEx(WH_KEYBOARD_LL, keyevent, base, 0);

	/* Message loop retrieves messages from the thread's message queue and dispatches them to the appropriate window procedures.
	 * For more info http://msdn.microsoft.com/en-us/library/ms644928%28v=VS.85%29.aspx#creating_loop
	 * Retrieves a message from the calling thread's message queue.
	 */
	while (GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	/* To free system resources associated with the hook and removes a hook procedure installed in a hook chain
	 * Parameter hhk: hKeyHook - A handle to the hook to be removed.
	 */
	UnhookWindowsHookEx(keyhook);

	return 0;
}

void exitApplication()
{
	trayicon_remove();
	PostQuitMessage(0);
}

bool fileExists(LPCSTR szPath)
{
	DWORD dwAttrib = GetFileAttributesA(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES
	    && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

int main(int argc, char *argv[])
{
	/**
	* find settings.ini (in same folder as neo-llkh.exe)
	*/
	// get path of neo-llkh.exe
	char ini[256];
	GetModuleFileNameA(NULL, ini, 256);
	//printf("exe: %s\n", ini);
	char * pch;
	// find last \ in path
	pch = strrchr(ini, '\\');
	// replace neo-llkh.exe by settings.ini
	strcpy(pch+1, "settings.ini");
	//printf("ini: %s\n", ini);

	/**
	* If settings.ini exists, read in settings.
	* Otherwise check for command line parameters.
	*/
	if (fileExists(ini)) {
		char returnValue[100];

		GetPrivateProfileStringA("Settings", "layout", "neo", layout, 100, ini);

		GetPrivateProfileStringA("Settings", "symmetricalLevel3Modifiers", "0", returnValue, 100, ini);
		quoteAsMod3R = (strcmp(returnValue, "1") == 0);

		GetPrivateProfileStringA("Settings", "returnKeyAsMod3R", "0", returnValue, 100, ini);
		returnAsMod3R = (strcmp(returnValue, "1") == 0);

		GetPrivateProfileStringA("Settings", "tabKeyAsMod4L", "0", returnValue, 100, ini);
		tabAsMod4L = (strcmp(returnValue, "1") == 0);

		GetPrivateProfileStringA("Settings", "capsLockEnabled", "0", returnValue, 100, ini);
		capsLockEnabled = (strcmp(returnValue, "1") == 0);

		GetPrivateProfileStringA("Settings", "shiftLockEnabled", "0", returnValue, 100, ini);
		shiftLockEnabled = (strcmp(returnValue, "1") == 0);

		GetPrivateProfileStringA("Settings", "level4LockEnabled", "0", returnValue, 100, ini);
		level4LockEnabled = (strcmp(returnValue, "1") == 0);

		GetPrivateProfileStringA("Settings", "qwertzForShortcuts", "0", returnValue, 100, ini);
		qwertzForShortcuts = (strcmp(returnValue, "1") == 0);

		GetPrivateProfileStringA("Settings", "swapLeftCtrlAndLeftAlt", "0", returnValue, 100, ini);
		swapLeftCtrlAndLeftAlt = (strcmp(returnValue, "1") == 0);

		GetPrivateProfileStringA("Settings", "swapLeftCtrlLeftAltAndLeftWin", "0", returnValue, 100, ini);
		swapLeftCtrlLeftAltAndLeftWin = (strcmp(returnValue, "1") == 0);

		GetPrivateProfileStringA("Settings", "supportLevels5and6", "0", returnValue, 100, ini);
		supportLevels5and6 = (strcmp(returnValue, "1") == 0);

		GetPrivateProfileStringA("Settings", "capsLockAsEscape", "0", returnValue, 100, ini);
		capsLockAsEscape = (strcmp(returnValue, "1") == 0);

		GetPrivateProfileStringA("Settings", "mod3RAsReturn", "0", returnValue, 100, ini);
		mod3RAsReturn = (strcmp(returnValue, "1") == 0);

		GetPrivateProfileStringA("Settings", "mod4LAsTab", "0", returnValue, 100, ini);
		mod4LAsTab = (strcmp(returnValue, "1") == 0);

		GetPrivateProfileStringA("Settings", "debugWindow", "0", returnValue, 100, ini);
		debugWindow = (strcmp(returnValue, "1") == 0);

		if (capsLockEnabled)
			shiftLockEnabled = false;

		if (swapLeftCtrlLeftAltAndLeftWin)
			swapLeftCtrlAndLeftAlt = false;

		if (debugWindow)
			// Open Console Window to see printf output
			SetStdOutToNewConsole();

		printf("\nEinstellungen aus %s:\n", ini);
		printf(" Layout: %s\n", layout);
		printf(" symmetricalLevel3Modifiers: %d\n", quoteAsMod3R);
		printf(" returnKeyAsMod3R: %d\n", returnAsMod3R);
		printf(" tabKeyAsMod4L: %d\n", tabAsMod4L);
		printf(" capsLockEnabled: %d\n", capsLockEnabled);
		printf(" shiftLockEnabled: %d\n", shiftLockEnabled);
		printf(" level4LockEnabled: %d\n", level4LockEnabled);
		printf(" qwertzForShortcuts: %d\n", qwertzForShortcuts);
		printf(" swapLeftCtrlAndLeftAlt: %d\n", swapLeftCtrlAndLeftAlt);
		printf(" swapLeftCtrlLeftAltAndLeftWin: %d\n", swapLeftCtrlLeftAltAndLeftWin);
		printf(" supportLevels5and6: %d\n", supportLevels5and6);
		printf(" capsLockAsEscape: %d\n", capsLockAsEscape);
		printf(" mod3RAsReturn: %d\n", mod3RAsReturn);
		printf(" mod4LAsTab: %d\n", mod4LAsTab);
		printf(" debugWindow: %d\n\n", debugWindow);

	} else {
		printf("\nKeine settings.ini gefunden: %s\n\n", ini);
	}


	if (argc >= 2) {
		printf("Einstellungen von der Kommandozeile:");
		char delimiter[] = "=";
		char *param, *value;
		for (int i=1; i< argc; i++) {
			if (strcmp(argv[i], "neo") == 0
				|| strcmp(argv[i], "adnw") == 0
				|| strcmp(argv[i], "adnwzjf") == 0
				|| strcmp(argv[i], "bone") == 0
				|| strcmp(argv[i], "koy") == 0
				|| strcmp(argv[i], "kou") == 0
				|| strcmp(argv[i], "kou-fw623") == 0
				|| strcmp(argv[i], "vou") == 0
				|| strcmp(argv[i], "qwertz") == 0) {
				strncpy(layout, argv[i], 100);
				printf("\n Layout: %s", layout);

			} else if (strstr(argv[i], "=") != NULL) {
				//printf("\narg%d: %s", i, argv[i]);
				param = strtok(argv[i], delimiter);
				if (param != NULL) {
					value = strtok(NULL, delimiter);
					if (value != NULL) {
						//printf("\n%s ist %s", param, value);
					}
				}

				if (strcmp(param, "debugWindow") == 0) {
					bool debugWindowAlreadyStarted = debugWindow;
					debugWindow = value==NULL ? false : (strcmp(value, "1") == 0);
					if (debugWindow && !debugWindowAlreadyStarted)
						// Open Console Window to see printf output
						SetStdOutToNewConsole();
					printf("\n debugWindow: %d", debugWindow);

				} else if (strcmp(param, "layout") == 0) {
					if (value != NULL) {
						strncpy(layout, value, 100);
						printf("\n Layout: %s", layout);
					}

				} else if (strcmp(param, "symmetricalLevel3Modifiers") == 0) {
					quoteAsMod3R = value==NULL ? false : (strcmp(value, "1") == 0);
					printf("\n symmetricalLevel3Modifiers: %d", quoteAsMod3R);

				} else if (strcmp(param, "returnKeyAsMod3R") == 0) {
					returnAsMod3R = value==NULL ? false : (strcmp(value, "1") == 0);
					printf("\n returnKeyAsMod3R: %d", returnAsMod3R);

				} else if (strcmp(param, "tabKeyAsMod4L") == 0) {
					tabAsMod4L = value==NULL ? false : (strcmp(value, "1") == 0);
					printf("\n tabKeyAsMod4L: %d", tabAsMod4L);

				} else if (strcmp(param, "capsLockEnabled") == 0) {
					capsLockEnabled = value==NULL ? false : (strcmp(value, "1") == 0);
					printf("\n capsLockEnabled: %d", capsLockEnabled);

				} else if (strcmp(param, "shiftLockEnabled") == 0) {
					shiftLockEnabled = value==NULL ? false : (strcmp(value, "1") == 0);
					printf("\n shiftLockEnabled: %d", shiftLockEnabled);

				} else if (strcmp(param, "level4LockEnabled") == 0) {
					level4LockEnabled = value==NULL ? false : (strcmp(value, "1") == 0);
					printf("\n level4LockEnabled: %d", level4LockEnabled);

				} else if (strcmp(param, "qwertzForShortcuts") == 0) {
					qwertzForShortcuts = value==NULL ? false : (strcmp(value, "1") == 0);
					printf("\n qwertzForShortcuts: %d", qwertzForShortcuts);

				} else if (strcmp(param, "swapLeftCtrlAndLeftAlt") == 0) {
					swapLeftCtrlAndLeftAlt = value==NULL ? false : (strcmp(value, "1") == 0);
					printf("\n swapLeftCtrlAndLeftAlt: %d", swapLeftCtrlAndLeftAlt);

				} else if (strcmp(param, "swapLeftCtrlLeftAltAndLeftWin") == 0) {
					swapLeftCtrlLeftAltAndLeftWin = value==NULL ? false : (strcmp(value, "1") == 0);
					printf("\n swapLeftCtrlLeftAltAndLeftWin: %d", swapLeftCtrlLeftAltAndLeftWin);

				} else if (strcmp(param, "supportLevels5and6") == 0) {
					supportLevels5and6 = value==NULL ? false : (strcmp(value, "1") == 0);
					printf("\n supportLevels5and6: %d", supportLevels5and6);

				} else if (strcmp(param, "capsLockAsEscape") == 0) {
					capsLockAsEscape = value==NULL ? false : (strcmp(value, "1") == 0);
					printf("\n capsLockAsEscape: %d", capsLockAsEscape);

				} else if (strcmp(param, "mod3RAsReturn") == 0) {
					mod3RAsReturn = value==NULL ? false : (strcmp(value, "1") == 0);
					printf("\n mod3RAsReturn: %d", mod3RAsReturn);

				} else if (strcmp(param, "mod4LAsTab") == 0) {
					mod4LAsTab = value==NULL ? false : (strcmp(value, "1") == 0);
					printf("\n mod4LAsTab: %d", mod4LAsTab);

				} else {
					printf("\nUnbekannter Parameter:%s", param);
				}
			} else {
				printf("\ninvalid arg: %s", argv[i]);
			}
		}
	}
	printf("\n\n");

	if (quoteAsMod3R)
		// use ä/quote key instead of #/backslash key as right level 3 modifier
		scanCodeMod4R = SCANCODE_QUOTE_KEY;
	else if (returnAsMod3R)
		// use return key instead of #/backslash as right level 3 modifier
		// (might be useful for US keyboards because the # key is missing there)
		scanCodeMod4R = SCANCODE_RETURN_KEY;

	if (tabAsMod4L)
		// use tab key instead of < key as left level 4 modifier
		// (might be useful for US keyboards because the < key is missing there)
		scanCodeMod4L = SCANCODE_TAB_KEY;

	if (swapLeftCtrlAndLeftAlt || swapLeftCtrlLeftAltAndLeftWin)
		// catch ctrl-c because it will send keydown for ctrl
		// but then keyup for alt. Then ctrl would be locked.
		SetConsoleCtrlHandler(CtrlHandler, TRUE);

	initLayout();

	setbuf(stdout, NULL);

	DWORD tid;

	/* Retrieves a module handle for the specified module.
	 * parameter is NULL, GetModuleHandle returns a handle to the file used to create the calling process (.exe file).
	 * If the function succeeds, the return value is a handle to the specified module.
	 * If the function fails, the return value is NULL.
	 */
	HINSTANCE hInstance = GetModuleHandle(NULL);
	trayicon_init(LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON)), APPNAME);
	trayicon_add_item(NULL, &toggleBypassMode);
	trayicon_add_item("Exit", &exitApplication);

	/* CreateThread function Creates a thread to execute within the virtual address space of the calling process.
	 * 1st Parameter lpThreadAttributes:  NULL - Thread gets a default security descriptor.
	 * 2nd Parameter dwStackSize:  0  - The new thread uses the default size for the executable.
	 * 3rd Parameter lpStartAddress:  KeyLogger - A pointer to the application-defined function to be executed by the thread
	 * 4th Parameter lpParameter:  argv[0] -  A pointer to a variable to be passed to the thread
	 * 5th Parameter dwCreationFlags: 0 - The thread runs immediately after creation.
	 * 6th Parameter pThreadId(out parameter): NULL - the thread identifier is not returned
	 * If the function succeeds, the return value is a handle to the new thread.
	 */
	HANDLE thread = CreateThread(0, 0, hookThreadMain, argv[0], 0, &tid);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		// Translates virtual-key messages into character messages.
		TranslateMessage(&msg);
		// Dispatches a message to a window procedure.
		DispatchMessage(&msg);
	}
	return 0;
}
