struct MatrixStorage {
  data: array<vec4<f32>, 4>,
};

struct MatUniform {
  world: MatrixStorage,
  view: MatrixStorage,
  proj: MatrixStorage,
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
};

struct VertexOutput {
  @builtin(position) position: vec4<f32>,
  @location(4) pos: vec3<f32>,
};

@vertex
fn VSMain(input: VertexInput) -> VertexOutput {
  var view = unpackMatrix(Mat.view);
  view[0][3] = 0.0;
  view[1][3] = 0.0;
  view[2][3] = 0.0;

  var output: VertexOutput;
  output.position = vec4<f32>(input.position, 1.0) * view *
                    unpackMatrix(Mat.proj);
  output.pos = input.position;
  return output;
}
