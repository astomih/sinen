#define UNIFORM_FRAG(name, bind)                                               \
  [[vk::binding(bind, 3)]]                                                     \
  cbuffer name : register(b##bind, space3)

struct FSInput {
  float4 position : SV_Position;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

struct FSOutput {
  float4 color : SV_Target;
};

UNIFORM_FRAG(RectColor, 1) { float4 color; };

[shader("fragment")]
FSOutput FSMain(FSInput input) {
  FSOutput output;
  output.color = color;
  return output;
}
