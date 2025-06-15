slangc -profile glsl_450 -target spirv -entry FSMain shader.slang -o shader.frag.spv
slangc -profile glsl_450 -target spirv -entry VSMain shader.slang -o shader.vert.spv
slangc -profile glsl_450 -target spirv -entry VSMain shader_instance.slang -o shader_instance.vert.spv

xxd -i shader.frag.spv > shader.frag.spv.hpp
xxd -i shader.vert.spv > shader.vert.spv.hpp
xxd -i shader_instance.vert.spv > shader_instance.vert.spv.hpp

