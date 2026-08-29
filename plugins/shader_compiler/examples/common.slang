#define UNIFORM_VERT(name, bind)                                               \
  [[vk::binding(bind, 1)]]                                                     \
  cbuffer name : register(b##bind, space1)
#define UNIFORM_FRAG(name, bind)                                               \
  [[vk::binding(bind, 3)]]                                                     \
  cbuffer name : register(b##bind, space3)

#ifdef SINEN_TARGET_WEBGPU
#define COMBINED_IMAGE_SAMPLER(name, logicalBind, textureBind, samplerBind)    \
  [[vk::binding(textureBind, 2)]]                                              \
  Sampler2D name : register(t##textureBind, space2) : register(s##samplerBind, space2);
#define COMBINED_CUBE_SAMPLER(name, logicalBind, textureBind, samplerBind)     \
  [[vk::binding(textureBind, 2)]]                                              \
  SamplerCube name : register(t##textureBind, space2) : register(s##samplerBind, space2);
#else
#define COMBINED_IMAGE_SAMPLER(name, logicalBind, textureBind, samplerBind)    \
  [[vk::binding(logicalBind, 2)]]                                              \
  Sampler2D name : register(t##logicalBind, space2) : register(s##logicalBind, space2);
#define COMBINED_CUBE_SAMPLER(name, logicalBind, textureBind, samplerBind)     \
  [[vk::binding(logicalBind, 2)]]                                              \
  SamplerCube name : register(t##logicalBind, space2) : register(s##logicalBind, space2);
#endif

#define SinenRWByteAddressBufferInCompute(name, bind)                          \
  [[vk::binding(bind, 0)]]                                                     \
  RWByteAddressBuffer name : register(u##bind, space0)

#define SinenRaytracingASInCompute(name, bind)                                 \
  [[vk::binding(bind, 6)]]                                                     \
  RaytracingAccelerationStructure name : register(t##bind, space6)

#define SinenRWByteAddressBufferInRaytracing(name, bind)                       \
  [[vk::binding(bind + 8, 4)]]                                                 \
  RWByteAddressBuffer name : register(u##bind, space4)

#define SinenRaytracingASInRaytracing(name, bind)                              \
  [[vk::binding(bind, 4)]]                                                     \
  RaytracingAccelerationStructure name : register(t##bind, space4)
