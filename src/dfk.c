/*
 * Dual Function Keys - remapping and tapping of keys
 * heavily inspired by https://gitlab.com/interception/linux/plugins/dual-function-keys
 *
 *
 * *The MIT License (MIT)*
 *
 * Copyright © 2020 Alexander Courtis
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the “Software”), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 **/

#include "dfk.h"

unsigned levelOfLastPress = 1;

// ignores `extended` flag of `flags`
KBDLLHOOKSTRUCT newKeyInfo (OutputKey send, DWORD flags) {
	return (KBDLLHOOKSTRUCT) {
		.vkCode = send.vk,
		.scanCode = send.scan,
		.flags = (send.isExtended ? LLKHF_EXTENDED : 0) | (flags & ~LLKHF_EXTENDED),
		.time = 0,
		.dwExtraInfo = 0
	};
}

void tap(Mapping *m, DWORD flags, bool useLevelOfLastPress) {
	// use level of last press so tapping behaves like an actual key
	unsigned level = useLevelOfLastPress ? levelOfLastPress : getLevel();

	for (Tap *t = m->tap; t; t = t->n) {
		writeEvent(newKeyInfo(t->code, flags), level);
	}
}

void first_tap(Mapping *m, KBDLLHOOKSTRUCT *input, bool useLevelOfLastPress) {
	// release "hold"
	writeEvent(newKeyInfo(m->hold, input->flags | LLKHF_UP), getLevel());

	// synthesize tap
	tap(m, 0, useLevelOfLastPress);
	tap(m, LLKHF_UP, useLevelOfLastPress);
}

void handle_press(Mapping *m, KBDLLHOOKSTRUCT *input) {
	levelOfLastPress = getLevel();

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
				? DOUBLETAPPED_FIRST_TIME
				: PRESSED;
			break;
		case DOUBLETAPPED_FIRST_TIME:
			m->state = DOUBLETAPPED_SECOND_TIME;
			break;
		case DOUBLETAPPED_SECOND_TIME:
			m->state = DOUBLETAPPED;
			break;
	}
	m->changed = input->time;

	// action
	switch (m->state) {
		case RELEASED:
		case PRESSED:
		case CONSUMED:
			writeEvent(newKeyInfo(m->hold, input->flags), getLevel());
			break;
		case TAPPED:
		case DOUBLETAPPED_FIRST_TIME:
			writeEvent(newKeyInfo(m->hold, input->flags), getLevel());
			break;
		case DOUBLETAPPED_SECOND_TIME:
			first_tap(m, input, false);
			break;
		case DOUBLETAPPED:
			tap(m, 0, false);
			break;
	}
}

void handle_release(Mapping *m, KBDLLHOOKSTRUCT *input) {
	// state
	switch (m->state) {
		case RELEASED:
		case TAPPED:
		case DOUBLETAPPED_FIRST_TIME:
			break;
		case PRESSED:
			m->state = input->time - m->changed < dfkConfig.tap_millis
				? TAPPED
				: RELEASED;
			break;
		case DOUBLETAPPED_SECOND_TIME:
		case DOUBLETAPPED:
		case CONSUMED:
			m->state = RELEASED;
			break;
	}
	m->changed = input->time;

	// action
	switch (m->state) {
		case RELEASED:
		case PRESSED:
		case CONSUMED:
			writeEvent(newKeyInfo(m->hold, input->flags), getLevel());
			break;
		case TAPPED:
		case DOUBLETAPPED_FIRST_TIME:
			first_tap(m, input, true);
			break;
		case DOUBLETAPPED_SECOND_TIME:
		case DOUBLETAPPED:
			tap(m, LLKHF_UP, true);
			break;
	}
}

void consume_pressed_and_other_first_doubletapped(Mapping *currentM) {
	// state
	for (Mapping *m = dfkConfig.m; m; m = m->n) {
		switch (m->state) {
			case RELEASED:
			case TAPPED:
			case DOUBLETAPPED_SECOND_TIME:
			case DOUBLETAPPED:
			case CONSUMED:
				break;
			case DOUBLETAPPED_FIRST_TIME:
				// consume first doubletap only of other mappings
				if (m != currentM) m->state = CONSUMED;
				break;
			case PRESSED:
				m->state = CONSUMED;
				break;
		}
	}
}

bool dual_function_keys(KBDLLHOOKSTRUCT *input) {
	Mapping *m;

	// find mapping for input; NULL if not defined
	for (m = dfkConfig.m; m && !isInputKey(*input, m->key); m = m->n);

	// consume all taps that are incomplete
	// (those which would synthesize a tap on release)
	if (!(input->flags & LLKHF_UP)) consume_pressed_and_other_first_doubletapped(m);

	// forward all other key events
	if (!m) return false;

	if (input->flags & LLKHF_UP) {
		handle_release(m, input);
	} else {
		handle_press(m, input);
	}

	return true;
}
