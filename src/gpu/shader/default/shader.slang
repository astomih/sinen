#define UNIFORM(name, bind)                                                    \
  [[vk::binding(bind, 1)]]                                                     \
  cbuffer name : register(c##bind, space1)

#define COMBINED_IMAGE_SAMPLER(name, bind, set)                                \
  [[vk::binding(bind, set)]]                                                   \
  Sampler2D name : register(t##bind, space##set) : register(s##bind, space##set);

struct VSInput {
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
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
  float4x4 pvw = mul(proj, mul(view, world));
  output.out = mul(pvw, float4(input.position, 1.0));
  output.uv = input.uv;
  output.color = input.color;
  return output;
};

struct FSInput {
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

struct FSOutput {
  float4 color : COLOR;
};

COMBINED_IMAGE_SAMPLER(sampledTexture, 0, 2)

[shader("fragment")]
FSOutput FSMain(FSInput input) {
  FSOutput output;
  output.color = sampledTexture.Sample(input.uv) * input.color;
  return output;
}

