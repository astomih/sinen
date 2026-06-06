#define UNIFORM_VERT(name, bind)                                               \
  [[vk::binding(bind, 1)]]                                                     \
  cbuffer name : register(b##bind, space1)

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

UNIFORM_VERT(Mat, 0) {
  float4x4 wvp;
}

[shader("vertex")]
VSOutput VSMain(VSInput input) {
  VSOutput output;
  output.out = mul(wvp, float4(input.position, 1.0));
  output.uv = input.uv;
  output.color = input.color;
  return output;
};

struct FSInput {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

struct FSOutput {
  float4 color : SV_Target;
};

COMBINED_IMAGE_SAMPLER(sampledTexture, 0, 2)

[shader("fragment")]
FSOutput FSMain(FSInput input) {
  FSOutput output;
  output.color = sampledTexture.Sample(input.uv) * input.color;
  return output;
}

