struct VSInput {
  float3 position : POSITION;
  float2 uv : TEXCOORD0;
  float4 color : COLOR0;
};

struct VSOutput {
  float4 out : SV_Position;
  float2 uv : TEXCOORD0;
  float4 color : COLOR0;
};

VSOutput VSMain(VSInput input) {
  VSOutput output;
  output.out = float4(input.position, 1.0);
  output.uv = input.uv;
  output.color = input.color;
  return output;
}

struct FSInput {
  float4 pos : SV_Position;
  float2 uv : TEXCOORD0;
  float4 color : COLOR0;
};

struct FSOutput {
  float4 color : SV_Target0;
};

FSOutput FSMain(FSInput input) {
  FSOutput output;
  output.color = float4(input.uv, 1.0, 1.0) * input.color;
  return output;
}
