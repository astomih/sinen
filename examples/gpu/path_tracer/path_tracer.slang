#include "../../common.slang"

static const uint MaxDepth = 3;
static const uint SamplesPerPixel = 10;
static const uint HeaderBytes = 16;
static const uint TriangleRecordBytes = 48;
static const uint MaterialRecordBytes = 32;

struct RayPayload {
  float3 color;
  uint seed;
  uint depth;
  uint mode;
  uint occluded;
};

SinenRaytracingASInRaytracing(scene, 0);
SinenRWByteAddressBufferInRaytracing(outputPixels, 0);
SinenRWByteAddressBufferInRaytracing(sceneData, 1);

uint hash(uint x) {
  x ^= x >> 16;
  x *= 0x7feb352du;
  x ^= x >> 15;
  x *= 0x846ca68bu;
  x ^= x >> 16;
  return x;
}

float rand01(inout uint seed) {
  seed = hash(seed);
  return float(seed & 0x00ffffffu) / 16777216.0;
}

float3 randomUnitVector(inout uint seed) {
  float z = rand01(seed) * 2.0 - 1.0;
  float a = rand01(seed) * 6.28318530718;
  float r = sqrt(max(0.0, 1.0 - z * z));
  return float3(cos(a) * r, sin(a) * r, z);
}

float3 randomHemisphere(float3 normal, inout uint seed) {
  float3 v = randomUnitVector(seed);
  return dot(v, normal) < 0.0 ? -v : v;
}

float4 loadFloat4(uint byteOffset) {
  return asfloat(sceneData.Load4(byteOffset));
}

uint materialBaseByte() { return uint(loadFloat4(0).x + 0.5); }

float3 lightPosition() { return loadFloat4(0).yzw; }

void loadTriangle(uint primitiveIndex, out float3 v0, out float3 v1,
                  out float3 v2, out uint materialIndex) {
  uint base = HeaderBytes + primitiveIndex * TriangleRecordBytes;
  float4 a = loadFloat4(base);
  float4 b = loadFloat4(base + 16);
  float4 c = loadFloat4(base + 32);
  v0 = a.xyz;
  v1 = b.xyz;
  v2 = c.xyz;
  materialIndex = uint(a.w + 0.5);
}

void loadMaterial(uint materialIndex, out float3 albedo, out float metallic,
                  out float3 emission, out float roughness) {
  uint base = materialBaseByte() + materialIndex * MaterialRecordBytes;
  float4 a = loadFloat4(base);
  float4 b = loadFloat4(base + 16);
  albedo = a.rgb;
  metallic = a.a;
  emission = b.rgb;
  roughness = b.a;
}

float3 traceScene(float3 origin, float3 direction, uint depth,
                  inout uint seed) {
  RayDesc ray;
  ray.Origin = origin;
  ray.Direction = direction;
  ray.TMin = 0.003;
  ray.TMax = 1000.0;

  RayPayload payload;
  payload.color = float3(0.0);
  payload.seed = seed;
  payload.depth = depth;
  payload.mode = 0;
  payload.occluded = 0;
  TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 1, 0, ray, payload);
  seed = payload.seed;
  return payload.color;
}

bool traceShadow(float3 origin, float3 direction, float distance,
                 inout uint seed) {
  RayDesc ray;
  ray.Origin = origin;
  ray.Direction = direction;
  ray.TMin = 0.004;
  ray.TMax = max(0.004, distance - 0.035);

  RayPayload payload;
  payload.color = float3(0.0);
  payload.seed = seed;
  payload.depth = MaxDepth;
  payload.mode = 1;
  payload.occluded = 0;
  TraceRay(scene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xff, 0, 1, 0, ray,
           payload);
  seed = payload.seed;
  return payload.occluded != 0;
}

uint packColor(float3 color) {
  color = max(color, float3(0.0));
  color = color / (color + float3(1.0));
  color = pow(saturate(color), float3(1.0 / 2.2));
  uint r = (uint)round(color.r * 255.0);
  uint g = (uint)round(color.g * 255.0);
  uint b = (uint)round(color.b * 255.0);
  return r | (g << 8) | (b << 16) | (255 << 24);
}

[shader("raygeneration")]
void RayGenMain() {
  uint2 pixel = DispatchRaysIndex().xy;
  uint2 size = DispatchRaysDimensions().xy;
  float2 resolution = float2(size);
  float aspect = resolution.x / max(1.0, resolution.y);

  float3 cameraPos = float3(0.0, 1.05, -4.25);
  float3 target = float3(0.0, 0.85, 0.35);
  float3 forward = normalize(target - cameraPos);
  float3 right = normalize(cross(forward, float3(0.0, 1.0, 0.0)));
  float3 up = cross(right, forward);
  float focalScale = tan(0.5 * radians(46.0));

  uint seed = hash(pixel.x * 1973u + pixel.y * 9277u + 0x9e3779b9u);
  float3 accum = float3(0.0);
  for (uint i = 0; i < SamplesPerPixel; ++i) {
    float2 jitter = float2(rand01(seed), rand01(seed));
    float2 uv = (float2(pixel) + jitter) / resolution;
    float2 sensor = float2((uv.x * 2.0 - 1.0) * aspect, 1.0 - uv.y * 2.0);
    float3 direction = normalize(forward + sensor.x * right * focalScale +
                                 sensor.y * up * focalScale);
    accum += traceScene(cameraPos, direction, 0, seed);
  }

  float3 color = accum / float(SamplesPerPixel);
  outputPixels.Store((pixel.y * size.x + pixel.x) * 4, packColor(color));
}

[shader("miss")]
void MissMain(inout RayPayload payload) {
  if (payload.mode == 1) {
    payload.occluded = 0;
    return;
  }

  float t = saturate(WorldRayDirection().y * 0.5 + 0.5);
  payload.color =
      lerp(float3(0.025, 0.035, 0.055), float3(0.24, 0.31, 0.42), t);
}

[shader("closesthit")]
void ClosestHitMain(inout RayPayload payload,
                    in BuiltInTriangleIntersectionAttributes attributes) {
  if (payload.mode == 1) {
    payload.occluded = 1;
    return;
  }

  float3 v0;
  float3 v1;
  float3 v2;
  uint materialIndex;
  loadTriangle(PrimitiveIndex(), v0, v1, v2, materialIndex);

  float3 albedo;
  float metallic;
  float3 emission;
  float roughness;
  loadMaterial(materialIndex, albedo, metallic, emission, roughness);

  float3 rayDirection = normalize(WorldRayDirection());
  float3 hitPosition = WorldRayOrigin() + rayDirection * RayTCurrent();
  float3 normal = normalize(cross(v1 - v0, v2 - v0));
  if (dot(normal, rayDirection) > 0.0) {
    normal = -normal;
  }

  if (length(emission) > 0.001) {
    payload.color = emission;
    return;
  }

  uint seed = payload.seed;
  float3 toLight = lightPosition() - hitPosition;
  float lightDistance = length(toLight);
  float3 lightDirection = toLight / max(lightDistance, 0.0001);
  float nDotL = saturate(dot(normal, lightDirection));

  float3 color = albedo * 0.025;
  if (payload.depth == 0 && nDotL > 0.0 &&
      !traceShadow(hitPosition + normal * 0.006, lightDirection, lightDistance,
                   seed)) {
    float attenuation = 1.0 / max(lightDistance * lightDistance, 0.35);
    float3 lightColor = float3(17.0, 14.5, 10.5);
    color += albedo * lightColor * nDotL * attenuation;
  }

  float fresnel = pow(1.0 - saturate(dot(-rayDirection, normal)), 5.0);
  float reflective = saturate(metallic + fresnel * 0.35);
  if (payload.depth + 1 < MaxDepth && reflective > 0.02) {
    float3 reflected = reflect(rayDirection, normal);
    float3 glossy = normalize(
        lerp(reflected, randomHemisphere(normal, seed), roughness * roughness));
    float3 reflectedColor = traceScene(hitPosition + normal * 0.008, glossy,
                                       payload.depth + 1, seed);
    color = lerp(color, reflectedColor * lerp(float3(1.0), albedo, metallic),
                 reflective);
  }

  payload.seed = seed;
  payload.color = color;
}
