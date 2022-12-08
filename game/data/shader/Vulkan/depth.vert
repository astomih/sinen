#version 450
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 inUV;
layout(location=3) in vec4 inColor;
layout(location=4) in vec4 m1;
layout(location=5) in vec4 m2;
layout(location=6) in vec4 m3;
layout(location=7) in vec4 m4;
layout(location=0) out vec2 outUV;
layout(location=1) out vec4 outColor;


layout(set=0,binding=0) uniform Matrices
{
  mat4 world;
  mat4 view;
  mat4 proj;
  mat4 light_view;
  mat4 light_proj;
};

out gl_PerVertex
{
  vec4 gl_Position;
};

void main()
{
  gl_Position =  vec4(inPos, 1.0);


  outUV = inUV;
  outColor = inColor;
}
