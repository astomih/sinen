#ifdef SINEN_TARGET_WEBGPU
[[vk::binding(0, 2)]] SamplerCube sky : register(t0, space2) : register(s1, space2);
#else
[[vk::binding(0, 2)]] SamplerCube sky : register(t0, space2) : register(s0, space2);
#endif

struct Input { float4 position : SV_Position; float3 direction : TEXCOORD0; };
[shader("fragment")]
float4 FSMain(Input input) : SV_Target {
  float3 value = max(sky.Sample(input.direction).rgb, 0.0);
  value = value / (1.0 + value);
  value = select(value <= 0.0031308, value * 12.92,
                 1.055 * pow(value, 1.0 / 2.4) - 0.055);
  return float4(value, 1.0);
}
