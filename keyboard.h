#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "decl.h"

// let me help ya
// instead of key_pressed(SDL_SCANCODE_*)
// use key_pressed(*)
// easier, innit?
#define KEY(x) SDL_SCANCODE_##x

BEGIN_DECL

void key_init(void);
bool key_pressed(SDL_Scancode);
bool key_once_pressed(SDL_Scancode);

END_DECL

#endif /* KEYBOARD_H */
