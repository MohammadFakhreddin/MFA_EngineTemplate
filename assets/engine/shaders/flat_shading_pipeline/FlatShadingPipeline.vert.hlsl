struct VSIn {
    float3 position : POSITION0;
    float2 baseColorUV : TEXCOORD0;
    float3 normal : NORMAL;
};

struct VSOut {
    float4 position : SV_POSITION;
    float2 baseColorUV : TEXCOORD0;
    float3 worldNormal : NORMAL;
};

struct ViewProjectionBuffer {
    float4x4 viewProjection;
};

ConstantBuffer <ViewProjectionBuffer> vpBuff: register(b0, space0);

struct PushConsts
{
    float4x4 model;
};

[[vk::push_constant]]
cbuffer {
    PushConsts pushConsts;
};

VSOut main(VSIn input) {
    VSOut output;

    float4x4 mvpMatrix = mul(vpBuff.viewProjection, pushConsts.model);
    output.position = mul(mvpMatrix, float4(input.position, 1.0));
    output.baseColorUV = input.baseColorUV;
    output.worldNormal = mul(pushConsts.model, float4(input.normal, 0.0f)).xyz;

    return output;
}