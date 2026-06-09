static const uint ImageWidth = 128;
static const uint ImageHeight = 128;

[[vk::binding(0, 1)]]
RWByteAddressBuffer outputPixels : register(u0, space1);

[shader("compute")]
[numthreads(8, 8, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID) {
  if (dispatchThreadID.x >= ImageWidth || dispatchThreadID.y >= ImageHeight) {
    return;
  }

  float2 uv = float2(dispatchThreadID.xy) / float2(ImageWidth - 1, ImageHeight - 1);
  uint r = (uint)round(uv.x * 255.0);
  uint g = (uint)round(uv.y * 255.0);
  uint b = (uint)round((0.5 + 0.5 * sin((uv.x + uv.y) * 24.0)) * 255.0);
  uint a = 255;

  uint packedRgba = r | (g << 8) | (b << 16) | (a << 24);
  uint pixelOffset = (dispatchThreadID.y * ImageWidth + dispatchThreadID.x) * 4;
  outputPixels.Store(pixelOffset, packedRgba);
}
