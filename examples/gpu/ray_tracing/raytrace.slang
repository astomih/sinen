#include "../../common.slang"

struct RayPayload {
  float3 color;
};

SinenRaytracingASInRaytracing(scene, 0);
SinenRWByteAddressBufferInRaytracing(outputPixels, 0);

[shader("raygeneration")]
void RayGenMain() {
  uint2 pixel = DispatchRaysIndex().xy;
  uint2 size = DispatchRaysDimensions().xy;
  float2 uv = (float2(pixel) + 0.5) / float2(size);

  RayDesc ray;
  ray.Origin = float3(0.0, 0.0, -2.0);
  ray.Direction = normalize(float3(uv * 2.0 - 1.0, 1.5));
  ray.TMin = 0.001;
  ray.TMax = 1000.0;

  RayPayload payload;
  payload.color = float3(0.0, 0.0, 0.0);
  TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, payload);

  uint r = (uint)round(saturate(payload.color.r) * 255.0);
  uint g = (uint)round(saturate(payload.color.g) * 255.0);
  uint b = (uint)round(saturate(payload.color.b) * 255.0);
  uint packedRgba = r | (g << 8) | (b << 16) | (255 << 24);
  outputPixels.Store((pixel.y * size.x + pixel.x) * 4, packedRgba);
}

[shader("miss")]
void MissMain(inout RayPayload payload) {
  payload.color = float3(0.02, 0.04, 0.08);
}

[shader("closesthit")]
void ClosestHitMain(inout RayPayload payload,
                    in BuiltInTriangleIntersectionAttributes attributes) {
  payload.color = float3(1.0 - attributes.barycentrics.x,
                         attributes.barycentrics.x, attributes.barycentrics.y);
}
