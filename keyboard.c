#include <stdlib.h>
#include "keyboard.h"

static Uint8 const *kb;
static Uint8 *pressed;

void key_init(void) {
	int length;

	kb = SDL_GetKeyboardState(&length);
	pressed = calloc(length, sizeof *pressed);
}

bool key_pressed(SDL_Scancode code) {
	return (pressed[code] = kb[code]);
}

bool key_once_pressed(SDL_Scancode code) {
	Uint8 old = pressed[code];
	pressed[code] = kb[code];
	return !old && pressed[code];
}
