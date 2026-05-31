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

#define UNIFORM_VERT(name, bind)                                               \
  [[vk::binding(bind, 1)]]                                                     \
  cbuffer name : register(b##bind, space1)

UNIFORM_VERT(Mat, 0) {
  float4x4 wvp;
}

[shader("vertex")]
VSOutput VSMain(VSInput input) {
  VSOutput output;
  output.out = mul(wvp, float4(input.position, 1.0));
  output.pos = output.out;
  return output;
};

struct FSInput {
  float4 position : SV_Position;
  float4 pos : TEXCOORD0;
};

struct FSOutput {
  float4 color : SV_Target;
};
#ifdef SINEN_TARGET_WEBGPU
#define COMBINED_IMAGE_SAMPLER(name, logicalBind, textureBind, samplerBind)    \
  [[vk::binding(textureBind, 2)]]                                              \
  Sampler2D name : register(t##textureBind, space2)                            \
      : register(s##samplerBind, space2);
#else
#define COMBINED_IMAGE_SAMPLER(name, logicalBind, textureBind, samplerBind)    \
  [[vk::binding(logicalBind, 2)]]                                              \
  Sampler2D name : register(t##logicalBind, space2)                            \
      : register(s##logicalBind, space2);
#endif

[shader("fragment")]
FSOutput FSMain(FSInput input) {
  FSOutput output;
  float zNDC = input.pos.z / input.pos.w;
  output.color = float4(zNDC, zNDC, zNDC, 1.0);
  return output;
}

