@binding(0) @group(2) var cubemap_texture_0 : texture_cube<f32>;

@binding(1) @group(2) var cubemap_sampler_0 : sampler;

struct FSOutput_0
{
    @location(0) color_0 : vec4<f32>,
};

struct pixelInput_0
{
    @location(4) pos_0 : vec3<f32>,
};

@fragment
fn FSMain( _S1 : pixelInput_0, @builtin(position) position_0 : vec4<f32>) -> FSOutput_0
{
    var output_0 : FSOutput_0;
    ;
    output_0.color_0 = (textureSample((cubemap_texture_0), (cubemap_sampler_0), (_S1.pos_0)));
    return output_0;
}

