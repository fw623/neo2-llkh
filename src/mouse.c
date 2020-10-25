#include "mouse.h"

volatile bool slowKeyPressed = false;
volatile bool fastKeyPressed = false;

int getValue(const NavigationMapping *key) {
  if (slowKeyPressed && !fastKeyPressed) return key->slowValue;
  if (!slowKeyPressed && fastKeyPressed) return key->fastValue;
  return key->value;
}

void moveUp        (const NavigationMapping *key) { mouse_event(MOUSEEVENTF_MOVE,              0, -getValue(key),              0, 0); }
void moveDown      (const NavigationMapping *key) { mouse_event(MOUSEEVENTF_MOVE,              0,  getValue(key),              0, 0); }
void moveLeft      (const NavigationMapping *key) { mouse_event(MOUSEEVENTF_MOVE, -getValue(key),              0,              0, 0); }
void moveRight     (const NavigationMapping *key) { mouse_event(MOUSEEVENTF_MOVE,  getValue(key),              0,              0, 0); }
void scrollUp      (const NavigationMapping *key) { mouse_event(MOUSEEVENTF_WHEEL,             0,              0,  getValue(key), 0); }
void scrollDown    (const NavigationMapping *key) { mouse_event(MOUSEEVENTF_WHEEL,             0,              0, -getValue(key), 0); }
void pressLeft     (const NavigationMapping *key) { mouse_event(MOUSEEVENTF_LEFTDOWN,          0,              0,              0, 0); }
void releaseLeft   (const NavigationMapping *key) { mouse_event(MOUSEEVENTF_LEFTUP,            0,              0,              0, 0); }
void pressMiddle   (const NavigationMapping *key) { mouse_event(MOUSEEVENTF_MIDDLEDOWN,        0,              0,              0, 0); }
void releaseMiddle (const NavigationMapping *key) { mouse_event(MOUSEEVENTF_MIDDLEUP,          0,              0,              0, 0); }
void pressRight    (const NavigationMapping *key) { mouse_event(MOUSEEVENTF_RIGHTDOWN,         0,              0,              0, 0); }
void releaseRight  (const NavigationMapping *key) { mouse_event(MOUSEEVENTF_RIGHTUP,           0,              0,              0, 0); }

DWORD slowKeyScan = SCAN_P;
DWORD fastKeyScan = SCAN_Oe;
NavigationMapping mappingMoveUp      = { SCAN_E, 33,  4,  25, 150,      moveUp,          NULL, NULL, NULL                };
NavigationMapping mappingMoveDown    = { SCAN_D, 33,  4,  25, 150,    moveDown,          NULL, NULL, &mappingMoveUp      };
NavigationMapping mappingMoveLeft    = { SCAN_S, 33,  4,  25, 150,    moveLeft,          NULL, NULL, &mappingMoveDown    };
NavigationMapping mappingMoveRight   = { SCAN_F, 33,  4,  25, 150,   moveRight,          NULL, NULL, &mappingMoveLeft    };
NavigationMapping mappingScrollUp    = { SCAN_I, 50, 70, 140, 500,    scrollUp,          NULL, NULL, &mappingMoveRight   };
NavigationMapping mappingScrollDown  = { SCAN_K, 50, 70, 140, 500,  scrollDown,          NULL, NULL, &mappingScrollUp    };
NavigationMapping mappingPressLeft   = { SCAN_J,  0,  0,   0,   0,   pressLeft,   releaseLeft, NULL, &mappingScrollDown  };
NavigationMapping mappingPressMiddle = { SCAN_N,  0,  0,   0,   0, pressMiddle, releaseMiddle, NULL, &mappingPressLeft   };
NavigationMapping mappingPressRight  = { SCAN_M,  0,  0,   0,   0,  pressRight,  releaseRight, NULL, &mappingPressMiddle };

NavigationMapping *mapping = &mappingPressRight;

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

bool updateSpeedKeys(KBDLLHOOKSTRUCT keyInfo) {
  bool isKeyUp = keyInfo.flags & LLKHF_UP;

  if (keyInfo.scanCode == slowKeyScan) {
    slowKeyPressed = !isKeyUp;
    return true;
  } else if (keyInfo.scanCode == fastKeyScan) {
    fastKeyPressed = !isKeyUp;
    return true;
  }

  return false;
}

void resetNavigationState() {
  for (NavigationMapping *m = mapping; m != NULL; m = m->n) {
    deleteTimerIfdef(m);
  }
}

bool handleNavigationLayer(KBDLLHOOKSTRUCT keyInfo) {
  bool isKeyUp = keyInfo.flags & LLKHF_UP;

  if (updateSpeedKeys(keyInfo)) return true;

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
