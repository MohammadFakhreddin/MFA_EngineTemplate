#include "../ColorUtils.hlsl"

struct PSIn {
    float4 position : SV_POSITION;
    float2 baseColorUV : TEXCOORD0;
    float3 worldNormal : NORMAL;
};

struct PSOut {
    float4 color : SV_Target0;
};

struct Material
{
    float4 color;
    int hasBaseColorTexture;
    int placeholder0;
    int placeholder1;
    int placeholder2;
};

sampler textureSampler : register(s1, space0);

ConstantBuffer <Material> material: register(b0, space1);

Texture2D baseColorTexture : register(t1, space1);

PSOut main(PSIn input) {
    PSOut output;

    float3 color;
    float alpha;
    if (material.hasBaseColorTexture == 0)
    {
        color = material.color.rgb;
        alpha = material.color.a;
    }
    else
    {
        color = baseColorTexture.Sample(textureSampler, input.baseColorUV);
        alpha = 1.0;
    }

    
    // float3 lightDir = float3(-1.0f, -1.0f, -1.0f);
    float3 lightDir = float3(1.0f, 1.0f, 1.0f);
    // float ambient = 0.25f;
    float ambient = 0.25f;

    float dot = dot(normalize(-lightDir), normalize(input.worldNormal));
    float3 dirLight = max(dot, 0.0f) * color;
    float3 color2 = dirLight + ambient * color;
    // color = ambient * color;

    color2 = ApplyExposureToneMapping(color2);
    // Gamma correct
    color2 = ApplyGammaCorrection(color2); 

    output.color = float4(color2, alpha);
    return output;
}