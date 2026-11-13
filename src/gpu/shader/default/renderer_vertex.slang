#include "renderer_common.slang"

struct Vertex {
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
};

UNIFORM_VERT(Transform, 0) { float4x4 wvp; }
UNIFORM_VERT(World, 1) { float4x4 world; }

[shader("vertex")]
Surface VSMain(Vertex input) {
  Surface output;
  output.clipPosition = mul(wvp, float4(input.position, 1.0));
  output.worldPosition = mul(world, float4(input.position, 1.0)).xyz;

  // The cofactor matrix transforms normals under nonuniform scaling.
  float3x3 linear = (float3x3)world;
  float3x3 cofactors = float3x3(cross(linear[1], linear[2]),
                               cross(linear[2], linear[0]),
                               cross(linear[0], linear[1]));
  float orientation = dot(linear[0], cofactors[0]) < 0.0 ? -1.0 : 1.0;
  output.worldNormal = mul(cofactors, input.normal) * orientation;
  output.uv = input.uv;
  output.color = input.color;
  return output;
}
