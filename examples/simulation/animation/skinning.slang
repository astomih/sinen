struct VSInput {
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
  float4 bone_index : BLENDINDICES;
  float4 bone_weight : BLENDWEIGHT;
};

struct VSOutput {
  float4 out : SV_Position;
  float2 uv : TEXCOORD0;
  float3 normal : NORMAL;
  float4 color : COLOR;
};

#define UNIFORM_VERT(name, bind)                                               \
  [[vk::binding(bind, 1)]]                                                     \
  cbuffer name : register(b##bind, space1)

UNIFORM_VERT(Mat, 0) {
  float4x4 model;
  float4x4 view;
  float4x4 proj;
}

UNIFORM_VERT(skinning, 1) { float4x4 bone[256]; }
[shader("vertex")]
VSOutput VSMain(VSInput input) {
  VSOutput output;
  float4x4 mvp = mul((proj), mul((view), (model)));
  float4x4 skin = float4x4(0);
  for (int i = 0; i < 4; i++)
    skin += ((bone[int(input.bone_index[i])]) * input.bone_weight[i]);
  output.out = mul(mvp, mul(skin, float4(input.position, 1.0)));
  output.uv = input.uv;
  output.normal = normalize(mul(skin, float4(input.normal, 0.0)).xyz);
  output.color = input.color;
  return output;
};

struct FSInput {
  float4 position : SV_Position;
  float2 uv : TEXCOORD0;
  float3 normal : NORMAL;
  float4 color : COLOR;
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

COMBINED_IMAGE_SAMPLER(sampledTexture, 0, 0, 1)

[shader("fragment")]
FSOutput FSMain(FSInput input) {
  FSOutput output;
  float3 lightDir = float3(-0.5, -0.5, 1.0);
  float diffuse = max(dot(input.normal, lightDir), 0.0);
  float3 ambient = float3(0.2);
  float3 result = diffuse * input.color.rgb + ambient;
  result *= sampledTexture.Sample(input.uv).rgb;
  output.color = float4(result, input.color.a);

  return output;
}
