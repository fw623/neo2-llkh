#include "dfk.h"

KBDLLHOOKSTRUCT newKeyInfo (OutputKey send, DWORD flags) {
	KBDLLHOOKSTRUCT keyInfo = {
		.vkCode = send.vk,
		.scanCode = send.scan,
		.flags = (send.isExtended ? LLKHF_EXTENDED : 0) | (flags & ~LLKHF_EXTENDED), // ignore extended flag of incoming key
		.time = 0,
		.dwExtraInfo = 0
	};
	return keyInfo;
}

void tap(Mapping *m, DWORD flags) {
	Tap *t;
	for (t = m->tap; t; t = t->n) {
		// printf("remapped tap\n");
		writeEvent(newKeyInfo(t->code, flags));
	}
}

void handle_press(Mapping *m, KBDLLHOOKSTRUCT *input) {
	// printf("down state1: %d\n", m->state);
	// state
	switch (m->state) {
		case RELEASED:
			m->state = PRESSED;
			break;
		case PRESSED:
		case DOUBLETAPPED:
		case CONSUMED:
			break;
		case TAPPED:
			m->state = input->time - m->changed < dfkConfig.double_tap_millis
				? DOUBLETAPPED
				: PRESSED;
			break;
	}
	m->changed = input->time;
	// printf("down state2: %d\n", m->state);

	// action
	switch (m->state) {
		case RELEASED:
		case PRESSED:
		case CONSUMED:
			// printf("remapped down\n");
			writeEvent(newKeyInfo(m->hold, input->flags));
			break;
		case TAPPED:
		case DOUBLETAPPED:
			tap(m, 0);
			break;
	}
}

void handle_release(Mapping *m, KBDLLHOOKSTRUCT *input) {
	// printf("up   state1: %d\n", m->state);
	// state
	switch (m->state) {
		case RELEASED:
		case TAPPED:
			break;
		case PRESSED:
			m->state = input->time - m->changed < dfkConfig.tap_millis
				? TAPPED
				: RELEASED;
			break;
		case DOUBLETAPPED:
		case CONSUMED:
			m->state = RELEASED;
			break;
	}
	m->changed = input->time;
	// printf("up   state2: %d\n", m->state);

	// action
	switch (m->state) {
		case RELEASED:
		case PRESSED:
		case CONSUMED:
			// printf("remapped up\n");
			writeEvent(newKeyInfo(m->hold, input->flags));
			break;
		case TAPPED:
			// printf("remapped release\n");
			// release "hold"
			writeEvent(newKeyInfo(m->hold, input->flags));
			// synthesize tap
			tap(m, 0);
			tap(m, LLKHF_UP);
			break;
		case DOUBLETAPPED:
			tap(m, LLKHF_UP);
			break;
	}
}

void consume_pressed() {
	// state
	for (Mapping *m = dfkConfig.m; m; m = m->n) {
		switch (m->state) {
			case PRESSED:
				m->state = CONSUMED;
				break;
			case TAPPED:
			case DOUBLETAPPED:
			case RELEASED:
			case CONSUMED:
				break;
		}
	}
}

bool dual_function_keys(KBDLLHOOKSTRUCT *input) {
	Mapping *m;

	// consume all taps that are incomplete
	if (!(input->flags & LLKHF_UP)) consume_pressed();

	// is this our key?
	for (m = dfkConfig.m; m && !isInputKey(*input, m->key); m = m->n);

	// forward all other key events
	if (!m) return false;

	if (input->flags & LLKHF_UP) {
		handle_release(m, input);
	} else {
		handle_press(m, input);
	}

	return true;
}
