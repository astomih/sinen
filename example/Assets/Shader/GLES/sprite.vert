#version 300 es
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
out vec2 outUV;
uniform mat4 uWorld;
uniform mat4 uView;
uniform mat4 uProj;
// Normal (in world space)
out vec3 fragNormal;
// Position (in world space)
out vec3 fragWorldPos;
void main()
{
  vec4 worldpos = vec4(inPos,1.0) * uWorld;
	fragWorldPos = worldpos.xyz;
  gl_Position = uProj * uView * uWorld * vec4(inPos,1.0);
	fragNormal = (uWorld*vec4(inNormal, 0.0f)).xyz;
  outUV = inUV;
}