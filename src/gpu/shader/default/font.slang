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

float median3(float3 v) { return max(min(v.r, v.g), min(max(v.r, v.g), v.b)); }

[shader("fragment")]
FSOutput FSMain(FSInput input) {
  FSOutput output;
  float4 sampleColor = sampledTexture.Sample(input.uv);
  float coverage = sampleColor.r;
  if (sampleColor.g > 0.0 || sampleColor.b > 0.0) {
    float signedDistance = median3(sampleColor.rgb) - 0.5;
    float antialiasWidth = max(fwidth(signedDistance), 0.0001);
    coverage = saturate(signedDistance / antialiasWidth + 0.5);
  }
  output.color = float4(input.color.rgb, input.color.a * coverage);
  return output;
}
