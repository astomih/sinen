#define UNIFORM_VERT(name, bind)                                               \
  [[vk::binding(bind, 1)]]                                                     \
  cbuffer name : register(b##bind, space1)

#define COMBINED_IMAGE_SAMPLER(name, bind, set)                                \
  [[vk::binding(bind, set)]]                                                   \
  Sampler2D name : register(t##bind, space##set) : register(s##bind, space##set);

struct FSInput {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD0;
  float4 color : COLOR0;
};

struct FSOutput {
  float4 color : SV_Target;
};

COMBINED_IMAGE_SAMPLER(sampledTexture, 0, 2)

[shader("fragment")]
FSOutput FSMain(FSInput input) {
  FSOutput output;
  float coverage = sampledTexture.Sample(input.uv).r;
  output.color = float4(input.color.rgb, input.color.a * coverage);
  return output;
}
