slangc -profile glsl_450 -target spirv -entry FSMain shader.slang -o shader.frag.spv
slangc -profile glsl_450 -target spirv -entry VSMain shader.slang -o shader.vert.spv
slangc -profile glsl_450 -target spirv -entry VSMain shader_instance.slang -o shader_instance.vert.spv

xxd -i shader.frag.spv > shader.frag.spv.h
xxd -i shader.vert.spv > shader.vert.spv.h
xxd -i shader_instance.vert.spv > shader_instance.vert.spv.h

