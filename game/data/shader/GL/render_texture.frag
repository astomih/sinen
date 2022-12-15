precision mediump float;
in vec2 outUV;
in vec4 outRgba;
out vec4 outColor;
uniform sampler2D diffuseMap;
uniform sampler2D shadowMap;
void main() {
  vec4 color = outRgba * texture(diffuseMap, outUV);
  outColor = color;
}
