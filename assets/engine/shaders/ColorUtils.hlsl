#ifndef COLOR_UTILS
#define COLOR_UTILS

float3 ApplyExposureToneMapping(float3 color)
{
    float exposure = 1.0f;
    if (color.r > exposure) {
        exposure = color.r;
    }
    if (color.g > exposure) {
        exposure = color.g;
    }
    if (color.b > exposure) {
        exposure = color.b;
    }
    exposure = 1 / exposure;

    return float3(1.0) - exp(-color * exposure);
}

float3 ApplyGammaCorrection(float3 color)
{
    return pow(color, float3(1.0f/2.2f)); 
}

#endif