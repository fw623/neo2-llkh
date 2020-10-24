#include "mouse.h"

DWORD slowKeyScan = SCAN_P;
volatile bool slowKeyPressed = false;

int getValue(const NavigationMapping *key) {
  return slowKeyPressed ? key->slowValue : key->value;
}

void moveUp(const NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_MOVE, 0, -getValue(key), 0, 0);
}

void moveLeft(const NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_MOVE, -getValue(key), 0, 0, 0);
}

void moveDown(const NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_MOVE, 0, getValue(key), 0, 0);
}

void moveRight(const NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_MOVE, getValue(key), 0, 0, 0);
}

void scrollUp(const NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_WHEEL, 0, 0, getValue(key), 0);
}

void scrollDown(const NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_WHEEL, 0, 0, -getValue(key), 0);
}

void pressLeft(const NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
}

void releaseLeft(const NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

void pressMiddle(const NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
}

void releaseMiddle(const NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
}

void pressRight(const NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
}

void releaseRight(const NavigationMapping *key) {
  mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
}

NavigationMapping rightPress  = { SCAN_M,  0,   0,   0, pressRight, releaseRight, NULL, NULL };
NavigationMapping middlePress = { SCAN_N,  0,   0,   0, pressMiddle, releaseMiddle, NULL, &rightPress };
NavigationMapping leftPress   = { SCAN_J,  0,   0,   0, pressLeft, releaseLeft, NULL, &middlePress };
NavigationMapping downScroll  = { SCAN_K, 50, 140,  70, scrollDown, NULL, NULL, &leftPress };
NavigationMapping upScroll    = { SCAN_I, 50, 140,  70, scrollUp, NULL, NULL, &downScroll };
NavigationMapping rightFast   = { SCAN_G, 33, 120,  60, moveRight, NULL, NULL, &upScroll };
NavigationMapping downFast    = { SCAN_R, 33, 120,  60, moveDown, NULL, NULL, &rightFast };
NavigationMapping leftFast    = { SCAN_A, 33, 120,  60, moveLeft, NULL, NULL, &downFast };
NavigationMapping upFast      = { SCAN_W, 33, 120,  60, moveUp, NULL, NULL, &leftFast };
NavigationMapping rightSlow   = { SCAN_F, 33,  25,   4, moveRight, NULL, NULL, &upFast };
NavigationMapping downSlow    = { SCAN_D, 33,  25,   4, moveDown, NULL, NULL, &rightSlow };
NavigationMapping leftSlow    = { SCAN_S, 33,  25,   4, moveLeft, NULL, NULL, &downSlow };
NavigationMapping upSlow      = { SCAN_E, 33,  25,   4, moveUp, NULL, NULL, &leftSlow };

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

  // update slowKey status
  if (keyInfo.scanCode == slowKeyScan) {
    slowKeyPressed = !isKeyUp;
    return true;
  }

  // find mapping of pressed key
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
