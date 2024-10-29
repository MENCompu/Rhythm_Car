@echo off

pushd bin

glslc ..\src\shaders\shader.glsl.vert -o shader.spv.vert
glslc ..\src\shaders\shader.glsl.frag -o shader.spv.frag

glslc ..\src\shaders\ui.glsl.vert -o ui.spv.vert
glslc ..\src\shaders\ui.glsl.frag -o ui.spv.frag

glslc ..\src\shaders\wireframe.glsl.vert -o wireframe.spv.vert
glslc ..\src\shaders\wireframe.glsl.frag -o wireframe.spv.frag

popd
