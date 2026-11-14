#include "renderer_common.slang"
#include "renderer_light.slang"

RENDERER_SAMPLER(Sampler2D, albedoMap, 0, 0, 1)

UNIFORM_FRAG(ToonLighting, 0) {
  float4 shadowColorAndBands;
  DirectLight directLight;
}

[shader("fragment")]
float4 FSMain(Surface input) : SV_Target {
  float4 base = albedoMap.Sample(input.uv) * input.color;
  LightSample sample = sampleDirectLight(directLight, input.worldPosition);
  float diffuse = saturate(max(dot(normalize(input.worldNormal), sample.direction), 0.0) *
                           sample.strength);
  float bands = shadowColorAndBands.w;
  float shade = min(floor(diffuse * bands), bands - 1.0) / (bands - 1.0);
  return float4(base.rgb * lerp(shadowColorAndBands.rgb,
                                directLight.colorAndInnerCos.rgb, shade), base.a);
}
