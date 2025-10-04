// module_sdl.c
#define LUA_COMPAT_APIINTCASTS  // For Lua 5.4 integer handling compatibility

#include "module_sdl.h"
#include <stdlib.h>
#include <string.h>

// Metatables
static const char* WINDOW_MT = "sdl.window";
static const char* RENDERER_MT = "sdl.renderer";

// Helper: Push a single event as a Lua table or nil for unhandled events.
static void push_event_table(lua_State* L, SDL_Event* e) {
    if (!e) {
        lua_pushnil(L);
        return;
    }

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
            lua_pop(L, 1);
            lua_pushnil(L);
            return;
    }
}

// GC metamethod for window: Destroy the SDL_Window.
static int window_gc(lua_State* L) {
    lua_SDL_Window* ud = lua_check_SDL_Window(L, 1);
    if (ud->window) {
        SDL_DestroyWindow(ud->window);
        ud->window = NULL;
    }
    return 0;
}

// GC metamethod for renderer: Destroy the SDL_Renderer.
static int renderer_gc(lua_State* L) {
    lua_SDL_Renderer* ud = lua_check_SDL_Renderer(L, 1);
    if (ud->renderer) {
        SDL_DestroyRenderer(ud->renderer);
        ud->renderer = NULL;
    }
    return 0;
}

// Window __index to access properties like windowID.
static int window_index(lua_State* L) {
    lua_SDL_Window* ud = lua_check_SDL_Window(L, 1);
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

// Set render draw color: sdl.set_render_draw_color(renderer, r, g, b, [a])
static int l_sdl_set_render_draw_color(lua_State* L) {
    lua_SDL_Renderer* ud = lua_check_SDL_Renderer(L, 1);
    int r = luaL_checkinteger(L, 2);
    int g = luaL_checkinteger(L, 3);
    int b = luaL_checkinteger(L, 4);
    int a = luaL_optinteger(L, 5, 255);

    if (!ud->renderer) {
        luaL_error(L, "No renderer available");
    }

    // Clamp values to [0, 255]
    r = r < 0 ? 0 : (r > 255 ? 255 : r);
    g = g < 0 ? 0 : (g > 255 ? 255 : g);
    b = b < 0 ? 0 : (b > 255 ? 255 : b);
    a = a < 0 ? 0 : (a > 255 ? 255 : a);

    SDL_Log("Setting render draw color: r=%d, g=%d, b=%d, a=%d", r, g, b, a);

    // if (!SDL_SetRenderDrawColor(ud->renderer, 0xFF, 0xFF, 0xFF, 0xFF)) {
    //     luaL_error(L, "Failed to set render draw color: %s", SDL_GetError());
    // }
    if (!SDL_SetRenderDrawColor(ud->renderer, (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a) != 0) {
        luaL_error(L, "Failed to set render draw color: %s", SDL_GetError());
    }
    return 0;
}

// Clear renderer: sdl.render_clear(renderer)
static int l_sdl_render_clear(lua_State* L) {
    lua_SDL_Renderer* ud = lua_check_SDL_Renderer(L, 1);

    if (!ud->renderer) {
        luaL_error(L, "No renderer available");
    }

    if (!SDL_RenderClear(ud->renderer)) {
        luaL_error(L, "Failed to clear renderer: %s", SDL_GetError());
    }
    return 0;
}

// Present renderer: sdl.render_present(renderer)
static int l_sdl_render_present(lua_State* L) {
    lua_SDL_Renderer* ud = lua_check_SDL_Renderer(L, 1);

    if (!ud->renderer) {
        luaL_error(L, "No renderer available");
    }

    SDL_RenderPresent(ud->renderer);
    return 0;
}

// Metatable setup for windows and renderers.
static void window_metatable(lua_State* L) {
    luaL_newmetatable(L, WINDOW_MT);
    lua_pushcfunction(L, window_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, window_index);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

static void renderer_metatable(lua_State* L) {
    luaL_newmetatable(L, RENDERER_MT);
    lua_pushcfunction(L, renderer_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

// lua_push_SDL_Window: Create and push window userdata.
void lua_push_SDL_Window(lua_State* L, SDL_Window* win) {
    if (!win) {
        luaL_error(L, "Cannot create userdata for null SDL_Window");
    }
    lua_SDL_Window* ud = (lua_SDL_Window*)lua_newuserdata(L, sizeof(lua_SDL_Window));
    ud->window = win;
    luaL_setmetatable(L, WINDOW_MT);
}

// lua_push_SDL_Renderer: Create and push renderer userdata.
void lua_push_SDL_Renderer(lua_State* L, SDL_Renderer* renderer) {
    if (!renderer) {
        luaL_error(L, "Cannot create userdata for null SDL_Renderer");
    }
    lua_SDL_Renderer* ud = (lua_SDL_Renderer*)lua_newuserdata(L, sizeof(lua_SDL_Renderer));
    ud->renderer = renderer;
    luaL_setmetatable(L, RENDERER_MT);
}

// lua_check_SDL_Window: Retrieve window userdata, error if invalid.
lua_SDL_Window* lua_check_SDL_Window(lua_State* L, int idx) {
    lua_SDL_Window* ud = (lua_SDL_Window*)luaL_checkudata(L, idx, WINDOW_MT);
    if (!ud->window) {
        luaL_error(L, "Invalid SDL window (already destroyed)");
    }
    return ud;
}

// lua_check_SDL_Renderer: Retrieve renderer userdata, error if invalid.
lua_SDL_Renderer* lua_check_SDL_Renderer(lua_State* L, int idx) {
    lua_SDL_Renderer* ud = (lua_SDL_Renderer*)luaL_checkudata(L, idx, RENDERER_MT);
    if (!ud->renderer) {
        luaL_error(L, "Invalid SDL renderer (already destroyed)");
    }
    return ud;
}

// sdl.init(): Initialize SDL with video.
static int l_sdl_init(lua_State* L) {
    if (!SDL_Init(SDL_INIT_VIDEO)) { // Correct: 0 (false) means success
        luaL_error(L, "Failed to init SDL: %s", SDL_GetError());
    }

    // Log available render drivers
    int num_drivers = SDL_GetNumRenderDrivers();
    for (int i = 0; i < num_drivers; i++) {
        const char* driver_name = SDL_GetRenderDriver(i);
        SDL_Log("Available renderer driver %d: %s", i, driver_name ? driver_name : "unknown");
    }

    // SDL3 version handling
    Uint32 compiled = SDL_VERSION;
    Uint32 linked = SDL_GetVersion();

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

// sdl.create_window(title, width, height, [flags]): Create window.
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

// sdl.create_renderer(window, [driver]): Create renderer for the window.
static int l_sdl_create_renderer(lua_State* L) {
    lua_SDL_Window* ud = lua_check_SDL_Window(L, 1);
    const char* driver = luaL_optstring(L, 2, NULL); // Optional driver name

    // Set renderer driver hint if specified
    if (driver) {
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, driver);
        SDL_Log("Attempting to create renderer with driver: %s", driver);
    } else {
        SDL_Log("Creating renderer with default driver");
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(ud->window, driver);
    if (!renderer) {
        luaL_error(L, "Failed to create renderer: %s", SDL_GetError());
    }

    // Optional: Set blend mode (disabled to avoid failure)
    /*
    SDL_Log("Attempting to set blend mode to SDL_BLENDMODE_BLEND");
    if (SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) != 0) {
        SDL_Log("Warning: Failed to set blend mode: %s (continuing without it)", SDL_GetError());
    }
    */

    // Log renderer properties
    SDL_PropertiesID props = SDL_GetRendererProperties(renderer);
    if (props != 0) {
        const char* name = SDL_GetStringProperty(props, SDL_PROP_RENDERER_NAME_STRING, "unknown");
        Sint64 max_texture_size = SDL_GetNumberProperty(props, SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER, 0);
        SDL_Log("Renderer created: name=%s, max_texture_size=%lld", name, (long long)max_texture_size);
        // SDL_ReleaseProperties(props);
    } else {
        SDL_Log("Failed to get renderer properties: %s", SDL_GetError());
    }

    lua_push_SDL_Renderer(L, renderer);
    return 1;
}

// sdl.create_window_and_renderer(title, width, height, [flags]): Create window and renderer (kept for compatibility).
static int l_sdl_create_window_and_renderer(lua_State* L) {
    const char* title = luaL_checkstring(L, 1);
    int w = luaL_checkinteger(L, 2);
    int h = luaL_checkinteger(L, 3);
    SDL_WindowFlags flags = luaL_optinteger(L, 4, 0);

    SDL_Window* win = NULL;
    SDL_Renderer* renderer = NULL;

    if (!SDL_CreateWindowAndRenderer(title, w, h, flags, &win, &renderer)) {
        luaL_error(L, "Failed to create window and renderer: %s", SDL_GetError());
    }

    if (!renderer) {
        if (win) SDL_DestroyWindow(win);
        luaL_error(L, "Renderer creation failed unexpectedly");
    }

    // Log renderer properties
    SDL_PropertiesID props = SDL_GetRendererProperties(renderer);
    if (props != 0) {
        const char* name = SDL_GetStringProperty(props, SDL_PROP_RENDERER_NAME_STRING, "unknown");
        Sint64 max_texture_size = SDL_GetNumberProperty(props, SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER, 0);
        SDL_Log("Renderer created: name=%s, max_texture_size=%lld", name, (long long)max_texture_size);
        // SDL_ReleaseProperties(props);
    } else {
        SDL_Log("Failed to get renderer properties: %s", SDL_GetError());
    }

    SDL_Log("Window and renderer created successfully");
    lua_push_SDL_Window(L, win);
    lua_push_SDL_Renderer(L, renderer);
    return 2; // Return window and renderer
}

// sdl.poll_events(): Return a table of events.
static int l_sdl_poll_events(lua_State* L) {
    lua_newtable(L);
    int event_count = 0;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        push_event_table(L, &e);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
        } else {
            lua_rawseti(L, -2, ++event_count);
        }
    }

    return 1;
}

// Module loader: Register functions and constants.
static const struct luaL_Reg sdl_lib[] = {
    {"init", l_sdl_init},
    {"create_window", l_sdl_create_window},
    {"create_renderer", l_sdl_create_renderer},
    {"create_window_and_renderer", l_sdl_create_window_and_renderer},
    {"poll_events", l_sdl_poll_events},
    {"set_render_draw_color", l_sdl_set_render_draw_color},
    {"render_clear", l_sdl_render_clear},
    {"render_present", l_sdl_render_present},
    {NULL, NULL}
};

int luaopen_sdl(lua_State* L) {
    window_metatable(L);
    renderer_metatable(L);
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