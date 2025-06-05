#define UNIFORM(name, bind)                                                    \
  [[vk::binding(bind, 1)]]                                                     \
  cbuffer name : register(c##bind, space1)

struct VSInput {
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
  float4 m1;
  float4 m2;
  float4 m3;
  float4 m4;
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
  float4x4 user;
  float4x4 light_view;
  float4x4 light_proj;
}

[shader("vertex")]
VSOutput VSMain(VSInput input) {
  VSOutput output;
  float4x4 inWorldPos;
  inWorldPos = float4x4(input.m1, input.m2, input.m3, input.m4);
  output.out = mul(proj * view * inWorldPos, float4(input.position, 1.0));
  output.uv = input.uv;
  output.color = input.color;
  return output;
};
