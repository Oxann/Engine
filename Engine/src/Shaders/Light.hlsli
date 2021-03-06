#define MAX_DIRECTIONAL_LIGHT_COUNT 4
#define MAX_POINT_LIGHT_COUNT 4
#define MINLIGHT 0.000001f

cbuffer AmbientLight : register(b0)
{
    // w is intensity
    float4 AmbientLight;
}

cbuffer DirectionalLights : register(b1)
{
    uint DirectionalLightCount;
    float3 padding;
    struct DirectionalLight
    {
        float3 ligth; //color * intensity
        float3 direction;
    } directionalLights[MAX_DIRECTIONAL_LIGHT_COUNT];
};

cbuffer PointLights : register(b2)
{
    uint PointLightCount;
    float PointLight_Constant;
    float PointLight_Linear;
    float PointLight_Quadratic;
    struct PointLight
    {
        float3 light; //color * intensity
        float3 position;
    } pointLights[MAX_POINT_LIGHT_COUNT];
}