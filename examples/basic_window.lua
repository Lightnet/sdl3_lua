local sdl = require 'sdl'

sdl.init(sdl.INIT_VIDEO)

-- Create a resizable window with flags.
local window = sdl.create_window("Resizable SDL3 Window", 800, 600, sdl.WINDOW_RESIZABLE)
local window_id = window.windowID

print("Window created. Press close to exit.")

while true do
    local events = sdl.poll_events()
    for i, event in ipairs(events) do
        if event.type == sdl.QUIT or (event.type == sdl.WINDOW_CLOSE and event.window_id == window_id) then
            print("Window closed.")
            return
        end
    end
end

sdl.destroy_window(window)
window = nil
sdl.quit()
print("Window closed.")