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
		// use level of last press so tapping behaves like an actual key
		writeEvent(newKeyInfo(t->code, flags), levelOfLastPress);
	}
}

void handle_press(Mapping *m, KBDLLHOOKSTRUCT *input) {
	levelOfLastPress = getLevel();
	bool isFirstDouble = true;

	// state
	switch (m->state) {
		case RELEASED:
			m->state = PRESSED;
			break;
		case PRESSED:
		case DOUBLETAPPED:
		case CONSUMED:
			isFirstDouble = false;
			break;
		case TAPPED:
			isFirstDouble = true;
			m->state = input->time - m->changed < dfkConfig.double_tap_millis
				? DOUBLETAPPED
				: PRESSED;
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
		case DOUBLETAPPED:
			if (isFirstDouble) {
				writeEvent(newKeyInfo(m->hold, input->flags), getLevel());
			} else {
				tap(m, 0);
			}
			break;
	}
}

void handle_release(Mapping *m, KBDLLHOOKSTRUCT *input) {
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

	// action
	switch (m->state) {
		case RELEASED:
		case PRESSED:
		case CONSUMED:
			writeEvent(newKeyInfo(m->hold, input->flags), getLevel());
			break;
		case TAPPED:
			// release "hold"
			writeEvent(newKeyInfo(m->hold, input->flags), getLevel());

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
