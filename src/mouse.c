#include "mouse.h"

typedef struct NavigationMapping {
  DWORD scan;
  int period; // 0 ... only run once
  int value;
  void (*timerEvent)(struct NavigationMapping *key); // gets attached to timer
  void (*releaseEvent)(struct NavigationMapping *key); // gets called on key release
  HANDLE timer;
  struct NavigationMapping *n;
} NavigationMapping;

void moveUp(NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_MOVE, 0, -key->value, 0, 0);
}

void moveLeft(NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_MOVE, -key->value, 0, 0, 0);
}

void moveDown(NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_MOVE, 0, key->value, 0, 0);
}

void moveRight(NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_MOVE, key->value, 0, 0, 0);
}

void scrollUp(NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_WHEEL, 0, 0, WHEEL_DELTA * key->value, 0);
}

void scrollDown(NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_WHEEL, 0, 0, -WHEEL_DELTA * key->value, 0);
}

void pressLeft(NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
}

void releaseLeft(NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

void pressRight(NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
}

void releaseRight(NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
}

NavigationMapping rightPress = { 37,  0,   0, pressRight, releaseRight, NULL, NULL };
NavigationMapping leftPress  = { 36,  0,   0, pressLeft, releaseLeft, NULL, &rightPress };
NavigationMapping downScroll = { 24, 33,   0, scrollDown, NULL, NULL, &leftPress };
NavigationMapping upScroll   = { 23, 33,   0, scrollUp, NULL, NULL, &downScroll };
NavigationMapping rightFast  = { 34, 33, 100, moveRight, NULL, NULL, &upScroll };
NavigationMapping downFast   = { 19, 33, 100, moveDown, NULL, NULL, &rightFast };
NavigationMapping leftFast   = { 30, 33, 100, moveLeft, NULL, NULL, &downFast };
NavigationMapping upFast     = { 17, 33, 100, moveUp, NULL, NULL, &leftFast };
NavigationMapping rightSlow  = { 33, 33,  25, moveRight, NULL, NULL, &upFast };
NavigationMapping downSlow   = { 32, 33,  25, moveDown, NULL, NULL, &rightSlow };
NavigationMapping leftSlow   = { 31, 33,  25, moveLeft, NULL, NULL, &downSlow };
NavigationMapping upSlow     = { 18, 33,  25, moveUp, NULL, NULL, &leftSlow };

NavigationMapping *mapping = &upSlow;

void createTimerIfndef(NavigationMapping *key) {
  if (!key->timer) {
    if (!CreateTimerQueueTimer(&key->timer, NULL, (WAITORTIMERCALLBACK)key->timerEvent, key, 0, key->period, 0)) {
      printf("!!! timer creation failed: %d\n", GetLastError());
    }
  }
}

void deleteTimerIfdef(NavigationMapping *key) {
  if (key->timer) {
    if (!DeleteTimerQueueTimer(NULL, key->timer, NULL)) {
      printf("!!! timer deletion failed: %d\n", GetLastError());
    }
    key->timer = NULL;
  }
}

void resetMouseNavigationState() {
  for (NavigationMapping *m = mapping; m != NULL; m = m->n) {
    deleteTimerIfdef(m);
  }
}

bool navigateMouse(KBDLLHOOKSTRUCT keyInfo) {
  bool isKeyUp = keyInfo.flags & LLKHF_UP;
  int slow = (modStates.shift.leftIsPressed || modStates.shift.rightIsPressed) ? 4 : 25;

  NavigationMapping *curr = NULL;
	for (curr = mapping; curr && curr->scan != keyInfo.scanCode; curr = curr->n);

  // ignore if we don't have a mapping for this key
  if (curr == NULL) return false;

  if (!isKeyUp) {
    createTimerIfndef(curr);
  } else {
    deleteTimerIfdef(curr);
    if (curr->releaseEvent) curr->releaseEvent(curr);
  }

  return true;
}
