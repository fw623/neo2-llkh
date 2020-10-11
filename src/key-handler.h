#ifndef KEY_HANDLER_H_
#define KEY_HANDLER_H_

#include <windows.h>
#include <stdbool.h>
#include <stdio.h>
#include "resources.h"
#include "config.h"
#include "states.h"
#include "tray-icon.h"
#include "dfk.h"

LRESULT CALLBACK keyevent(int code, WPARAM wparam, LPARAM lparam);

void toggleBypassMode();

bool isInputKey(KBDLLHOOKSTRUCT actual, InputKey desired);

bool writeEvent(const KBDLLHOOKSTRUCT keyInfo, unsigned level);

unsigned getLevel();

#endif /* KEY_HANDLER_H_ */
