precision mediump float;
in vec2 outUV;
in vec4 outRgba;
in mat4 outUser;
out vec4 outColor;
uniform sampler2D diffuseMap;
void main() {
  vec4 color = outRgba * texture2D(diffuseMap, outUV);
  outColor = color;
}
