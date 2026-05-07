#define UNIFORM_VERT(name, bind)                                               \
  [[vk::binding(bind, 1)]]                                                     \
  cbuffer name : register(b##bind, space1)

#define COMBINED_IMAGE_SAMPLER(name, bind, set)                                \
  [[vk::binding(bind, set)]]                                                   \
  Sampler2D name : register(t##bind, space##set) : register(s##bind, space##set);

#define UNIFORM_FRAG(name, bind)                                               \
  [[vk::binding(bind, 3)]]                                                     \
  cbuffer name : register(b##bind, space3)

struct FSInput {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

struct FSOutput {
  float4 color : SV_Target;
};

COMBINED_IMAGE_SAMPLER(sampledTexture, 0, 2)

UNIFORM_FRAG(FontParams, 1) {
  float4 textColor;

  // atlasParams.xy = atlas pixel size
  // atlasParams.z  = msdf pxRange used when generating the atlas
  // atlasParams.w  = unused
  float4 atlasParams;
};

float median3(float3 v) { return max(min(v.r, v.g), min(max(v.r, v.g), v.b)); }

float screenPxRange(float2 uv) {
  float pxRange = atlasParams.z;
  float2 atlasSize = atlasParams.xy;

  float2 unitRange = float2(pxRange, pxRange) / atlasSize;
  float2 screenTexSize = 1.0 / max(fwidth(uv), float2(0.0001, 0.0001));

  return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

[shader("fragment")]
FSOutput FSMain(FSInput input) {
  FSOutput output;

  float4 sampleColor = sampledTexture.Sample(input.uv);

  float signedDistance = median3(sampleColor.rgb);
  float pxRange = screenPxRange(input.uv);
  float coverage = saturate(pxRange * (signedDistance - 0.5) + 0.5);

  float4 color = input.color * textColor;
  output.color = float4(color.rgb, color.a * coverage);

  return output;
}
