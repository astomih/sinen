@binding(0) @group(2) var cubemapTexture: texture_cube<f32>;
@binding(1) @group(2) var cubemapSampler: sampler;

struct FragmentInput {
  @location(4) pos: vec3<f32>,
};

struct FragmentOutput {
  @location(0) color: vec4<f32>,
};

@fragment
fn FSMain(input: FragmentInput) -> FragmentOutput {
  var output: FragmentOutput;
  output.color = textureSample(cubemapTexture, cubemapSampler, input.pos);
  return output;
}
