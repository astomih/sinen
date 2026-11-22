#include "renderer_light.slang"

[[vk::binding(0, 0)]] RWByteAddressBuffer scene : register(u0, space0);
[[vk::binding(1, 0)]] RWByteAddressBuffer probes : register(u1, space0);
[[vk::binding(0, 1)]] cbuffer GI : register(b0, space1) {
  float4 minimum;
  float4 extent;
  uint4 counts;
  uint4 settings; // frame, diffuse hits, row pitch in bytes
  float4 skyAndHistory;
  DirectLight directLight;
}

static const float PI = 3.14159265358979323846;
static const float EPSILON = 0.0001;

float random01(inout uint state) {
  state ^= state << 13;
  state ^= state >> 17;
  state ^= state << 5;
  return float(state >> 8) * (1.0 / 16777216.0);
}

bool hitBox(float3 origin, float3 direction, float3 lo, float3 hi, float limit) {
  float near = 0.0;
  for (int axis = 0; axis < 3; ++axis) {
    if (abs(direction[axis]) < 1e-8) {
      if (origin[axis] < lo[axis] || origin[axis] > hi[axis]) return false;
    } else {
      float a = (lo[axis] - origin[axis]) / direction[axis];
      float b = (hi[axis] - origin[axis]) / direction[axis];
      near = max(near, min(a, b));
      limit = min(limit, max(a, b));
      if (near > limit) return false;
    }
  }
  return true;
}

// Stackless, depth-first BVH traversal. Each node has an escape index.
bool intersect(float3 origin, float3 direction, inout float distance, out uint triangle) {
  uint node = 0;
  uint nodeCount = scene.Load(0);
  uint triangles = scene.Load(4);
  bool found = false;
  triangle = 0;
  while (node < nodeCount) {
    uint address = 16 + node * 48;
    uint4 lo = scene.Load4(address);
    uint4 hi = scene.Load4(address + 16);
    uint escape = scene.Load(address + 32);
    if (!hitBox(origin, direction, asfloat(lo.xyz), asfloat(hi.xyz), distance)) {
      node = escape;
      continue;
    }
    if (hi.w == 0) { ++node; continue; }
    for (uint i = 0; i < hi.w; ++i) {
      uint t = triangles + (lo.w + i) * 80;
      float3 a = asfloat(scene.Load3(t));
      float3 e1 = asfloat(scene.Load3(t + 16)) - a;
      float3 e2 = asfloat(scene.Load3(t + 32)) - a;
      float3 p = cross(direction, e2);
      float det = dot(e1, p);
      if (abs(det) < 1e-9) continue;
      float3 offset = origin - a;
      float u = dot(offset, p) / det;
      if (u < 0.0 || u > 1.0) continue;
      float3 q = cross(offset, e1);
      float v = dot(direction, q) / det;
      float d = dot(e2, q) / det;
      if (v >= 0.0 && u + v <= 1.0 && d > EPSILON && d < distance) {
        found = true;
        distance = d;
        triangle = t;
      }
    }
    node = escape;
  }
  return found;
}

float3 directDiffuse(float3 position, float3 normal) {
  LightSample sample = sampleDirectLight(directLight, position);
  float cosine = max(dot(normal, sample.direction), 0.0);
  if (cosine == 0.0 || sample.strength <= 0.0) return float3(0.0);
  float distance = directLight.positionAndType.w == 0.0 ? 1e20
      : length(directLight.positionAndType.xyz - position) - 4.0 * EPSILON;
  uint triangle;
  if (intersect(position + normal * (2.0 * EPSILON), sample.direction, distance, triangle))
    return float3(0.0);
  return directLight.colorAndInnerCos.rgb * (sample.strength * cosine / PI);
}

float3 trace(float3 origin, float3 direction, inout uint state) {
  float3 result = float3(0.0);
  float3 throughput = float3(1.0);
  for (uint bounce = 0; bounce < settings.y; ++bounce) {
    float distance = 1e20;
    uint triangle;
    if (!intersect(origin, direction, distance, triangle)) {
      result += throughput * skyAndHistory.rgb;
      break;
    }
    float3 a = asfloat(scene.Load3(triangle));
    float3 b = asfloat(scene.Load3(triangle + 16));
    float3 c = asfloat(scene.Load3(triangle + 32));
    float3 albedo = asfloat(scene.Load3(triangle + 48));
    float3 emission = asfloat(scene.Load3(triangle + 64));
    float3 position = origin + direction * distance;
    float3 normal = normalize(cross(b - a, c - a));
    if (dot(normal, direction) > 0.0) normal = -normal;
    result += throughput * (emission + albedo * directDiffuse(position, normal));
    throughput *= albedo;
    float r = sqrt(random01(state));
    float phi = 2.0 * PI * random01(state);
    float3 tangent = normalize(cross(abs(normal.y) < 0.9 ? float3(0, 1, 0) : float3(1, 0, 0), normal));
    direction = tangent * (r * cos(phi)) + cross(normal, tangent) * (r * sin(phi)) +
                normal * sqrt(max(0.0, 1.0 - r * r));
    origin = position + normal * (2.0 * EPSILON);
  }
  return result;
}

groupshared float3 radiance[64];
groupshared float3 directions[64];

[shader("compute")]
[numthreads(64, 1, 1)]
void CSMain(uint3 probe : SV_GroupID, uint lane : SV_GroupIndex) {
  uint index = (probe.z * counts.y + probe.y) * counts.x + probe.x;
  uint state = (index * 1973 + lane * 9277 + settings.x * 26699 + 1) | 1;
  float h = 1.0 - 2.0 * (float(lane) + random01(state)) / 64.0;
  float phi = 2.0 * PI * random01(state);
  float r = sqrt(max(0.0, 1.0 - h * h));
  float3 direction = float3(r * cos(phi), h, r * sin(phi));
  float3 position = minimum.xyz + extent.xyz * float3(probe) / float3(counts.xyz - 1);
  radiance[lane] = trace(position, direction, state);
  directions[lane] = direction;
  GroupMemoryBarrierWithGroupSync();
  if (lane < 6) {
    float3 axis = float3(0.0);
    axis[lane / 2] = (lane % 2) == 0 ? 1.0 : -1.0;
    float3 value = float3(0.0);
    for (uint i = 0; i < 64; ++i)
      value += radiance[i] * max(dot(axis, directions[i]), 0.0);
    value *= 4.0 * PI / 64.0;
    uint address = (probe.z * counts.y + probe.y) * settings.z + (probe.x * 6 + lane) * 16;
    // The first dispatch does not read uninitialized buffer memory.
    if (settings.x != 0)
      value = lerp(value, asfloat(probes.Load3(address)), skyAndHistory.w);
    probes.Store4(address, asuint(float4(value, 1.0)));
  }
}
