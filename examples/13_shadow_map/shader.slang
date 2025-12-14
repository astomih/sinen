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

#define UNIFORM_VERT(name, bind)                                               \
  [[vk::binding(bind, 1)]]                                                     \
  cbuffer name //: register(c##bind, space1)
#define UNIFORM_FRAG(name, bind)                                               \
  [[vk::binding(bind, 3)]]                                                     \
  cbuffer name //: register(c##bind, space1)

UNIFORM_VERT(Mat, 0) {
  float4x4 world;
  float4x4 view;
  float4x4 proj;
}
UNIFORM_VERT(shadowMat, 1) {
  float4x4 shadowView;
  float4x4 shadowProj;
}

[shader("vertex")]
VSOutput VSMain(VSInput input) {
  VSOutput output;
  output.out = mul(mul(proj, mul(view, world)), float4(input.position, 1.0));
  output.shadowCoord =
      mul(mul(shadowProj, mul(shadowView, world)), float4(input.position, 1.0));
  output.uv = input.uv;
  return output;
};

struct FSInput {
  float4 shadowCoord : TEXCOORD0;
  float2 uv : TEXCOORD1;
};

struct FSOutput {
  float4 color : COLOR;
};

#define COMBINED_IMAGE_SAMPLER(name, bind)                                     \
  [[vk::binding(bind, 2)]]                                                     \
  Sampler2D name; //: register(t##bind, space##set) : register(s##bind,
                  //: space##set);
                  //
COMBINED_IMAGE_SAMPLER(baseColor, 0)
COMBINED_IMAGE_SAMPLER(depthTexture, 1)

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
  float depthSM = depthTexture.Sample(depthUV).r;
  float shadow = (depthSM + bias) < proj.z ? 1.0f : 0.0f;

  output.color *= lerp(1.0f, 0.5f, shadow);
  return output;
}

