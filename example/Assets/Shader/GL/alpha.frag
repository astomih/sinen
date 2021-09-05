#version 330 core
precision mediump float;
in vec2 outUV;
out vec4 outColor;
uniform sampler2D diffuseMap;
void main()
{
  vec4 color = texture(diffuseMap, outUV);
  outColor = color;
}
