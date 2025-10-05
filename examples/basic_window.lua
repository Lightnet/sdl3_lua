local sdl = require 'sdl'

sdl.init()

-- Create a resizable window with flags.
local window = sdl.create_window("Resizable SDL3 Window", 800, 600, sdl.WINDOW_RESIZABLE)

print("Window created. Press close to exit.")

while not sdl.poll_events(window) do
    -- Add rendering or other logic here.
end

print("Window closed.")