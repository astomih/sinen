#include "renderer_common.slang"
#include "renderer_light.slang"

static const float PI = 3.14159265358979323846;

RENDERER_SAMPLER(Sampler2D, albedoMap, 0, 0, 1)
RENDERER_SAMPLER(Sampler2D, normalMap, 1, 2, 3)
RENDERER_SAMPLER(Sampler2D, metallicRoughnessMap, 2, 4, 5)
RENDERER_SAMPLER(Sampler2D, emissiveMap, 3, 6, 7)
RENDERER_SAMPLER(Sampler2D, occlusionMap, 4, 8, 9)
#ifdef SINEN_RENDERER_IBL
RENDERER_SAMPLER(Sampler2D, brdfLUT, 5, 10, 11)
RENDERER_SAMPLER(SamplerCube, irradianceMap, 6, 12, 13)
RENDERER_SAMPLER(SamplerCube, prefilteredMap, 7, 14, 15)
#endif

UNIFORM_FRAG(Lighting, 0) {
  float4 cameraPositionAndMaxMip;
  float4 normalMapEnabled;
  DirectLight directLight;
}

float3 schlickFresnel(float cosine, float3 reflectance) {
  return reflectance + (1.0 - reflectance) * pow(1.0 - cosine, 5.0);
}

float ggxDistribution(float nDotH, float roughness) {
  float a = roughness * roughness * roughness * roughness;
  float d = nDotH * nDotH * (a - 1.0) + 1.0;
  return a / max(PI * d * d, 1e-7);
}

float smithGeometry(float nDotV, float nDotL, float roughness) {
  float r = roughness + 1.0;
  float k = r * r / 8.0;
  return nDotV / lerp(nDotV, 1.0, k) * nDotL / lerp(nDotL, 1.0, k);
}

float3 surfaceNormal(Surface input) {
  float3 normal = normalize(input.worldNormal);
  // Reconstruct the tangent frame from the surface and UV gradients.
  float3 dpdx = ddx(input.worldPosition);
  float3 dpdy = ddy(input.worldPosition);
  float2 duvdx = ddx(input.uv);
  float2 duvdy = ddy(input.uv);
  float3 mapped = normalMap.Sample(input.uv).rgb * 2.0 - 1.0;
  float det = duvdx.x * duvdy.y - duvdx.y * duvdy.x;
  float3 tangent = dpdx * duvdy.y - dpdy * duvdx.y;
  tangent -= normal * dot(normal, tangent);
  if (normalMapEnabled.x == 0.0 || det == 0.0 ||
      dot(tangent, tangent) == 0.0) {
    return normal;
  }
  tangent = normalize(tangent) * sign(det);
  float3 bitangent = cross(normal, tangent);
  float3 uvY = dpdy * duvdx.x - dpdx * duvdy.x;
  bitangent *= sign(dot(bitangent, uvY) * sign(det));
  return normalize(mapped.x * tangent + mapped.y * bitangent + mapped.z * normal);
}

[shader("fragment")]
float4 FSMain(Surface input) : SV_Target {
  float3 albedo = albedoMap.Sample(input.uv).rgb * input.color.rgb;
  // glTF packs roughness in green and metallic in blue.
  float2 roughnessMetallic = metallicRoughnessMap.Sample(input.uv).gb;
  float roughness = clamp(roughnessMetallic.x, 0.045, 1.0);
  float metallic = saturate(roughnessMetallic.y);
  float occlusion = occlusionMap.Sample(input.uv).r;
  float3 emissive = emissiveMap.Sample(input.uv).rgb;
  float3 normal = surfaceNormal(input);
  float3 view = normalize(cameraPositionAndMaxMip.xyz - input.worldPosition);
  LightSample sample = sampleDirectLight(directLight, input.worldPosition);
  float3 light = sample.direction;
  float3 halfVector = normalize(view + light);

  float3 reflectance = lerp(float3(0.04), albedo, metallic);
  float nDotV = max(dot(normal, view), 1e-6);
  float nDotL = max(dot(normal, light), 0.0);
  float3 fresnel = schlickFresnel(saturate(dot(halfVector, view)), reflectance);
  float distribution = ggxDistribution(saturate(dot(normal, halfVector)), roughness);
  float geometry = smithGeometry(nDotV, nDotL, roughness);
  float3 specular = distribution * geometry * fresnel /
                    max(4.0 * nDotV * nDotL, 1e-6);
  float3 diffuse = (1.0 - fresnel) * (1.0 - metallic) * albedo / PI;
  float3 radiance = directLight.colorAndInnerCos.rgb * sample.strength;
  float3 direct = (diffuse + specular) * radiance * nDotL;

#ifdef SINEN_RENDERER_IBL
  float3 envFresnel = schlickFresnel(saturate(nDotV), reflectance);
  float3 irradiance = irradianceMap.Sample(normal).rgb;
  float3 reflected = reflect(-view, normal);
  float3 prefiltered =
      prefilteredMap.SampleLevel(reflected, roughness * cameraPositionAndMaxMip.w).rgb;
  float2 brdf = brdfLUT.Sample(float2(saturate(nDotV), roughness)).rg;
  float3 ambient = (1.0 - envFresnel) * (1.0 - metallic) * irradiance * albedo / PI +
                   prefiltered * (reflectance * brdf.x + brdf.y);
#else
  float3 ambient = albedo * 0.4;
#endif
  return float4(direct + ambient * occlusion + emissive, 1.0);
}
