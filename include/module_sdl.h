// module_sdl.h
#ifndef MODULE_SDL_H
#define MODULE_SDL_H

#include <lua.h>
#include <lauxlib.h> // Added for luaL_ functions
#include <SDL3/SDL.h>

typedef struct {
    SDL_Window* window;
} lua_SDL_Window;

typedef struct {
    SDL_Renderer* renderer;
} lua_SDL_Renderer;

typedef struct {
    SDL_Texture* texture;
} lua_SDL_Texture; // for texture support

void lua_push_SDL_Window(lua_State* L, SDL_Window* win);
lua_SDL_Window* lua_check_SDL_Window(lua_State* L, int idx);
void lua_push_SDL_Renderer(lua_State* L, SDL_Renderer* renderer);
lua_SDL_Renderer* lua_check_SDL_Renderer(lua_State* L, int idx);
void lua_push_SDL_Texture(lua_State* L, SDL_Texture* texture); 
lua_SDL_Texture* lua_check_SDL_Texture(lua_State* L, int idx); 
int luaopen_sdl(lua_State* L);

#endif