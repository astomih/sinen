#version 450

precision mediump float;
layout(location = 0) in vec2 inUV;
layout(location = 1) in vec4 inColor;
layout(location = 0) out vec4 outColor;
// Normal (in world space)
layout(location = 2) in vec3 fragNormal;
// Position (in world space)
layout(location = 3) in vec3 fragWorldPos;
layout(location = 4) in vec4 shadowCoord;
layout(binding = 1) uniform sampler2D diffuseMap;
layout(binding = 2) uniform sampler2D shadowMap;
float simple_shadow(vec3 proj_pos) {
  float shadow_distance = max((texture(shadowMap, proj_pos.xy).r), 0.0);
  float distance = proj_pos.z;
  if (shadow_distance < distance)
    return 0.5;
  return 1.0;
}

void main() {
  vec3 uCameraPos = vec3(0, 0, 0);
  // Direction of light
  vec3 mDirection = vec3(0.0, -0.25, -0.25);
  // Diffuse color
  vec3 mDiffuseColor = vec3(1, 1, 1);
  // Specular color
  vec3 mSpecColor = vec3(1, 1, 1);
  // Specular power for this surface
  float uSpecPower = 100;
  // Ambient light level
  vec3 uAmbientLight = vec3(0.5);
  // Surface normal
  vec3 N = normalize(fragNormal);
  // Vector from surface to light
  vec3 L = normalize(-mDirection);
  // Vector from surface to camera
  vec3 V = normalize(uCameraPos - fragWorldPos);
  // Reflection of -L about N
  vec3 R = normalize(reflect(-L, N));

  // Compute phong reflection
  vec3 Phong = uAmbientLight;
  float NdotL = dot(N, L);
  if (NdotL > 0) {
    float visibility = simple_shadow(shadowCoord.xyz);
    vec3 Diffuse = mDiffuseColor * NdotL * visibility;
    Phong += Diffuse;
  }
  vec4 color = vec4(Phong, 1.0) * inColor * texture(diffuseMap, inUV);
  outColor = color;
}
