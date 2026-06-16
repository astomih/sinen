#include "../../common.slang"

struct VSInput {
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

struct VSOutput {
  float4 out : SV_Position;
  float4 shadowCoord : TEXCOORD0;
  float2 uv : TEXCOORD1;
};

UNIFORM_VERT(Mat, 0) { float4x4 wvp; }
UNIFORM_VERT(shadowMat, 1) {
  float4x4 world;
  float4x4 shadowViewProj;
}

[shader("vertex")]
VSOutput VSMain(VSInput input) {
  VSOutput output;
  float4 wpos = mul(world, float4(input.position, 1.0));
  output.out = mul(wvp, float4(input.position, 1.0));
  output.shadowCoord = mul(shadowViewProj, wpos);
  output.uv = input.uv;
  return output;
};

struct FSInput {
  float4 position : SV_Position;
  float4 shadowCoord : TEXCOORD0;
  float2 uv : TEXCOORD1;
};

struct FSOutput {
  float4 color : SV_Target;
};

COMBINED_IMAGE_SAMPLER(baseColor, 0, 0, 1)
COMBINED_IMAGE_SAMPLER(depthTexture, 1, 2, 3)

[shader("fragment")]
FSOutput FSMain(FSInput input) {
  FSOutput output;
  output.color = baseColor.Sample(input.uv);

  float3 proj = input.shadowCoord.xyz / input.shadowCoord.w;

  float2 depthUV = proj.xy * 0.5f + 0.5f;

  if (any(depthUV < 0.0f) || any(depthUV > 1.0f)) {
    return output;
  }

  depthUV.y = 1.0 - depthUV.y;
  const float bias = 0.05f;
#ifdef SINEN_TARGET_WEBGPU
  float depthSM = depthTexture.SampleLevel(depthUV, 0.0f).r;
#else
  float depthSM = depthTexture.Sample(depthUV).r;
#endif
  float shadow = (depthSM + bias) < proj.z ? 1.0f : 0.0f;

  output.color *= lerp(1.0f, 0.5f, shadow);
  return output;
}

