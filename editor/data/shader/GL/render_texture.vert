precision mediump float;
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inRgba;
out vec2 outUV;
out vec4 outRgba;
out vec3 fragNormal;
out vec3 fragWorldPos;
out mat4 outUser;
uniform Matrices {
  mat4 world;
  mat4 view;
  mat4 proj;
  mat4 user;
};
void main() {
  mat4 inworldmat = world;
  vec4 worldpos = vec4(inPos, 1.0) * inworldmat;
  fragWorldPos = worldpos.xyz;
  gl_Position = proj * view * inworldmat * vec4(inPos, 1.0);
  fragNormal = (inworldmat * vec4(inNormal, 0.0)).xyz;
  outUV = inUV;
  outUV.y = 1.0 - outUV.y;
  outRgba = inRgba;
  outUser = user;
}
