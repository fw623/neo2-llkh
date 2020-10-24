#ifndef MOUSE_H_
#define MOUSE_H_

#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include "config.h"
#include "key-handler.h"

void resetMouseNavigationState();
bool navigateMouse(KBDLLHOOKSTRUCT keyInfo);


#endif /* MOUSE_H_ */
