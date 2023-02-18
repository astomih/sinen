#version 450
layout(location = 0) in vec3 inPos;
layout(binding = 0) uniform Matrices {
  mat4 world;
  mat4 view;
  mat4 proj;
  mat4 user;
  mat4 light_view;
  mat4 light_proj;
};

void main() {
  gl_Position = light_proj * light_view * world * vec4(inPos, 1.0);
}
