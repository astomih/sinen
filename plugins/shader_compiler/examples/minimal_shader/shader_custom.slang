#include "../../common.slang"

struct VSInput {
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

struct VSOutput {
  float4 out : SV_Position;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

UNIFORM_VERT(Mat, 0) { float4x4 wvp; }

UNIFORM_FRAG(Test, 1) { float4 test; }

[shader("vertex")]
VSOutput VSMain(VSInput input) {
  VSOutput output;
  output.out = mul(wvp, float4(input.position, 1.0));
  output.uv = input.uv;
  output.color = input.color;
  return output;
};

struct FSInput {
  float4 position : SV_Position;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

struct FSOutput {
  float4 color : SV_Target;
};

COMBINED_IMAGE_SAMPLER(sampledTexture, 0, 0, 1)

[shader("fragment")]
FSOutput FSMain(FSInput input) {
  FSOutput output;
  output.color = sampledTexture.Sample(input.uv) * input.color * test;
  return output;
}
