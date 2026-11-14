struct DirectLight {
  float4 positionAndType;
  float4 directionAndIntensity;
  float4 colorAndInnerCos;
  float4 outerCos;
};

struct LightSample {
  float3 direction;
  float strength;
};

LightSample sampleDirectLight(DirectLight light, float3 position) {
  LightSample result;
  result.direction = -light.directionAndIntensity.xyz;
  result.strength = light.directionAndIntensity.w;

  float type = light.positionAndType.w;
  if (type == 0.0) {
    return result;
  }

  float3 toLight = light.positionAndType.xyz - position;
  float distanceSquared = max(dot(toLight, toLight), 1e-6);
  result.direction = toLight * rsqrt(distanceSquared);
  result.strength /= distanceSquared * 0.23;
  if (type == 2.0) {
    float cosAngle = dot(-result.direction, light.directionAndIntensity.xyz);
    float inner = light.colorAndInnerCos.w;
    float outer = light.outerCos.x;
    float falloff = inner > outer ? smoothstep(outer, inner, cosAngle)
                                  : step(outer, cosAngle);
    result.strength *= falloff;
  }
  return result;
}
