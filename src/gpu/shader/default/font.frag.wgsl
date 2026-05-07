@binding(0) @group(2) var sampledTexture_texture_0 : texture_2d<f32>;

@binding(1) @group(2) var sampledTexture_sampler_0 : sampler;

struct SLANG_ParameterGroup_FontParams_std140_0
{
    @align(16) textColor_0 : vec4<f32>,
    @align(16) atlasParams_0 : vec4<f32>,
};

@binding(1) @group(3) var<uniform> FontParams_0 : SLANG_ParameterGroup_FontParams_std140_0;
fn median3_0( v_0 : vec3<f32>) -> f32
{
    var _S1 : f32 = v_0.x;
    var _S2 : f32 = v_0.y;
    return max(min(_S1, _S2), min(max(_S1, _S2), v_0.z));
}

fn screenPxRange_0( uv_0 : vec2<f32>) -> f32
{
    var pxRange_0 : f32 = FontParams_0.atlasParams_0.z;
    return max(0.5f * dot(vec2<f32>(pxRange_0, pxRange_0) / FontParams_0.atlasParams_0.xy, vec2<f32>(1.0f) / max((fwidth((uv_0))), vec2<f32>(0.00009999999747379f, 0.00009999999747379f))), 1.0f);
}

struct FSOutput_0
{
    @location(0) color_0 : vec4<f32>,
};

struct pixelInput_0
{
    @location(0) uv_1 : vec2<f32>,
    @location(1) color_1 : vec4<f32>,
};

@fragment
fn FSMain( _S3 : pixelInput_0, @builtin(position) position_0 : vec4<f32>) -> FSOutput_0
{
    ;
    var color_2 : vec4<f32> = _S3.color_1 * FontParams_0.textColor_0;
    var output_0 : FSOutput_0;
    output_0.color_0 = vec4<f32>(color_2.xyz, color_2.w * clamp(screenPxRange_0(_S3.uv_1) * (median3_0((textureSample((sampledTexture_texture_0), (sampledTexture_sampler_0), (_S3.uv_1))).xyz) - 0.5f) + 0.5f, 0.0f, 1.0f));
    return output_0;
}

