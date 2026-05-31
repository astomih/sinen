#define UNIFORM_VERT(name, bind)                                               \
  [[vk::binding(bind, 1)]]                                                     \
  cbuffer name : register(b##bind, space1)

#ifdef SINEN_TARGET_WEBGPU
#define COMBINED_CUBE_SAMPLER(name, logicalBind, textureBind, samplerBind)     \
  [[vk::binding(textureBind, 2)]]                                              \
  SamplerCube name : register(t##textureBind, space2)                          \
      : register(s##samplerBind, space2);
#else
#define COMBINED_CUBE_SAMPLER(name, logicalBind, textureBind, samplerBind)     \
  [[vk::binding(logicalBind, 2)]]                                              \
  SamplerCube name : register(t##logicalBind, space2)                          \
      : register(s##logicalBind, space2);
#endif

struct VSInput {
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

struct VSOutput {
  float4 out : SV_Position;
  float3 direction : TEXCOORD0;
};

UNIFORM_VERT(Mat, 0) {
  float4x4 wvp;
}

[shader("vertex")]
VSOutput VSMain(VSInput input) {
  VSOutput output;
  output.out = mul(wvp, float4(input.position, 1.0));
  output.direction = input.position;
  return output;
}

struct FSInput {
  float4 position : SV_Position;
  float3 direction : TEXCOORD0;
};

struct FSOutput {
  float4 color : SV_Target;
};

COMBINED_CUBE_SAMPLER(cubemap, 0, 0, 1)

[shader("fragment")]
FSOutput FSMain(FSInput input) {
  FSOutput output;
  output.color = cubemap.Sample(input.direction);
  return output;
}
