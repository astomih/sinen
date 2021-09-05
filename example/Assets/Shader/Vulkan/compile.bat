@echo off
glslangValidator -V -S vert ./vkshader.vert -o shader.vert.spv
glslangValidator -V -S frag ./vkshaderAlpha.frag -o shaderAlpha.frag.spv
glslangValidator -V -S frag ./vkshaderOpaque.frag -o shaderOpaque.frag.spv
@echo on
