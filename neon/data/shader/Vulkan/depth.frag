#version 450
layout(location = 0) out vec4 outColor;

void main() {
  float d = gl_FragCoord.z;
  outColor = vec4(d, d, d, 1.0);
}