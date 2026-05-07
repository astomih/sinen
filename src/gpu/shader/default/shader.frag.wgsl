@binding(0) @group(2) var sampledTextureTexture: texture_2d<f32>;
@binding(1) @group(2) var sampledTextureSampler: sampler;

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
  output.color = textureSample(sampledTextureTexture, sampledTextureSampler,
                               input.uv) * input.color;
  return output;
}
