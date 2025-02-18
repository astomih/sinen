#version 450
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inColor;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragWorldPos;
layout(location = 4) out vec4 shadowCoord;
layout(location = 5) out mat4 outUser;

layout(set = 1, binding = 0) uniform Matrices {
  mat4 world;
  mat4 view;
  mat4 proj;
  mat4 user;
  mat4 light_view;
  mat4 light_proj;
};

out gl_PerVertex { vec4 gl_Position; };

void main() {
  fragWorldPos = (vec4(inPos, 1.0) * world).xyz;
  gl_Position = proj * view * world * vec4(inPos, 1.0);
  fragNormal = (world * vec4(inNormal, 0.0f)).xyz;
  outUV = inUV;
  outColor = inColor;
  outUser = user;
  shadowCoord = light_proj * light_view * world * vec4(inPos, 1.0);
}
