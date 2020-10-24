#ifndef MOUSE_H_
#define MOUSE_H_

#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include "scancode.h"
#include "config.h"
#include "key-handler.h"

void resetMouseNavigationState();
bool navigateMouse(KBDLLHOOKSTRUCT keyInfo);

typedef struct NavigationMapping {
  DWORD scan;
  int period; // 0 ... only run once
  int value;
  int shiftedValue; // value when shift is active
  void (*timerEvent)(const struct NavigationMapping *key); // gets attached to timer
  void (*releaseEvent)(const struct NavigationMapping *key); // gets called on key release
  HANDLE timer;
  struct NavigationMapping *n;
} NavigationMapping;

#endif /* MOUSE_H_ */
