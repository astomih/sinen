precision mediump float;
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inRgba;
uniform Matrices {
  mat4 world;
  mat4 view;
  mat4 proj;
  mat4 light_view;
  mat4 light_proj;
};
void main() {
  gl_Position = light_proj * light_view * world * vec4(inPos, 1.0);
}