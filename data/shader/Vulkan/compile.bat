@echo off
glslangValidator -V -S vert ./vkshader.vert -o shader.vert.spv
glslangValidator -V -S vert ./vk_shader_instance.vert -o shader_instance.vert.spv
glslangValidator -V -S frag ./vkshaderAlpha.frag -o shaderAlpha.frag.spv
glslangValidator -V -S frag ./vkshaderOpaque.frag -o shaderOpaque.frag.spv
glslangValidator -V -S frag ./vkshaderPlain.frag -o shaderPlain.frag.spv
@echo on
