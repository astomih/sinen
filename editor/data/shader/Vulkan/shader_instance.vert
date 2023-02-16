#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inColor;
layout(location = 4) in vec4 m1;
layout(location = 5) in vec4 m2;
layout(location = 6) in vec4 m3;
layout(location = 7) in vec4 m4;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragWorldPos;
layout(location = 4) out vec4 shadowCoord;
layout(location = 5) out mat4 outUser;

layout(binding = 0) uniform Matrices {
  mat4 world;
  mat4 view;
  mat4 proj;
  mat4 user;
  mat4 light_view;
  mat4 light_proj;
};

out gl_PerVertex { vec4 gl_Position; };

void main() {
  mat4 inworldmat;
  inworldmat[0] = m1;
  inworldmat[1] = m2;
  inworldmat[2] = m3;
  inworldmat[3] = m4;

  vec4 worldpos = vec4(inPos, 1.0) * inworldmat;
  fragWorldPos = worldpos.xyz;
  gl_Position = proj * view * inworldmat * vec4(inPos, 1.0);
  fragNormal = (inworldmat * vec4(inNormal, 0.0f)).xyz;
  outUV = inUV;
  outColor = inColor;
  outUser = user;
  shadowCoord = light_proj * light_view * inworldmat * vec4(inPos, 1.0);
}