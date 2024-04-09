struct Input
{
    [[vk::location(0)]] float2 position : POSITION0;
    [[vk::location(1)]] float2 uv : TEXCOORD0;
};

struct Output
{
    float4 position : SV_POSITION;
    [[vk::location(0)]] float2 uv : TEXCOORD0;
};

Output main(Input input)
{
    Output output;
    output.position = float4(input.position, 0.0, 1.0);
    output.uv = input.uv;
    return output;
}