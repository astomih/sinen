struct VSInput {
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

struct VSOutput {
  float4 out : SV_Position;
  float3 pos : TEXCOORD1;
};

#define UNIFORM_VERT(name, bind)                                               \
  [[vk::binding(bind, 1)]]                                                     \
  cbuffer name //: register(c##bind, space1)
#define UNIFORM_FRAG(name, bind)                                               \
  [[vk::binding(bind, 3)]]                                                     \
  cbuffer name //: register(c##bind, space1)

UNIFORM_VERT(Mat, 0) {
  float4x4 world;
  float4x4 view;
  float4x4 proj;
}
[shader("vertex")]
VSOutput VSMain(VSInput input) {
  VSOutput output;

  float4x4 viewWithoutTranslation = view;
  viewWithoutTranslation[0][3] = 0.0;
  viewWithoutTranslation[1][3] = 0.0;
  viewWithoutTranslation[2][3] = 0.0;

  output.out =
      mul(proj, mul(viewWithoutTranslation, float4(input.position.xyz, 1.0)));
  output.pos = input.position.xyz;
  return output;
};

struct FSInput {
  float3 pos : TEXCOORD1;
};

struct FSOutput {
  float4 color : COLOR;
};
#define COMBINED_IMAGE_SAMPLER(name, bind)                                     \
  [[vk::binding(bind, 2)]]                                                     \
  Sampler2D name; //: register(t##bind, space##set) : register(s##bind,
                  //: space##set);

[[vk::binding(0, 2)]]
SamplerCube cubemap;

[shader("fragment")]
FSOutput FSMain(FSInput input) {
  FSOutput output;
  output.color = cubemap.Sample(input.pos);
  return output;
}

