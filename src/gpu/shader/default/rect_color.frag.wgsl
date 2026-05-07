struct RectColor {
  color: vec4<f32>,
};

@binding(1) @group(3) var<uniform> Color: RectColor;

struct FragmentInput {
  @location(0) uv: vec2<f32>,
  @location(1) color: vec4<f32>,
};

struct FragmentOutput {
  @location(0) color: vec4<f32>,
};

@fragment
fn FSMain(input: FragmentInput) -> FragmentOutput {
  var output: FragmentOutput;
  output.color = Color.color;
  return output;
}
