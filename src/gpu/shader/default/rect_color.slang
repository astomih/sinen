#define UNIFORM_FRAG(name, bind)                                               \
  [[vk::binding(bind, 3)]]                                                     \
  cbuffer name : register(b##bind, space1)

struct FSInput {
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

struct FSOutput {
  float4 color : SV_Target0;
};

UNIFORM_FRAG(RectColor, 1) { float4 color; };

[shader("fragment")]
FSOutput FSMain(FSInput input) {
  FSOutput output;
  output.color = color;
  return output;
}
