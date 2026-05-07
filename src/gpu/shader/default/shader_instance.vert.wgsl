struct MatrixStorage {
  data: array<vec4<f32>, 4>,
};

struct MatUniform {
  world: MatrixStorage,
  view: MatrixStorage,
  proj: MatrixStorage,
  user: MatrixStorage,
  lightView: MatrixStorage,
  lightProj: MatrixStorage,
};

@binding(0) @group(1) var<uniform> Mat: MatUniform;

fn unpackMatrix(m: MatrixStorage) -> mat4x4<f32> {
  return mat4x4<f32>(m.data[0], m.data[1], m.data[2], m.data[3]);
}

struct VertexInput {
  @location(0) position: vec3<f32>,
  @location(1) normal: vec3<f32>,
  @location(2) uv: vec2<f32>,
  @location(3) color: vec4<f32>,
  @location(4) m0: vec4<f32>,
  @location(5) m1: vec4<f32>,
  @location(6) m2: vec4<f32>,
  @location(7) m3: vec4<f32>,
};

struct VertexOutput {
  @builtin(position) position: vec4<f32>,
  @location(0) uv: vec2<f32>,
  @location(1) color: vec4<f32>,
};

@vertex
fn VSMain(input: VertexInput) -> VertexOutput {
  var output: VertexOutput;
  let world = mat4x4<f32>(input.m0, input.m1, input.m2, input.m3);
  let view = unpackMatrix(Mat.view);
  let proj = unpackMatrix(Mat.proj);
  output.position = vec4<f32>(input.position, 1.0) * world * view * proj;
  output.uv = input.uv;
  output.color = input.color;
  return output;
}
