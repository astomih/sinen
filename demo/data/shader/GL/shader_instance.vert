precision mediump float;
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inRgba;
layout(location = 4) in vec4 m1;
layout(location = 5) in vec4 m2;
layout(location = 6) in vec4 m3;
layout(location = 7) in vec4 m4;
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
  mat4 inworldmat;
  inworldmat[0] = m1;
  inworldmat[1] = m2;
  inworldmat[2] = m3;
  inworldmat[3] = m4;
  vec4 worldpos = vec4(inPos, 1.0) * inworldmat;
  fragWorldPos = worldpos.xyz;
  gl_Position = proj * view * inworldmat * vec4(inPos, 1.0);
  fragNormal = (inworldmat * vec4(inNormal, 0.0)).xyz;
  outUV = inUV;
  outRgba = inRgba;
  outUser = user;
}
