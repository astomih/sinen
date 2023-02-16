#version 450
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inColor;
layout(set = 0, binding = 0) uniform Matrices {
  mat4 world;
  mat4 view;
  mat4 proj;
  mat4 user;
};

out gl_PerVertex { vec4 gl_Position; };

void main() { gl_Position = proj * view * world * vec4(inPos, 1.0); }
