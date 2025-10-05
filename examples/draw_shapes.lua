-- main.txt (copied to main.lua)
local sdl = require 'sdl'

sdl.init()

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
    sdl.set_render_draw_color(renderer, 100, 100, 100, 255) -- White text
    sdl.render_clear(renderer)

    -- Draw a single point (white)
    sdl.set_render_draw_color(renderer, 255, 255, 255, 255)
    sdl.render_point(renderer, 10, 10)

    -- Draw multiple points (yellow)
    sdl.set_render_draw_color(renderer, 255, 255, 0, 255)
    sdl.render_points(renderer, {
        {x=100, y=100},
        {x=120, y=120},
        {x=140, y=140}
    })

    -- Draw a rectangle outline (red)
    sdl.set_render_draw_color(renderer, 255, 0, 0, 255)
    sdl.render_rect(renderer, 200, 200, 100, 50)

    -- Draw a filled rectangle (blue)
    sdl.set_render_draw_color(renderer, 0, 0, 255, 255)
    sdl.render_fill_rect(renderer, 350, 200, 100, 50)

    -- Draw a polyline (green)
    sdl.set_render_draw_color(renderer, 0, 255, 0, 255)
    sdl.render_lines(renderer, {
        {x=500, y=100},
        {x=550, y=150},
        {x=500, y=200},
        {x=600, y=200}
    })



    -- Set draw color to red and draw a line
    sdl.set_render_draw_color(renderer, 255, 0, 0, 255)
    sdl.render_line(renderer, 100, 100, 700, 500)

    -- Draw debug text
    sdl.set_render_draw_color(renderer, 255, 0, 0, 255) -- Red text
    sdl.render_debug_text(renderer, 50, 50, "Testing SDL3 with Lua!")

    -- Present the renderer
    sdl.render_present(renderer)
end