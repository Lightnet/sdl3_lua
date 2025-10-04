#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Declare the sdl module's entry point (from module_sdl.c).
int luaopen_sdl(lua_State* L);

// Check if a file exists.
static int file_exists(const char* path) {
    FILE* file = fopen(path, "r");
    if (file) {
        fclose(file);
        return 1; // File exists.
    }
    return 0; // File does not exist.
}

int main(int argc, char* argv[]) {
    // Initialize Lua state.
    lua_State* L = luaL_newstate();
    if (!L) {
        fprintf(stderr, "Failed to create Lua state\n");
        return 1;
    }

    // Load standard Lua libraries.
    luaL_openlibs(L);

    // Register the sdl module.
    luaL_requiref(L, "sdl", luaopen_sdl, 1);
    lua_pop(L, 1); // Remove module from stack.

    // SDL_CreateWindowAndRenderer

    // Determine script path: command-line arg or default to "main.lua".
    const char* script_path = (argc >= 2) ? argv[1] : "main.lua";

    // Check if the script file exists.
    if (!file_exists(script_path)) {
        fprintf(stderr, "Error: Script '%s' not found\n", script_path);
        if (argc < 2) {
            fprintf(stderr, "Usage: %s [<lua_script_path>]\n", argv[0]);
        }
        lua_close(L);
        return 1;
    }

    // Load and run the Lua script.
    if (luaL_loadfile(L, script_path) != LUA_OK) {
        fprintf(stderr, "Error loading script '%s': %s\n", script_path, lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }

    // Execute the script.
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        fprintf(stderr, "Error running script '%s': %s\n", script_path, lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }

    // Clean up.
    lua_close(L);
    SDL_Quit(); // Ensure SDL is cleaned up after script execution.
    return 0;
}