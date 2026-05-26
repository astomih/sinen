static const uint ImageWidth = 384;
static const uint ImageHeight = 288;

[[vk::binding(0, 6)]]
RaytracingAccelerationStructure scene : register(t0, space6);

[[vk::binding(0, 0)]]
RWByteAddressBuffer outputPixels : register(u0, space0);

bool traceScene(float2 uv, out float hitT) {
  RayDesc ray;
  ray.Origin = float3(0.0, 0.0, -2.0);
  ray.Direction = normalize(float3(uv * 2.0 - 1.0, 1.5));
  ray.TMin = 0.001;
  ray.TMax = 1000.0;

  RayQuery<RAY_FLAG_NONE> query;
  query.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, ray);
  query.Proceed();

  if (query.CommittedStatus() == COMMITTED_TRIANGLE_HIT) {
    hitT = query.CommittedRayT();
    return true;
  }

  hitT = 0.0;
  return false;
}

[shader("compute")]
[numthreads(8, 8, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID) {
  if (dispatchThreadID.x >= ImageWidth || dispatchThreadID.y >= ImageHeight) {
    return;
  }

  uint2 pixel = dispatchThreadID.xy;
  float2 uv = (float2(pixel) + 0.5) / float2(ImageWidth, ImageHeight);

  float hitT = 0.0;
  float3 color = float3(0.02, 0.04, 0.08);
  if (traceScene(uv, hitT)) {
    float shade = saturate(1.0 - hitT * 0.08);
    color = float3(1.0, 0.35, 0.12) * shade + float3(0.0, 0.25, 0.75) * (1.0 - shade);
  }

  uint r = (uint)round(saturate(color.r) * 255.0);
  uint g = (uint)round(saturate(color.g) * 255.0);
  uint b = (uint)round(saturate(color.b) * 255.0);
  uint packedRgba = r | (g << 8) | (b << 16) | (255 << 24);
  outputPixels.Store((pixel.y * ImageWidth + pixel.x) * 4, packedRgba);
}
