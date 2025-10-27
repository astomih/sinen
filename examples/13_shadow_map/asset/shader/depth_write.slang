struct VSInput {
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

struct VSOutput {
  float4 out : SV_Position;
  float4 pos : TEXCOORD0;
};

#define UNIFORM(name, bind)                                                    \
  [[vk::binding(bind, 1)]]                                                     \
  cbuffer name //: register(c##bind, space1)

UNIFORM(Mat, 0) {
  float4x4 world;
  float4x4 view;
  float4x4 proj;
}

[shader("vertex")]
VSOutput VSMain(VSInput input) {
  VSOutput output;
  output.out = mul(mul(proj, mul(view, world)), float4(input.position, 1.0));
  output.pos = output.out;
  return output;
};

struct FSInput {
  float4 pos : TEXCOORD0;
};

struct FSOutput {
  float4 color : SV_Target;
};
#define COMBINED_IMAGE_SAMPLER(name, bind, set)                                \
  [[vk::binding(bind, set)]]                                                   \
  Sampler2D name : register(t##bind, space##set) : register(s##bind, space##set);

[shader("fragment")]
FSOutput FSMain(FSInput input) {
  FSOutput output;
  float zNDC = input.pos.z / input.pos.w;
  output.color = float4(zNDC, zNDC, zNDC, 1.0);
  return output;
}

