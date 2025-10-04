#define LUA_COMPAT_APIINTCASTS  // For Lua 5.4 integer handling compatibility

#include "module_sdl.h"
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL_keycode.h> // For SDL_Scancode and keycodes

// Metatable for lua_SDL_Window userdata.
static const char* WINDOW_MT = "sdl.window";

// Helper: Push a single event as a Lua table or nil for unhandled events.
static void push_event_table(lua_State* L, SDL_Event* e) {
    if (!e) {
        lua_pushnil(L); // Push nil for null events (unlikely).
        return;
    }

    // Create a new table for handled events.
    lua_newtable(L);

    switch (e->type) {
        case SDL_EVENT_QUIT:
            lua_pushinteger(L, SDL_EVENT_QUIT);
            lua_setfield(L, -2, "type");
            break;

        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            lua_pushinteger(L, SDL_EVENT_WINDOW_CLOSE_REQUESTED);
            lua_setfield(L, -2, "type");
            lua_pushinteger(L, e->window.windowID);
            lua_setfield(L, -2, "window_id");
            break;

        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            lua_pushinteger(L, e->type);
            lua_setfield(L, -2, "type");
            lua_pushinteger(L, e->key.scancode);
            lua_setfield(L, -2, "scancode");
            {
                const char* scancode_name = SDL_GetScancodeName(e->key.scancode);
                lua_pushstring(L, scancode_name ? scancode_name : "unknown");
                lua_setfield(L, -2, "scancode_name");
            }
            lua_pushinteger(L, e->key.key);
            lua_setfield(L, -2, "keycode");
            {
                const char* key_name = SDL_GetKeyName(e->key.key);
                lua_pushstring(L, key_name ? key_name : "unknown");
                lua_setfield(L, -2, "key_name");
            }
            lua_pushboolean(L, e->key.repeat);
            lua_setfield(L, -2, "is_repeat");
            lua_pushinteger(L, e->key.windowID);
            lua_setfield(L, -2, "window_id");
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            lua_pushinteger(L, e->type);
            lua_setfield(L, -2, "type");
            lua_pushinteger(L, e->button.button);
            lua_setfield(L, -2, "button");
            lua_pushinteger(L, e->button.clicks);
            lua_setfield(L, -2, "clicks");
            lua_pushnumber(L, e->button.x);
            lua_setfield(L, -2, "x");
            lua_pushnumber(L, e->button.y);
            lua_setfield(L, -2, "y");
            lua_pushinteger(L, e->button.windowID);
            lua_setfield(L, -2, "window_id");
            break;

        case SDL_EVENT_MOUSE_MOTION:
            lua_pushinteger(L, SDL_EVENT_MOUSE_MOTION);
            lua_setfield(L, -2, "type");
            lua_pushnumber(L, e->motion.x);
            lua_setfield(L, -2, "x");
            lua_pushnumber(L, e->motion.y);
            lua_setfield(L, -2, "y");
            lua_pushnumber(L, e->motion.xrel);
            lua_setfield(L, -2, "xrel");
            lua_pushnumber(L, e->motion.yrel);
            lua_setfield(L, -2, "yrel");
            lua_pushinteger(L, e->motion.windowID);
            lua_setfield(L, -2, "window_id");
            break;

        default:
            lua_pop(L, 1); // Remove the table for unhandled events.
            lua_pushnil(L); // Push nil instead to maintain stack consistency.
            return;
    }
}

// GC metamethod: Destroy the SDL_Window.
static int window_gc(lua_State* L) {
    lua_SDL_Window* ud = (lua_SDL_Window*)luaL_checkudata(L, 1, WINDOW_MT);
    if (ud->window) {
        SDL_DestroyWindow(ud->window);
        ud->window = NULL;
    }
    return 0;
}

// Window __index to access properties like windowID.
static int window_index(lua_State* L) {
    lua_SDL_Window* ud = (lua_SDL_Window*)luaL_checkudata(L, 1, WINDOW_MT);
    const char* key = luaL_checkstring(L, 2);

    if (!ud->window) {
        luaL_error(L, "Attempt to access property '%s' on destroyed window", key);
        return 0;
    }

    if (strcmp(key, "windowID") == 0) {
        Uint32 window_id = SDL_GetWindowID(ud->window);
        if (window_id == 0) {
            luaL_error(L, "Failed to get windowID: %s", SDL_GetError());
        }
        lua_pushinteger(L, window_id);
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

// Metatable setup for windows.
static void window_metatable(lua_State* L) {
    luaL_newmetatable(L, WINDOW_MT);
    lua_pushcfunction(L, window_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, window_index);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

// lua_push_SDL_Window: Create and push userdata.
void lua_push_SDL_Window(lua_State* L, SDL_Window* win) {
    if (!win) {
        luaL_error(L, "Cannot create userdata for null SDL_Window");
    }
    lua_SDL_Window* ud = (lua_SDL_Window*)lua_newuserdata(L, sizeof(lua_SDL_Window));
    ud->window = win;
    luaL_setmetatable(L, WINDOW_MT);
}

// lua_check_SDL_Window: Retrieve userdata, error if invalid.
lua_SDL_Window* lua_check_SDL_Window(lua_State* L, int idx) {
    lua_SDL_Window* ud = (lua_SDL_Window*)luaL_checkudata(L, idx, WINDOW_MT);
    if (!ud->window) {
        luaL_error(L, "Invalid SDL window (already destroyed)");
    }
    return ud;
}

// sdl.init(): Initialize SDL with video.
static int l_sdl_init(lua_State* L) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        luaL_error(L, "Failed to init SDL: %s", SDL_GetError());
    }

    // SDL3 version handling
    const int compiled = SDL_VERSION;  /* hardcoded number from SDL headers */
    const int linked = SDL_GetVersion();  /* reported by linked SDL library */
    
    SDL_Log("We compiled against SDL version %d.%d.%d ...\n",
        SDL_VERSIONNUM_MAJOR(compiled),
        SDL_VERSIONNUM_MINOR(compiled),
        SDL_VERSIONNUM_MICRO(compiled));

    SDL_Log("But we are linking against SDL version %d.%d.%d.\n",
        SDL_VERSIONNUM_MAJOR(linked),
        SDL_VERSIONNUM_MINOR(linked),
        SDL_VERSIONNUM_MICRO(linked));
    return 0;
}

// sdl.create_window(title, width, height, [flags]): Create window with optional flags.
static int l_sdl_create_window(lua_State* L) {
    const char* title = luaL_checkstring(L, 1);
    int w = luaL_checkinteger(L, 2);
    int h = luaL_checkinteger(L, 3);
    SDL_WindowFlags flags = luaL_optinteger(L, 4, 0);

    SDL_Window* win = SDL_CreateWindow(title, w, h, flags);
    if (!win) {
        luaL_error(L, "Failed to create window: %s", SDL_GetError());
    }

    lua_push_SDL_Window(L, win);
    return 1;
}

// sdl.poll_events(): Return a table of events.
static int l_sdl_poll_events(lua_State* L) {
    lua_newtable(L);
    int event_count = 0;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        push_event_table(L, &e);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1); // Remove nil for unhandled events.
        } else {
            lua_rawseti(L, -2, ++event_count); // Store table at index.
        }
    }

    return 1;
}

// Module loader: Register functions and constants.
static const struct luaL_Reg sdl_lib[] = {
    {"init", l_sdl_init},
    {"create_window", l_sdl_create_window},
    {"poll_events", l_sdl_poll_events},
    {NULL, NULL}
};

int luaopen_sdl(lua_State* L) {
    window_metatable(L);
    luaL_newlib(L, sdl_lib);

    lua_pushinteger(L, SDL_WINDOW_FULLSCREEN);
    lua_setfield(L, -2, "WINDOW_FULLSCREEN");
    lua_pushinteger(L, SDL_WINDOW_RESIZABLE);
    lua_setfield(L, -2, "WINDOW_RESIZABLE");
    lua_pushinteger(L, SDL_WINDOW_HIDDEN);
    lua_setfield(L, -2, "WINDOW_HIDDEN");
    lua_pushinteger(L, SDL_WINDOW_BORDERLESS);
    lua_setfield(L, -2, "WINDOW_BORDERLESS");

    lua_pushinteger(L, SDL_EVENT_QUIT);
    lua_setfield(L, -2, "QUIT");
    lua_pushinteger(L, SDL_EVENT_WINDOW_CLOSE_REQUESTED);
    lua_setfield(L, -2, "WINDOW_CLOSE");
    lua_pushinteger(L, SDL_EVENT_KEY_DOWN);
    lua_setfield(L, -2, "KEY_DOWN");
    lua_pushinteger(L, SDL_EVENT_KEY_UP);
    lua_setfield(L, -2, "KEY_UP");
    lua_pushinteger(L, SDL_EVENT_MOUSE_BUTTON_DOWN);
    lua_setfield(L, -2, "MOUSE_BUTTON_DOWN");
    lua_pushinteger(L, SDL_EVENT_MOUSE_BUTTON_UP);
    lua_setfield(L, -2, "MOUSE_BUTTON_UP");
    lua_pushinteger(L, SDL_EVENT_MOUSE_MOTION);
    lua_setfield(L, -2, "MOUSE_MOTION");

    lua_pushinteger(L, SDLK_SPACE);
    lua_setfield(L, -2, "KEY_SPACE");
    lua_pushinteger(L, SDLK_RETURN);
    lua_setfield(L, -2, "KEY_RETURN");
    lua_pushinteger(L, SDLK_ESCAPE);
    lua_setfield(L, -2, "KEY_ESCAPE");
    lua_pushinteger(L, SDLK_A);
    lua_setfield(L, -2, "KEY_A");
    lua_pushinteger(L, SDLK_B);
    lua_setfield(L, -2, "KEY_B");

    lua_pushinteger(L, SDL_BUTTON_LEFT);
    lua_setfield(L, -2, "BUTTON_LEFT");
    lua_pushinteger(L, SDL_BUTTON_RIGHT);
    lua_setfield(L, -2, "BUTTON_RIGHT");
    lua_pushinteger(L, SDL_BUTTON_MIDDLE);
    lua_setfield(L, -2, "BUTTON_MIDDLE");

    return 1;
}

