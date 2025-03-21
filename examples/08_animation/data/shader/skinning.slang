struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float4 color : COLOR;
    float4 bone_index;
    float4 bone_weight;
};

struct VSOutput
{
    float4 out : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR;
};

#define UNIFORM(name, bind)  \
    [[vk::binding(bind, 1)]] \
    cbuffer name : register(c##bind, space1)

UNIFORM(Mat, 0)
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
    float4x4 user;
    float4x4 light_view;
    float4x4 light_proj;
}

UNIFORM(skinning, 1)
{
    float4x4 bone[256];
}

[shader("vertex")]
VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4x4 skin = float4x4(1, 0, 0, 0,
                             0, 1, 0, 0,
                             0, 0, 1, 0,
                             0, 0, 0, 1);

    skin += bone[int(input.bone_index.x)] * input.bone_weight.x;
    skin += bone[int(input.bone_index.y)] * input.bone_weight.y;
    skin += bone[int(input.bone_index.z)] * input.bone_weight.z;
    skin += bone[int(input.bone_index.w)] * input.bone_weight.w;
    float4x4 pvw = mul(proj, mul(view, world));
    float4 pos = mul(skin, float4(input.position, 1.0));
    output.out = mul(pvw, pos);
    output.uv = input.uv;
    output.color = input.color;
    return output;
};

struct FSInput
{
    float2 uv : TEXCOORD0;
    float4 color : COLOR;
};

struct FSOutput
{
    float4 color : COLOR;
};
#define COMBINED_IMAGE_SAMPLER(name, bind) \
    [[vk::binding(bind, 2)]]               \
    Sampler2D name : register(t##bind, space2) : register(s##bind, space2);

COMBINED_IMAGE_SAMPLER(sampledTexture, 0)

[shader("fragment")]
FSOutput FSMain(FSInput input)
{
    FSOutput output;
    output.color = sampledTexture.Sample(input.uv) * input.color;
    return output;
}

