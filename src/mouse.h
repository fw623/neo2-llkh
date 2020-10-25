#ifndef MOUSE_H_
#define MOUSE_H_

#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include "scancode.h"
#include "config.h"
#include "key-handler.h"

void resetNavigationState();
bool handleNavigationLayer(KBDLLHOOKSTRUCT keyInfo);

typedef struct NavigationMapping {
  DWORD scan;
  int period; // 0 ... only run once
  int slowValue; // value when only slowKey is pressed
  int value;
  int fastValue; // value when only fastKey is pressed
  void (*timerEvent)(const struct NavigationMapping *key); // gets attached to timer
  void (*releaseEvent)(const struct NavigationMapping *key); // gets called on key release
  HANDLE timer;
  struct NavigationMapping *n;
} NavigationMapping;

#endif /* MOUSE_H_ */
