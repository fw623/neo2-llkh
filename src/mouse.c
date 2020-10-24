#include "mouse.h"

void moveUp(bool isKeyUp, int amount) {
  if (!isKeyUp)
    mouse_event(MOUSEEVENTF_MOVE, 0, -amount, 0, 0);
}

void moveLeft(bool isKeyUp, int amount) {
  if (!isKeyUp)
    mouse_event(MOUSEEVENTF_MOVE, -amount, 0, 0, 0);
}

void moveDown(bool isKeyUp, int amount) {
  if (!isKeyUp)
    mouse_event(MOUSEEVENTF_MOVE, 0, amount, 0, 0);
}

void moveRight(bool isKeyUp, int amount) {
  if (!isKeyUp)
    mouse_event(MOUSEEVENTF_MOVE, amount, 0, 0, 0);
}

void pressLeft(bool isKeyUp, int amount) {
  mouse_event(isKeyUp ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
}

void pressRight(bool isKeyUp, int amount) {
  mouse_event(isKeyUp ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
}

void scrollUp(bool isKeyUp, int amount) {
  if (!isKeyUp)
    mouse_event(MOUSEEVENTF_WHEEL, 0, 0, WHEEL_DELTA * amount, 0);
}

void scrollDown(bool isKeyUp, int amount) {
  if (!isKeyUp)
    mouse_event(MOUSEEVENTF_WHEEL, 0, 0, - WHEEL_DELTA * amount, 0);
}


typedef struct NavigationKey {
  DWORD scan;
  bool isPressed;
  bool repeatPresses;
  void (*sendEvent)(bool isKeyUp, int amount);
  int amount;
  struct NavigationKey *n;
} NavigationKey;

NavigationKey downScroll = { 24, false, true, scrollDown, 1, NULL };
NavigationKey upScroll   = { 23, false, true, scrollUp, 1, &downScroll };
NavigationKey rightPress = { 37, false, false, pressRight, 1, &upScroll };
NavigationKey leftPress  = { 36, false, false, pressLeft, 1, &rightPress };
NavigationKey rightFast  = { 34, false, true, moveRight, 250, &leftPress };
NavigationKey downFast   = { 19, false, true, moveDown, 250, &rightFast };
NavigationKey leftFast   = { 30, false, true, moveLeft, 250, &downFast };
NavigationKey upFast     = { 17, false, true, moveUp, 250, &leftFast };
NavigationKey rightSlow  = { 33, false, true, moveRight, 25, &upFast };
NavigationKey downSlow   = { 32, false, true, moveDown, 25, &rightSlow };
NavigationKey leftSlow   = { 31, false, true, moveLeft, 25, &downSlow };
NavigationKey upSlow     = { 18, false, true, moveUp, 25, &leftSlow };

NavigationKey *mapping = &upSlow;

void resetMouseNavigationState() {
  for (NavigationKey *m = mapping; m != NULL; m = m->n) {
    if (m->isPressed) {
      m->isPressed = false;
      m->sendEvent(true, m->amount);
    }
  }
}



bool navigateMouse(KBDLLHOOKSTRUCT keyInfo) {
  bool isKeyUp = keyInfo.flags & LLKHF_UP;
  // int slow = (modStates.shift.leftIsPressed || modStates.shift.rightIsPressed) ? 4 : 25;
  // int fast = 250;

  NavigationKey *curr = NULL;
	for (curr = mapping; curr && curr->scan != keyInfo.scanCode; curr = curr->n);

  if (curr == NULL) return false;

  curr->isPressed = !isKeyUp;
  curr->sendEvent(isKeyUp, curr->amount);

  for (NavigationKey *m = mapping; m != NULL; m = m->n) {
    if (m == curr) continue;
    if (m->isPressed && m->repeatPresses)
      m->sendEvent(false, m->amount);
  }

  return true;
}
