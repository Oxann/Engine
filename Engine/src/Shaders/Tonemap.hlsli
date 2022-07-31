

float3 Reinhard(float3 color)
{
	return color / (color + float3(1.0f, 1.0f, 1.0f));
}

float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float3 LinearToSRGB(float3 color)
{
    return pow(color.xyz, float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f));
}
