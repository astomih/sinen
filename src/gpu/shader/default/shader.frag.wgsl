@binding(0) @group(2) var sampledTexture_texture_0 : texture_2d<f32>;

@binding(1) @group(2) var sampledTexture_sampler_0 : sampler;

struct FSOutput_0
{
    @location(0) color_0 : vec4<f32>,
};

struct pixelInput_0
{
    @location(0) uv_0 : vec2<f32>,
    @location(1) color_1 : vec4<f32>,
};

@fragment
fn FSMain( _S1 : pixelInput_0, @builtin(position) position_0 : vec4<f32>) -> FSOutput_0
{
    var output_0 : FSOutput_0;
    ;
    output_0.color_0 = (textureSample((sampledTexture_texture_0), (sampledTexture_sampler_0), (_S1.uv_0))) * _S1.color_1;
    return output_0;
}

