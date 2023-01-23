#version 450
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inColor;
layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outColor;

layout(set = 0, binding = 0) uniform Matrices {
  mat4 world;
  mat4 view;
  mat4 proj;
};

out gl_PerVertex { vec4 gl_Position; };

// Normal (in world space)
layout(location = 2) out vec3 fragNormal;
// Position (in world space)
layout(location = 3) out vec3 fragWorldPos;

void main() {
  mat4 inworldmat;
  inworldmat = world;

  vec4 worldpos = vec4(inPos, 1.0) * inworldmat;
  fragWorldPos = worldpos.xyz;
  gl_Position = proj * view * world * vec4(inPos, 1.0);
  fragNormal = (inworldmat * vec4(inNormal, 0.0f)).xyz;
  outUV = inUV;
  outColor = inColor;
}
