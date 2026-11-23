#include "../../common.slang"

struct VSInput {
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};
struct Varyings {
  float4 position : SV_Position;
  float3 world : TEXCOORD0;
  float3 normal : TEXCOORD1;
  float foam : TEXCOORD2;
};
UNIFORM_VERT(Mat, 0) { float4x4 wvp; }
UNIFORM_VERT(OceanWorld, 1) { float4x4 world; }
UNIFORM_FRAG(OceanView, 0) { float3 eye; float padding; }

[shader("vertex")]
Varyings VSMain(VSInput input) {
  Varyings output;
  output.position = mul(wvp, float4(input.position, 1));
  output.world = mul(world, float4(input.position, 1)).xyz;
  output.normal = normalize(mul((float3x3)world, input.normal));
  output.foam = input.color.r;
  return output;
}

float3 sky(float3 direction) {
  float t = saturate(direction.y);
  float3 color = lerp(float3(0.62, 0.75, 0.83), float3(0.12, 0.32, 0.59), pow(t, 0.45));
  float3 sun = normalize(float3(-0.5, 0.23, -0.8));
  color += float3(4.0, 3.0, 1.8) * pow(saturate(dot(direction, sun)), 600);
  return color;
}

[shader("fragment")]
float4 FSMain(Varyings input) : SV_Target {
  float3 n = normalize(input.normal);
  float3 v = normalize(eye - input.world);
  float3 l = normalize(float3(-0.5, 0.23, -0.8));
  // Schlick Fresnel at the air/water interface (IOR 1.333).
  float fresnel = 0.0204 + 0.9796 * pow(1 - saturate(dot(n, v)), 5);
  float3 reflected = sky(reflect(-v, n));
  float crest = saturate(input.world.y * 0.18 + 0.2);
  float3 water = lerp(float3(0.005, 0.055, 0.075), float3(0.015, 0.22, 0.24), crest);
  water *= 0.5 + 0.5 * saturate(dot(n, l));
  float3 halfVector = normalize(l + v);
  float glint = pow(saturate(dot(n, halfVector)), 180) * 2.0;
  float3 color = lerp(water, reflected, fresnel) + glint * float3(1, 0.85, 0.6);
  color = lerp(color, float3(0.82, 0.9, 0.91), saturate(input.foam));
  float fog = 1 - exp(-length(eye - input.world) * 0.004);
  color = lerp(color, float3(0.62, 0.75, 0.83), fog);
  return float4(pow(saturate(color), 1.0 / 2.2), 1);
}
