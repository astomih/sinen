#define UNIFORM_VERT(name, bind)                                               \
  [[vk::binding(bind, 1)]]                                                     \
  cbuffer name : register(b##bind, space1)

#define COMBINED_IMAGE_SAMPLER(name, bind, set)                                \
  [[vk::binding(bind, set)]]                                                   \
  Sampler2D name : register(t##bind, space##set) : register(s##bind, space##set);

struct VSInput {
  float3 position : POSITION0;
  float3 normal : NORMAL0;
  float2 uv : TEXCOORD0;
  float4 color : COLOR0;
};

struct VSOutput {
  float4 out : SV_Position0;
  float2 uv : TEXCOORD0;
  float4 color : COLOR0;
};

UNIFORM_VERT(Mat, 0) {
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
  float4x4 pvw = mul(proj, mul(view, world));
  output.out = mul(pvw, float4(input.position, 1.0));
  output.uv = input.uv;
  output.color = input.color;
  return output;
};

struct FSInput {
  float2 uv : TEXCOORD0;
  float4 color : COLOR0;
};

struct FSOutput {
  float4 color : SV_Target0;
};

COMBINED_IMAGE_SAMPLER(sampledTexture, 0, 2)

[shader("fragment")]
FSOutput FSMain(FSInput input) {
  FSOutput output;
  output.color = sampledTexture.Sample(input.uv) * input.color;
  return output;
}

