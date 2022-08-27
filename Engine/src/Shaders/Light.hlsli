#define MAX_DIRECTIONAL_LIGHT_COUNT 4
#define MAX_POINT_LIGHT_COUNT 4
#define MINLIGHT 0.000001f
#define PI 3.14159265f

#define HASSOFTSHADOWS(light) light.shadowType == 1

cbuffer AmbientLight : register(b0)
{
    // w is intensity
    float4 ambientLight;
}

cbuffer DirectionalLights : register(b1)
{
    uint DirectionalLightCount;
    float3 padding;
    struct DirectionalLight
    {
        float depthBias;
        float3 light; //color * intensity
        float3 direction;
        int shadowType; // 0 is hard, 1 is soft
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
        float range;
        float3 light; //color * intensity
        float3 position;
    } pointLights[MAX_POINT_LIGHT_COUNT];
}


Texture2D shadowMap : register(t9);
SamplerState hardShadowsSampler : register(s9);
SamplerComparisonState softShadowsSampler : register(s10);

TextureCube diffuseIrradianceMap : register (t11);
TextureCube specularIrradianceMap : register (t12);
SamplerState irradianceMapSampler : register(s11);

Texture2D BRDFLUT : register(t13);
SamplerState BRDFLUTSampler : register(s13);

struct Surface
{
    float4 diffuseColor;
    float metalness;
    float roughness;
};

float DistributionGGX(float NdotH, float roughness)
{
    float roughnessSquare = roughness * roughness;
    float a2 = roughnessSquare * roughnessSquare;

    float num = a2;
    float denom = (NdotH * NdotH * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;

    if (denom < 0.00000001f) return 1.0f;
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    const float k = pow((roughness + 1.0f), 2) / 8.0f;
    const float num = NdotV;
    const float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 FresnelSchlick(float VdotH, float3 f0)
{
    return f0 + (1.0f - f0) * pow(saturate(1.0f - VdotH), 5.0f);
}

float3 FresnelSchlickRoughness(float roughness, float cosTheta, float3 f0)
{
    return f0 + (max(float3(1.0f.xxx - roughness), f0) - f0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float3 CalculateBRDF(Surface surface, float3 lightDirection, float3 viewDirection, float3 normal)
{
    const float3 halfway = normalize(lightDirection + viewDirection);
    const float NdotH = saturate(dot(normal, halfway));
    const float NdotV = saturate(dot(normal, viewDirection));
    const float NdotL = saturate(dot(normal, lightDirection));
    const float VdotH = saturate(dot(viewDirection, halfway));
    const float3 f0 = lerp(float3(0.04f, 0.04f, 0.04f), surface.diffuseColor.xyz, surface.metalness);

    const float D = DistributionGGX(NdotH, surface.roughness);
    const float G = GeometrySmith(NdotV, NdotL, surface.roughness);
    const float3 F = FresnelSchlick(VdotH, f0);
    const float denom = max(4.0 * NdotV * NdotL, 0.0000001);
    const float3 specular = D * F * G / denom;

    const float3 diffuse = (float3(1.0f, 1.0f, 1.0f) - F) * (1.0f - surface.metalness) * surface.diffuseColor.xyz / PI;

    return diffuse + specular;
}

float3 CalculateDirectionalLight(DirectionalLight light, Surface surface, float3 viewDirection, float3 normal)
{
    const float3 lightDir = -light.direction;
    const float3 NdotL = saturate(dot(normal, lightDir));
    return CalculateBRDF(surface, lightDir, viewDirection, normal) * light.light * NdotL;
}

float3 CalculatePointLight(PointLight light, Surface surface, float3 viewDirection, float3 normal, float3 worldSpacePosition)
{
    const float3 pixelToLightVec = light.position - worldSpacePosition;
    const float distance = length(pixelToLightVec);

    if (distance > light.range) return float3(0.0f,0.0f,0.0f);

    const float attenuation = 1.0f / (distance * distance);
    const float3 lightDir = pixelToLightVec / distance;
    const float3 NdotL = saturate(dot(normal, lightDir));
    return CalculateBRDF(surface, lightDir, viewDirection, normal) * light.light * NdotL * attenuation;
}

float3 CalculateEnvironment(Surface surface, float3 viewDirection, float3 normal)
{
    const float NdotV = saturate(dot(viewDirection, normal));

    const float3 R = reflect(-viewDirection, normal);
    const float3 prefilteredColor = specularIrradianceMap.SampleLevel(irradianceMapSampler, R, surface.roughness * 9).rgb;
    const float2 envBRDF = BRDFLUT.Sample(BRDFLUTSampler, float2(NdotV, surface.roughness)).rg;

    const float3 f0 = lerp(float3(0.04f, 0.04f, 0.04f), surface.diffuseColor.xyz, surface.metalness);
    const float3 F = FresnelSchlickRoughness(surface.roughness, NdotV, f0);
    const float3 Kd = float3(1.0f - F) * (1.0f - surface.metalness);
    const float3 diffuse = surface.diffuseColor.xyz * diffuseIrradianceMap.Sample(irradianceMapSampler, normal).rgb * Kd;
    const float3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    return diffuse + specular;
}