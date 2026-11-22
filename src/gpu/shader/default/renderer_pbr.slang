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
RENDERER_SAMPLER(Sampler2D, giAtlas, 8, 16, 17)
#else
RENDERER_SAMPLER(Sampler2D, giAtlas, 5, 10, 11)
#endif

UNIFORM_FRAG(Lighting, 0) {
  float4 cameraPositionAndMaxMip;
  // Normal map enabled, SDR exposure, fill light enabled, GI enabled.
  float4 settings;
  DirectLight directLight;
  DirectLight fillLight;
  float4 giMinimumAndIntensity;
  float4 giInverseExtent;
  float4 giProbeCounts;
}

float3 probeLobe(float3 cell, float lobe) {
  float2 size = float2(giProbeCounts.x * 6.0, giProbeCounts.y * giProbeCounts.z);
  float2 texel = float2(cell.x * 6.0 + lobe, cell.z * giProbeCounts.y + cell.y);
  return giAtlas.SampleLevel((texel + 0.5) / size, 0.0).rgb;
}

float3 probeIrradiance(float3 position, float3 normal) {
  float3 grid = saturate((position - giMinimumAndIntensity.xyz) * giInverseExtent.xyz) *
                (giProbeCounts.xyz - 1.0);
  float3 base = min(floor(grid), giProbeCounts.xyz - 2.0);
  float3 f = grid - base;
  float3 value = float3(0.0);
  for (int z = 0; z < 2; ++z) {
    for (int y = 0; y < 2; ++y) {
      for (int x = 0; x < 2; ++x) {
        float3 cell = base + float3(x, y, z);
        float weight = (x == 1 ? f.x : 1.0 - f.x) *
                       (y == 1 ? f.y : 1.0 - f.y) *
                       (z == 1 ? f.z : 1.0 - f.z);
        value += weight * (probeLobe(cell, normal.x >= 0.0 ? 0.0 : 1.0) * normal.x * normal.x +
                           probeLobe(cell, normal.y >= 0.0 ? 2.0 : 3.0) * normal.y * normal.y +
                           probeLobe(cell, normal.z >= 0.0 ? 4.0 : 5.0) * normal.z * normal.z);
      }
    }
  }
  return value * giMinimumAndIntensity.w;
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
  if (settings.x == 0.0 || det == 0.0 ||
      dot(tangent, tangent) == 0.0) {
    return normal;
  }
  tangent = normalize(tangent) * sign(det);
  float3 bitangent = cross(normal, tangent);
  float3 uvY = dpdy * duvdx.x - dpdx * duvdy.x;
  bitangent *= sign(dot(bitangent, uvY) * sign(det));
  return normalize(mapped.x * tangent + mapped.y * bitangent + mapped.z * normal);
}

float3 evaluateLight(DirectLight source, float3 position, float3 normal,
                     float3 view, float nDotV, float3 reflectance,
                     float3 albedo, float roughness, float metallic) {
  LightSample sample = sampleDirectLight(source, position);
  float3 light = sample.direction;
  float nDotL = max(dot(normal, light), 0.0);
  if (nDotL == 0.0 || sample.strength <= 0.0) {
    return float3(0.0);
  }
  float3 halfVector = normalize(view + light);
  float3 fresnel = schlickFresnel(saturate(dot(halfVector, view)), reflectance);
  float distribution = ggxDistribution(saturate(dot(normal, halfVector)), roughness);
  float geometry = smithGeometry(nDotV, nDotL, roughness);
  float3 specular = distribution * geometry * fresnel /
                    max(4.0 * nDotV * nDotL, 1e-6);
  float3 diffuse = (1.0 - fresnel) * (1.0 - metallic) * albedo / PI;
  float3 radiance = source.colorAndInnerCos.rgb * sample.strength;
  return (diffuse + specular) * radiance * nDotL;
}

float3 linearToSrgb(float3 value) {
  return select(value <= 0.0031308, value * 12.92,
                1.055 * pow(value, 1.0 / 2.4) - 0.055);
}

float3 srgbToLinear(float3 value) {
  return select(value <= 0.04045, value / 12.92,
                pow((value + 0.055) / 1.055, 2.4));
}

[shader("fragment")]
float4 FSMain(Surface input) : SV_Target {
  float3 baseColor = albedoMap.Sample(input.uv).rgb;
  // The SDR path accepts conventional sRGB base-color textures in UNORM slots.
  float3 albedo = (settings.y > 0.0 ? srgbToLinear(baseColor) : baseColor) * input.color.rgb;
  // glTF packs roughness in green and metallic in blue.
  float2 roughnessMetallic = metallicRoughnessMap.Sample(input.uv).gb;
  float roughness = clamp(roughnessMetallic.x, 0.045, 1.0);
  float metallic = saturate(roughnessMetallic.y);
  float occlusion = occlusionMap.Sample(input.uv).r;
  float3 emissive = emissiveMap.Sample(input.uv).rgb;
  float3 normal = surfaceNormal(input);
  float3 view = normalize(cameraPositionAndMaxMip.xyz - input.worldPosition);
  float3 reflectance = lerp(float3(0.04), albedo, metallic);
  float nDotV = max(dot(normal, view), 1e-6);
  float3 direct = evaluateLight(directLight, input.worldPosition, normal, view,
                                nDotV, reflectance, albedo, roughness, metallic);
  if (settings.z != 0.0) {
    direct += evaluateLight(fillLight, input.worldPosition, normal, view,
                            nDotV, reflectance, albedo, roughness, metallic);
  }

#ifdef SINEN_RENDERER_IBL
  float3 envFresnel = schlickFresnel(saturate(nDotV), reflectance);
  // A local volume replaces the environment's diffuse term, retaining IBL specular.
  float3 irradiance = settings.w != 0.0 ? probeIrradiance(input.worldPosition, normal)
                                        : irradianceMap.Sample(normal).rgb;
  float3 reflected = reflect(-view, normal);
  float3 prefiltered =
      prefilteredMap.SampleLevel(reflected, roughness * cameraPositionAndMaxMip.w).rgb;
  float2 brdf = brdfLUT.Sample(float2(saturate(nDotV), roughness)).rg;
  float3 ambient = (1.0 - envFresnel) * (1.0 - metallic) * irradiance * albedo / PI +
                   prefiltered * (reflectance * brdf.x + brdf.y);
#else
  float3 ambient = settings.w != 0.0
      ? (1.0 - schlickFresnel(saturate(nDotV), reflectance)) * (1.0 - metallic) *
        probeIrradiance(input.worldPosition, normal) * albedo / PI
      : albedo * 0.4;
#endif
  float3 result = direct + ambient * occlusion + emissive;
  if (settings.y > 0.0) {
    float3 exposed = max(result * settings.y, 0.0);
    result = linearToSrgb(exposed / (1.0 + exposed));
  }
  return float4(result, 1.0);
}
