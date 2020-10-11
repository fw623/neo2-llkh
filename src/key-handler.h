#ifndef KEY_HANDLER_H_
#define KEY_HANDLER_H_

#include <windows.h>
#include <stdbool.h>
#include <stdio.h>
#include "resources.h"
#include "config.h"
#include "states.h"
#include "trayicon.h"

LRESULT CALLBACK keyevent(int code, WPARAM wparam, LPARAM lparam);

void toggleBypassMode();

#endif /* KEY_HANDLER_H_ */
