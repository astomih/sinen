@echo off
glslangValidator -V -S vert ./shader.vert -o shader.vert.spv
glslangValidator -V -S vert ./shader_instance.vert -o shader_instance.vert.spv
glslangValidator -V -S frag ./shaderAlpha.frag -o shaderAlpha.frag.spv
@echo on
