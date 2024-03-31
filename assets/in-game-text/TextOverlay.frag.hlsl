Texture2D textureFont : register(t0, space0);
SamplerState samplerFont : register(s0, space0);

struct Input
{
    [[vk::location(0)]] float2 uv : TEXCOORD0;
};

struct Output
{
    float4 color : SV_Target0;
};

float4 main(Input input) : SV_TARGET
{
    float color = textureFont.Sample(samplerFont, input.uv).r;
    return float4(color.xxx, 1.0);
}