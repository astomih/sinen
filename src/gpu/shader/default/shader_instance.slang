#define UNIFORM(name, bind)                                                    \
  [[vk::binding(bind, 1)]]                                                     \
  cbuffer name : register(b##bind, space1)

struct VSInput {
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
  float4 m0 : TEXCOORD4;
  float4 m1 : TEXCOORD5;
  float4 m2 : TEXCOORD6;
  float4 m3 : TEXCOORD7;
};

struct VSOutput {
  float4 out : SV_Position;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

UNIFORM(Mat, 0) {
  float4x4 world;
  float4x4 view;
  float4x4 proj;
}

[shader("vertex")]
VSOutput VSMain(VSInput input) {
  VSOutput output;
  float4x4 instanceWorld = float4x4(input.m0, input.m1, input.m2, input.m3);
  float4x4 pvw = mul(proj, mul(view, instanceWorld));
  output.out = mul(pvw, float4(input.position, 1.0));
  output.uv = input.uv;
  output.color = input.color;
  return output;
};
