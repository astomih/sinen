#version 450

layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 inUV;
layout(location=0) out vec2 outUV;

struct InstanceData
{
  mat4 world;
};

layout(set=0,binding=0) uniform Matrices
{
  mat4 world;
  mat4 view;
  mat4 proj;
};
layout(set=0,binding=2) uniform InstanceParameters
{
  InstanceData data[1000];
};

out gl_PerVertex
{
  vec4 gl_Position;
};

// Normal (in world space)
layout(location=3) out vec3 fragNormal;
// Position (in world space)
layout(location=4) out vec3 fragWorldPos;

void main()
{
  
  /*
  vec4 worldpos = world*vec4(inPos,1.0);
	fragWorldPos = worldpos.xyz;
  gl_Position = vec4(inPos,1.0) * world * view*proj;
	fragNormal = (vec4(inNormal, 0.0f) * world).xyz;
  */
  vec4 worldpos = vec4(inPos,1.0) * world;
	fragWorldPos = worldpos.xyz;
  gl_Position = proj * view * world * vec4(inPos,1.0);
	fragNormal = (world*vec4(inNormal, 0.0f)).xyz;
  outUV = inUV;
}