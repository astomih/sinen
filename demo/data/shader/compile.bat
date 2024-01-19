@echo off
glslangValidator -V -S vert ./shader.vert -o shader.vert.spv
glslangValidator -V -S vert ./shader_instance.vert -o shader_instance.vert.spv
glslangValidator -V -S frag ./shaderAlpha.frag -o shaderAlpha.frag.spv
glslangValidator -V -S frag ./render_texture.frag -o render_texture.frag.spv
glslangValidator -V -S frag ./shaderOpaque.frag -o shaderOpaque.frag.spv
glslangValidator -V -S vert ./depth.vert -o depth.vert.spv
glslangValidator -V -S vert ./depth_instance.vert -o depth_instance.vert.spv
glslangValidator -V -S frag ./depth.frag -o depth.frag.spv
@echo on
