local sdl = require 'sdl'

sdl.init()

local window = sdl.create_window("SDL3 Input Demo", 800, 600, sdl.WINDOW_RESIZABLE)
local window_id = window.windowID

print("Window created. Press ESC or close to exit.")

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
                print("A key pressed!")
            elseif event.keycode == sdl.KEY_B then
                print("B key pressed!")
            end
        elseif event.type == sdl.KEY_UP then
            print(string.format("Key released: %s (scancode: %d, keycode: %d, window_id: %d)",
                event.key_name, event.scancode, event.keycode, event.window_id))
        elseif event.type == sdl.MOUSE_BUTTON_DOWN then
            print(string.format("Mouse button %d down at (%f, %f), clicks: %d, window_id: %d",
                event.button, event.x, event.y, event.clicks, event.window_id))
        elseif event.type == sdl.MOUSE_BUTTON_UP then
            print(string.format("Mouse button %d up at (%f, %f), window_id: %d",
                event.button, event.x, event.y, event.window_id))
        elseif event.type == sdl.MOUSE_MOTION then
            print(string.format("Mouse moved to (%f, %f), relative: (%f, %f), window_id: %d",
                event.x, event.y, event.xrel, event.yrel, event.window_id))
        end
    end
end