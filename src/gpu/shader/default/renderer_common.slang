#ifndef SINEN_RENDERER_COMMON
#define SINEN_RENDERER_COMMON

#define UNIFORM_VERT(name, slot)                                              \
  [[vk::binding(slot, 1)]] cbuffer name : register(b##slot, space1)
#define UNIFORM_FRAG(name, slot)                                              \
  [[vk::binding(slot, 3)]] cbuffer name : register(b##slot, space3)

#ifdef SINEN_TARGET_WEBGPU
#define RENDERER_SAMPLER(type, name, slot, tex, samp)                          \
  [[vk::binding(tex, 2)]]                                                      \
  type name : register(t##tex, space2) : register(s##samp, space2);
#else
#define RENDERER_SAMPLER(type, name, slot, tex, samp)                          \
  [[vk::binding(slot, 2)]]                                                     \
  type name : register(t##slot, space2) : register(s##slot, space2);
#endif

struct Surface {
  float4 clipPosition : SV_Position;
  float3 worldPosition : TEXCOORD0;
  float3 worldNormal : TEXCOORD1;
  float2 uv : TEXCOORD2;
  float4 color : COLOR;
};

#endif
