#include "renderer_common.slang"

RENDERER_SAMPLER(Sampler2D, albedoMap, 0, 0, 1)

UNIFORM_FRAG(ToonLighting, 0) {
  float4 lightDirectionAndBands;
  float4 shadowColor;
}

[shader("fragment")]
float4 FSMain(Surface input) : SV_Target {
  float4 base = albedoMap.Sample(input.uv) * input.color;
  float diffuse = saturate(dot(normalize(input.worldNormal), lightDirectionAndBands.xyz));
  float bands = lightDirectionAndBands.w;
  float shade = min(floor(diffuse * bands), bands - 1.0) / (bands - 1.0);
  return float4(base.rgb * lerp(shadowColor.rgb, float3(1.0), shade), base.a);
}
