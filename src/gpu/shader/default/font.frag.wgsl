@binding(0) @group(2) var sampledTextureTexture: texture_2d<f32>;
@binding(1) @group(2) var sampledTextureSampler: sampler;

struct FontParams {
  textColor: vec4<f32>,
  atlasParams: vec4<f32>,
};

@binding(1) @group(3) var<uniform> Params: FontParams;

struct FragmentInput {
  @location(0) uv: vec2<f32>,
  @location(1) color: vec4<f32>,
};

struct FragmentOutput {
  @location(0) color: vec4<f32>,
};

fn median3(v: vec3<f32>) -> f32 {
  return max(min(v.r, v.g), min(max(v.r, v.g), v.b));
}

fn screenPxRange(uv: vec2<f32>) -> f32 {
  let pxRange = Params.atlasParams.z;
  let atlasSize = Params.atlasParams.xy;
  let unitRange = vec2<f32>(pxRange, pxRange) / atlasSize;
  let screenTexSize = vec2<f32>(1.0) / max(fwidth(uv),
                                           vec2<f32>(0.0001, 0.0001));
  return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

@fragment
fn FSMain(input: FragmentInput) -> FragmentOutput {
  let sampleColor = textureSample(sampledTextureTexture,
                                  sampledTextureSampler, input.uv);
  let signedDistance = median3(sampleColor.rgb);
  let coverage = clamp(screenPxRange(input.uv) * (signedDistance - 0.5) + 0.5,
                       0.0, 1.0);
  let color = input.color * Params.textColor;

  var output: FragmentOutput;
  output.color = vec4<f32>(color.rgb, color.a * coverage);
  return output;
}
