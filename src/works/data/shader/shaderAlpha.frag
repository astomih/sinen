#version 450

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec4 inColor;
layout(location = 0) out vec4 outColor;

layout(set=2, binding = 0) uniform sampler2D diffuseMap;

void main() {
  vec4 color = inColor * texture(diffuseMap, inUV);
  outColor = color;
}