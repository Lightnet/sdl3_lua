# sdl3_lua

# License: MIT

# Program languages:
- c language
- cmake

# libs:
- SDL 3.2.22
- Lua 5.4

# Information:
  This is a Sample builf for SDL3 and Lua 5.4. To test basic features renderer and inputs.

  Want to keep it simple.

# msys2:
  This build using the windows and msys2. Required some libs to install.

# build.bat
```bat
@echo off
setlocal
set MSYS2_PATH=C:\msys64\mingw64\bin
set VULKAN_SDK=C:\VulkanSDK\1.4.313.0
set PATH=%MSYS2_PATH%;%VULKAN_SDK%\Bin;%PATH%
if not exist build mkdir build
cd build
%MSYS2_PATH%\cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=%MSYS2_PATH%\gcc.exe -DCMAKE_CXX_COMPILER=%MSYS2_PATH%\g++.exe
%MSYS2_PATH%\cmakecmake --build . --config Debug
endlocal
```


# Lua:
  Lua can be easy and hard.

# SDL 3.2

```
Available renderer driver 0: direct3d11
Available renderer driver 1: direct3d12
Available renderer driver 2: direct3d
Available renderer driver 3: opengl
Available renderer driver 4: opengles2
Available renderer driver 5: vulkan
Available renderer driver 6: gpu
Available renderer driver 7: software
```

```c
#include <SDL3/SDL.h>
```

```c
SDL_Window* window = SDL_CreateWindow("SDL 3 Lua", 800, 600, 0);
```

```c
SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
```
By default to direct3d11 if on windows. As it try by list one by one to load check.

```c
SDL_PropertiesID props = SDL_GetRendererProperties(renderer);
const char* name = SDL_GetStringProperty(props, SDL_PROP_RENDERER_NAME_STRING, "unknown");
Sint64 max_texture_size = SDL_GetNumberProperty(props, SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER, 0);
SDL_Log("Renderer created: name=%s, max_texture_size=%lld", name, (long long)max_texture_size);
```
 - SDL_PROP_RENDERER_NAME_STRING: the name of the rendering driver
 - SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER: the maximum texture width and height

# Lua SDL 3:

```lua
-- main.txt (copied to main.lua)
local sdl = require 'sdl'

sdl.init(sdl.INIT_VIDEO)

local window = sdl.create_window("SDL3 Renderer Demo", 800, 600, sdl.WINDOW_RESIZABLE)
local window_id = window.windowID

local renderer, err = sdl.create_renderer(window)
if not renderer then
    print("Error creating renderer: " .. (err or "Unknown error"))
    return
end

print("Window and renderer created. Press ESC or close to exit.")

-- Initial color (red)
local status, err = pcall(sdl.set_render_draw_color, renderer, 255, 0, 0, 255)
if not status then
    print("Error setting render draw color: " .. (err or "Unknown error"))
    return
end
sdl.set_render_draw_color(renderer, 255, 0, 0, 255)
sdl.render_clear(renderer)
sdl.render_present(renderer)

while true do
    local events = sdl.poll_events()
    for i, event in ipairs(events) do
        if event.type == sdl.QUIT or (event.type == sdl.WINDOW_CLOSE and event.window_id == window_id) then
            print("Window closed.")
            return
        elseif event.type == sdl.KEY_DOWN then
            print(string.format("Key pressed: %s (scancode: %d, keycode: %d, is_repeat: %s, window_id: %d)",
                event.key_name, event.scancode, event.keycode, tostring(event.is_repeat), event.window_id))
            if event.keycode == sdl.KEY_ESCAPE then
                print("ESC pressed. Exiting.")
                return
            elseif event.keycode == sdl.KEY_A then
                print("A key pressed! Changing to blue.")
                sdl.set_render_draw_color(renderer, 0, 0, 255, 255) -- Blue
            elseif event.keycode == sdl.KEY_B then
                print("B key pressed! Changing to green.")
                sdl.set_render_draw_color(renderer, 0, 255, 0, 255) -- Green
            end
        elseif event.type == sdl.KEY_UP then
            print(string.format("Key released: %s (scancode: %d, keycode: %d, window_id: %d)",
                event.key_name, event.scancode, event.keycode, event.window_id))
        elseif event.type == sdl.MOUSE_BUTTON_DOWN then
            print(string.format("Mouse button %d down at (%f, %f), clicks: %d, window_id: %d)",
                event.button, event.x, event.y, event.clicks, event.window_id))
        elseif event.type == sdl.MOUSE_BUTTON_UP then
            print(string.format("Mouse button %d up at (%f, %f), window_id: %d)",
                event.button, event.x, event.y, event.window_id))
        elseif event.type == sdl.MOUSE_MOTION then
            print(string.format("Mouse moved to (%f, %f), relative: (%f, %f), window_id: %d)",
                event.x, event.y, event.xrel, event.yrel, event.window_id))
        end
    end

    -- Update the screen
    sdl.render_clear(renderer)
    sdl.render_present(renderer)
end

sdl.destroy_window(window)
window = nil
sdl.quit()
```

# Notes:
- console log will lag if there too much in logging.

# refs:
- https://wiki.libsdl.org/SDL3/CategoryAPI
- https://wiki.libsdl.org/SDL3/SDL_GetRenderDriver
- https://wiki.libsdl.org/SDL3/QuickReference
- https://wiki.libsdl.org/SDL3/SDL_HINT_RENDER_DRIVER
- https://wiki.libsdl.org/SDL3/SDL_GetCurrentVideoDriver
- https://wiki.libsdl.org/SDL3/SDL_GetRendererProperties
- https://wiki.libsdl.org/SDL3/SDL_SetHint
- https://wiki.libsdl.org/SDL3/SDL_FillSurfaceRect
- https://wiki.libsdl.org/SDL3/SDL_RenderLine
- https://wiki.libsdl.org/SDL3/SDL_RenderDebugText

# SDL_GetRendererProperties:
- SDL_PROP_RENDERER_NAME_STRING: the name of the rendering driver
  - direct3d11, opengl, vulkan, etc...
