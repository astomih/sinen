struct _MatrixStorage_float4x4std140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct SLANG_ParameterGroup_Mat_std140_0
{
    @align(16) world_0 : _MatrixStorage_float4x4std140_0,
    @align(16) view_0 : _MatrixStorage_float4x4std140_0,
    @align(16) proj_0 : _MatrixStorage_float4x4std140_0,
};

@binding(0) @group(1) var<uniform> Mat_0 : SLANG_ParameterGroup_Mat_std140_0;
fn unpackStorage_0( _S1 : _MatrixStorage_float4x4std140_0) -> mat4x4<f32>
{
    return mat4x4<f32>(_S1.data_0[i32(0)][i32(0)], _S1.data_0[i32(0)][i32(1)], _S1.data_0[i32(0)][i32(2)], _S1.data_0[i32(0)][i32(3)], _S1.data_0[i32(1)][i32(0)], _S1.data_0[i32(1)][i32(1)], _S1.data_0[i32(1)][i32(2)], _S1.data_0[i32(1)][i32(3)], _S1.data_0[i32(2)][i32(0)], _S1.data_0[i32(2)][i32(1)], _S1.data_0[i32(2)][i32(2)], _S1.data_0[i32(2)][i32(3)], _S1.data_0[i32(3)][i32(0)], _S1.data_0[i32(3)][i32(1)], _S1.data_0[i32(3)][i32(2)], _S1.data_0[i32(3)][i32(3)]);
}

struct VSOutput_0
{
    @builtin(position) out_0 : vec4<f32>,
    @location(4) pos_0 : vec3<f32>,
};

struct vertexInput_0
{
    @location(0) position_0 : vec3<f32>,
    @location(1) normal_0 : vec3<f32>,
    @location(2) uv_0 : vec2<f32>,
    @location(3) color_0 : vec4<f32>,
};

@vertex
fn VSMain( _S2 : vertexInput_0) -> VSOutput_0
{
    var viewWithoutTranslation_0 : mat4x4<f32> = unpackStorage_0(Mat_0.view_0);
    viewWithoutTranslation_0[i32(0)][i32(3)] = 0.0f;
    viewWithoutTranslation_0[i32(1)][i32(3)] = 0.0f;
    viewWithoutTranslation_0[i32(2)][i32(3)] = 0.0f;
    var output_0 : VSOutput_0;
    var _S3 : vec3<f32> = _S2.position_0.xyz;
    output_0.out_0 = ((((((vec4<f32>(_S3, 1.0f)) * (viewWithoutTranslation_0)))) * (unpackStorage_0(Mat_0.proj_0))));
    output_0.pos_0 = _S3;
    return output_0;
}

