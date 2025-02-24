#version 450

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

layout(set=2, binding = 0) uniform sampler2D diffuseMap;

void main() {
  // Lambert shading
  
  // Ambient
  vec3 ambient = vec3(0.4);

  // Diffuse
  vec3 norm = normalize(fragNormal);
  vec3 lightDir = vec3(1.0, 1.0, 0.0);
  float diffuse = max(dot(norm, lightDir), 0.0);

  vec3 result = (ambient + diffuse) * inColor.rgb;
  outColor = vec4(result, inColor.a) * texture(diffuseMap, inUV);
}