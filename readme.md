Rhythm_Car
My attempt at making a Windows-only game engine from scratch using nothing more than Vulkan and Win32 APIs. Hardware rendering is hard :(

Build instructions
Just use the build.bat script present in the repository. There are pre-built shaders in the repository, but if you want to build them by yourself you would need to install the glsl compiler.

Features
The features that the ""Game Engine"" have are:

A event based input system with timestamps for input timing.
Assets hot reloading (doesnt't work with compress .tga files, just with its uncompress variants).
Original parsers for .obj, .tga, .bmp, .fnt, file formats.
support for Xbox 360 controllers.
How to Play
a, w, d, s -> camera movement.
f -> fullscreen.
escape -> close the game.
v -> start driving the car.
left arrow, right arrow -> car movement.
q -> toggle cursor mode.
u -> turn UI on/off.
z -> sponza.
r -> render Axis Aligned Bounding Boxes (AABB).
Shoutouts
This ""Game Engine"" could be made thanks to the Handmade Hero series and the Kohi Game Engine series.

Overview
Rhythm car overview.