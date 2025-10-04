#ifndef MODULE_SDL_H
#define MODULE_SDL_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <SDL3/SDL.h>

// Userdata for SDL_Window
typedef struct {
    SDL_Window* window;
} lua_SDL_Window;

// Pushes a new lua_SDL_Window userdata onto the stack.
void lua_push_SDL_Window(lua_State* L, SDL_Window* win);

// Gets lua_SDL_Window from userdata at index.
lua_SDL_Window* lua_check_SDL_Window(lua_State* L, int idx);

#endif // MODULE_SDL_H