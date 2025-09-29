struct VSInput {
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
  float4 bone_index;
  float4 bone_weight;
};

struct VSOutput {
  float4 out : SV_Position;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

#define UNIFORM(name, bind)                                                    \
  [[vk::binding(bind, 1)]]                                                     \
  cbuffer name //: register(c##bind, space1)

UNIFORM(Mat, 0) {
  float4x4 model;
  float4x4 view;
  float4x4 proj;
}

UNIFORM(skinning, 1) { float4x4 bone[256]; }

[shader("vertex")]
VSOutput VSMain(VSInput input) {
  VSOutput output;
  float4x4 mvp = mul((proj), mul((view), (model)));
  float4 skin = float4(0);
  for (int i = 0; i < 4; i++)
    skin += mul((bone[int(input.bone_index[i])]), float4(input.position, 1.0)) *
            input.bone_weight[i];
  output.out = mul(mvp, skin);
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
#define COMBINED_IMAGE_SAMPLER(name, bind)                                     \
  [[vk::binding(bind, 2)]]                                                     \
  Sampler2D name : register(t##bind, space2) : register(s##bind, space2);

COMBINED_IMAGE_SAMPLER(sampledTexture, 0)

[shader("fragment")]
FSOutput FSMain(FSInput input) {
  FSOutput output;
  output.color = input.color;
  return output;
}
