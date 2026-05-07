struct SLANG_ParameterGroup_RectColor_std140_0
{
    @align(16) color_0 : vec4<f32>,
};

@binding(1) @group(3) var<uniform> RectColor_0 : SLANG_ParameterGroup_RectColor_std140_0;
struct FSOutput_0
{
    @location(0) color_1 : vec4<f32>,
};

struct pixelInput_0
{
    @location(0) uv_0 : vec2<f32>,
    @location(1) color_2 : vec4<f32>,
};

@fragment
fn FSMain( _S1 : pixelInput_0, @builtin(position) position_0 : vec4<f32>) -> FSOutput_0
{
    var output_0 : FSOutput_0;
    output_0.color_1 = RectColor_0.color_0;
    return output_0;
}

