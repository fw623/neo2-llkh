/**
 * convention: functions return `true` if they match the input and it
 * should not be processed further; `false` otherwise
 */

#define UNICODE

#include "key-handler.h"

#define SCANCODE_TAB_KEY 15
#define SCANCODE_CAPSLOCK_KEY 58
#define SCANCODE_LOWER_THAN_KEY 86 // <
#define SCANCODE_QUOTE_KEY 40      // Ä
#define SCANCODE_HASH_KEY 43       // #
#define SCANCODE_RETURN_KEY 28
#define SCANCODE_ANY_ALT_KEY 56        // Alt or AltGr

/**
 * Map a key scancode to the char that should be displayed after typing
 **/
TCHAR mapScanCodeToChar(unsigned level, char in) {
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
DWORD dwFlagsFromKeyInfo(KBDLLHOOKSTRUCT keyInfo) {
	DWORD dwFlags = 0;
	if (keyInfo.flags & LLKHF_EXTENDED) dwFlags |= KEYEVENTF_EXTENDEDKEY;
	if (keyInfo.flags & LLKHF_UP) dwFlags |= KEYEVENTF_KEYUP;
	return dwFlags;
}

void sendKey(KBDLLHOOKSTRUCT keyInfo) {
	keybd_event(keyInfo.vkCode, keyInfo.scanCode, dwFlagsFromKeyInfo(keyInfo), keyInfo.dwExtraInfo);
}

void sendDown(BYTE vkCode, BYTE scanCode, bool isExtendedKey) {
	keybd_event(vkCode, scanCode, (isExtendedKey ? KEYEVENTF_EXTENDEDKEY : 0), 0);
}

void sendUp(BYTE vkCode, BYTE scanCode, bool isExtendedKey) {
	keybd_event(vkCode, scanCode, (isExtendedKey ? KEYEVENTF_EXTENDEDKEY : 0) | KEYEVENTF_KEYUP, 0);
}

void sendDownUp(BYTE vkCode, BYTE scanCode, bool isExtendedKey) {
	sendDown(vkCode, scanCode, isExtendedKey);
	sendUp(vkCode, scanCode, isExtendedKey);
}

void sendUnicodeChar(TCHAR key, KBDLLHOOKSTRUCT keyInfo) {
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
void sendChar(TCHAR key, KBDLLHOOKSTRUCT keyInfo) {
	SHORT keyScanResult = VkKeyScanEx(key, GetKeyboardLayout(0));

	if (keyScanResult == -1 || shiftLockActive || capsLockActive || modStates.mod4.isLocked
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
		if (alt) sendDown(VK_MENU, 56, false);
		if (shift) sendDown(VK_SHIFT, 42, false);

		sendKey(keyInfo);

		if (shift) sendUp(VK_SHIFT, 42, false);
		if (alt) sendUp(VK_MENU, 56, false);
		if (ctrl) sendUp(VK_CONTROL, 29, false);
		if (altgr) sendUp(VK_RMENU, 56, false);
	}
}

/**
 * Send a usually dead key by injecting space after (on down).
 * This will add an actual space if actual dead key is followed by "dead" key with this
 **/
void commitDeadKey(KBDLLHOOKSTRUCT keyInfo) {
	if (!(keyInfo.flags & LLKHF_UP)) sendDownUp(VK_SPACE, 57, false);
}

bool isInputKey(KBDLLHOOKSTRUCT actual, InputKey desired) {
	return actual.vkCode == desired.vk && actual.scanCode == desired.scan;
}

bool handleLayer2SpecialCases(KBDLLHOOKSTRUCT keyInfo) {
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

bool handleLayer3SpecialCases(KBDLLHOOKSTRUCT keyInfo) {
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

bool handleLayer4SpecialCases(KBDLLHOOKSTRUCT keyInfo) {
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

	// remap RETURN to 0 so we can use it when tapping AltGr
	if (strcmp(layout, "kou-fw623") == 0 && (keyInfo.scanCode == 28))	{
		printf("\nhello %d \n\n", keyInfo.scanCode);
		if (keyInfo.flags & LLKHF_UP) {
			sendUp(VK_NUMPAD0, 82, false);
		} else {
			sendDown(VK_NUMPAD0, 82, false);
		}
		return true;
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

bool isSystemKeyPressed() {
	return modStates.lCtrlIsPressed
		|| modStates.lWinIsPressed
		|| modStates.lAltIsPressed
		|| modStates.rAltIsPressed
		|| modStates.rWinIsPressed
		|| modStates.rCtrlIsPressed;
}

bool isLetter(TCHAR key) {
	return (key >= 65 && key <= 90  // A-Z
		|| key >= 97 && key <= 122 // a-z
		|| key == L'ä' || key == L'ö'
		|| key == L'ü' || key == L'ß'
		|| key == L'Ä' || key == L'Ö'
		|| key == L'Ü' || key == L'ẞ');
}

void toggleShiftLock() {
	shiftLockActive = !shiftLockActive;
	printf("Shift lock %s!\n", shiftLockActive ? "activated" : "deactivated");
}

void toggleCapsLock() {
	capsLockActive = !capsLockActive;
	printf("Caps lock %s!\n", capsLockActive ? "activated" : "deactivated");
}

void logKeyEvent(char *desc, KBDLLHOOKSTRUCT keyInfo) {
	char vkCodeLetter[4] = {'(', keyInfo.vkCode, ')', 0};
	char *keyName = "";

	if (keyInfo.vkCode == modConfig.shift.left.vk) {
		keyName = "(Shift left)";
	} else if (keyInfo.vkCode == modConfig.shift.right.vk) {
		keyName = "(Shift right)";
	} else if (keyInfo.vkCode == modConfig.mod3.left.vk) {
		keyName = "(M3 left)";
	} else if (keyInfo.vkCode == modConfig.mod3.right.vk) {
		keyName = "(M3 right)";
	} else if (keyInfo.vkCode == modConfig.mod4.left.vk) {
		keyName = "(M4 left)";
	} else if (keyInfo.vkCode == modConfig.mod4.right.vk) {
		keyName = "(M4 right)";
	} else if (keyInfo.vkCode == VK_SHIFT) {
		keyName = "(Shift)";
	} else if (keyInfo.vkCode == VK_CONTROL) {
		keyName = "(Ctrl)";
	} else if (keyInfo.vkCode == VK_LCONTROL) {
		keyName = "(Ctrl left)";
	} else if (keyInfo.vkCode == VK_RCONTROL) {
		keyName = "(Ctrl right)";
	} else if (keyInfo.vkCode == VK_MENU) {
		keyName = "(Alt)";
	} else if (keyInfo.vkCode == VK_LMENU) {
		keyName = "(Alt left)";
	} else if (keyInfo.vkCode == VK_RMENU) {
		keyName = "(Alt right)";
	} else if (keyInfo.vkCode == VK_LWIN) {
		keyName = "(Win left)";
	} else if (keyInfo.vkCode == VK_RWIN) {
		keyName = "(Win right)";
	} else if (keyInfo.vkCode == VK_BACK) {
		keyName = "(Backspace)";
	} else if (keyInfo.vkCode == VK_RETURN) {
		keyName = "(Return)";
	}

	char *shiftLockCapsLockInfo = shiftLockActive ? " [shift lock active]"
						: (capsLockActive ? " [caps lock active]" : "");
	char *level4LockInfo = modStates.mod4.isLocked ? " [level4 lock active]" : "";
	char *vkPacket = (desc=="injected" && keyInfo.vkCode == VK_PACKET) ? " (VK_PACKET)" : "";
	char *upDown = keyInfo.flags & LLKHF_UP ? "up" : "down";

	printf(
		"%-4s %-9s | sc:%03u vk:0x%02X flags:0x%02X extra:%d %s%s%s%s\n",
		upDown, desc, keyInfo.scanCode, keyInfo.vkCode, keyInfo.flags, keyInfo.dwExtraInfo,
		keyName, shiftLockCapsLockInfo, level4LockInfo, vkPacket
	);
}

bool modIsActive (NeoModState state) {
	return (state.leftIsPressed || state.rightIsPressed) != state.isLocked;
}

unsigned getLevel() {
	unsigned level = 1;

	if (modIsActive(modStates.shift))
		level = 2;
	if (modIsActive(modStates.mod3))
		level = (supportLevels5and6 && level == 2) ? 5 : 3;
	if (modIsActive(modStates.mod4))
		level = (supportLevels5and6 && level == 3) ? 6 : 4;

	return level;
}

// returns `false` because the keys will be sent by the default handler;
// must be executed after the neoModState updating because those could be systemKeys
boolean updateSystemKeyStates(KBDLLHOOKSTRUCT keyInfo) {
	bool newValue = !(keyInfo.flags & LLKHF_UP);

	if (isInputKey(keyInfo, systemKey.lCtrl)) modStates.lCtrlIsPressed = newValue;
	else if (isInputKey(keyInfo, systemKey.lWin)) modStates.lWinIsPressed = newValue;
	else if (isInputKey(keyInfo, systemKey.lAlt)) modStates.lAltIsPressed = newValue;
	else if (isInputKey(keyInfo, systemKey.rAlt)) modStates.rAltIsPressed = newValue;
	else if (isInputKey(keyInfo, systemKey.rWin)) modStates.rWinIsPressed = newValue;
	else if (isInputKey(keyInfo, systemKey.rCtrl)) modStates.rCtrlIsPressed = newValue;

	return false;
}

void toggleModLockConditionally(bool *isLocked, bool isKeyUp, bool ignoreLocking) {
	if (!isKeyUp && !ignoreLocking) *isLocked = !(*isLocked);
}

// also toggle CapsLock (for keyboard LED)
void toggleModLockConditionallyShift(bool *isLocked, bool isKeyUp, bool ignoreLocking) {
	if (!isKeyUp && !ignoreLocking) {
		// TODO: fix NumberRow characters during lock
		// TODO: fix - shiftLock doesn't work with this approach (but did in the published version)

		// NOTE: when sending CapsLock like this, we mustn't use the internal isLocked state
		// because these 2 interfere with each other
		// the probably cleaner solution is to use only the internal state and ignore the CapsLock LED
		// or maybe when toggling bypassMode, reset lock state

		sendDownUp(VK_CAPITAL, 58, false);

		if (capsLockEnabled) {
			toggleCapsLock();
		} else {
			toggleShiftLock();
		}
	}
}

// ignoreLocking - is used for shift key during bypassMode
bool handleModKey(KBDLLHOOKSTRUCT keyInfo, ModTypeConfig mod, NeoModState *state, bool ignoreLocking, void (*toggleLockConditionally)(bool *, bool, bool)) {
	bool isKeyUp = keyInfo.flags & LLKHF_UP;

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

// updates system key and layerLock states; writes key
bool updateStatesAndWriteKey(KBDLLHOOKSTRUCT keyInfo, unsigned level) {
	if (handleModKey(keyInfo, modConfig.mod3, &modStates.mod3, false, toggleModLockConditionally)) return true;
	if (handleModKey(keyInfo, modConfig.mod4, &modStates.mod4, false, toggleModLockConditionally)) return true;

	// must be after the neoModState updating because those could be systemKeys
	if (updateSystemKeyStates(keyInfo)) return true;

	// do nothing else if it has only the isExtendedKey bit set (why?)
	if (keyInfo.flags == 1) return false;

	if (level == 2 && handleLayer2SpecialCases(keyInfo)) return true;
	if (level == 3 && handleLayer3SpecialCases(keyInfo)) return true;
	if (level == 4 && handleLayer4SpecialCases(keyInfo)) return true;

	// Numeric keypad -> don't remap
	if (keyInfo.vkCode >= 0x60 && keyInfo.vkCode <= 0x6F) return false;
	// numbers 0 to 9 -> don't remap
	if (level == 1 && keyInfo.vkCode >= 0x30 && keyInfo.vkCode <= 0x39) return false;

	if (!(qwertzForShortcuts && isSystemKeyPressed())) {
		TCHAR key = mapScanCodeToChar(level, keyInfo.scanCode);
		if (capsLockActive && (level == 1 || level == 2) && isLetter(key)) {
			key = mapScanCodeToChar(level==1 ? 2 : 1, keyInfo.scanCode);
		}

		if (key != 0 && (keyInfo.flags & LLKHF_INJECTED) == 0) {
			// if key must be mapped
			int character = MapVirtualKeyA(keyInfo.vkCode, MAPVK_VK_TO_CHAR);
			printf("     %-9s | sc:%03d %c->%c [0x%04X] (level %u)\n", "mapped", keyInfo.scanCode, character, key, key, level);
			sendChar(key, keyInfo);
			return true;
		}
	}

	return false;
}

void toggleBypassMode() {
	bypassMode = !bypassMode;

	HINSTANCE hInstance = GetModuleHandle(NULL);
	HICON icon = bypassMode
		? LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON_DISABLED))
		: LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));

	trayicon_change_icon(icon);
	printf("%i bypass mode \n", bypassMode);
}

bool writeEvent(const KBDLLHOOKSTRUCT keyInfo, unsigned level) {
	WPARAM wparam = (keyInfo.flags & LLKHF_UP) ? WM_KEYUP : WM_KEYDOWN;

	// handle shift here; necessary because we need to track it also in bypassMode
	if (handleModKey(keyInfo, modConfig.shift, &modStates.shift, bypassMode, toggleModLockConditionallyShift)) {
		return false;
	}

	// Shift + Pause
	if (wparam == WM_KEYDOWN && keyInfo.vkCode == VK_PAUSE && (modStates.shift.leftIsPressed || modStates.shift.rightIsPressed)) {
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
		sendKey(keyInfo);
		return true;
	}

	// skip LCONTROL sent by pressing ALTGR
	if (keyInfo.scanCode == 541) return true;

	logKeyEvent("key", keyInfo);
	if (updateStatesAndWriteKey(keyInfo, level)) return true;

	// send the incoming key if nothing matches
	sendKey(keyInfo);
	return true;
}

__declspec(dllexport)
LRESULT CALLBACK keyevent(int code, WPARAM wparam, LPARAM lparam) {
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
		// ignore injected events because they most probably come from us
		logKeyEvent("injected", keyInfo);
		return CallNextHookEx(NULL, code, wparam, lparam);
	}

	printf("\n");
	logKeyEvent("input", keyInfo);

	// remap keys and handle tapping
	if (!bypassMode && dual_function_keys(&keyInfo)) return -1;

	// update lastKey and currentKey
	lastKey.key.vk = currentKey.key.vk;
	lastKey.key.scan = currentKey.key.scan;
	lastKey.time = currentKey.time;
	currentKey.key.vk = keyInfo.vkCode;
	currentKey.key.scan = keyInfo.scanCode;
	currentKey.time = 0;

	if (getLevel() == 6) {

		int slow = 25;
		int medium = 250;
		int fast = 1000;

		switch (keyInfo.scanCode) {
			case 18: // up
				if (keyInfo.flags & LLKHF_UP) return -1;
				mouse_event(MOUSEEVENTF_MOVE, 0, -slow, 0, 0);
				break;
			case 31: // left
				if (keyInfo.flags & LLKHF_UP) return -1;
				mouse_event(MOUSEEVENTF_MOVE, -slow, 0, 0, 0);
				break;
			case 32: // down
				if (keyInfo.flags & LLKHF_UP) return -1;
				mouse_event(MOUSEEVENTF_MOVE, 0, slow, 0, 0);
				break;
			case 33: // right
				if (keyInfo.flags & LLKHF_UP) return -1;
				mouse_event(MOUSEEVENTF_MOVE, slow, 0, 0, 0);
				break;

			case 17: // 8*up
				if (keyInfo.flags & LLKHF_UP) return -1;
				mouse_event(MOUSEEVENTF_MOVE, 0, -medium, 0, 0);
				break;
			case 19: // 8*down
				if (keyInfo.flags & LLKHF_UP) return -1;
				mouse_event(MOUSEEVENTF_MOVE, 0, medium, 0, 0);
				break;
			case 30: // home
				if (keyInfo.flags & LLKHF_UP) return -1;
				mouse_event(MOUSEEVENTF_MOVE, -medium, 0, 0, 0);
				break;
			case 34: // end
				if (keyInfo.flags & LLKHF_UP) return -1;
				mouse_event(MOUSEEVENTF_MOVE, medium, 0, 0, 0);
				break;

			case 36: // j
				mouse_event((keyInfo.flags & LLKHF_UP) ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
				break;
			case 37: // k
				mouse_event((keyInfo.flags & LLKHF_UP) ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
				break;
			case 23: // i
				if (keyInfo.flags & LLKHF_UP) return -1;
				mouse_event(MOUSEEVENTF_WHEEL, 0, 0, WHEEL_DELTA, 0);
				break;
			case 24: // o
				if (keyInfo.flags & LLKHF_UP) return -1;
				mouse_event(MOUSEEVENTF_WHEEL, 0, 0, -WHEEL_DELTA, 0);
				break;
		}
		return -1;
	}

	if (writeEvent(keyInfo, getLevel())) return -1;

	return CallNextHookEx(NULL, code, wparam, lparam);
}
