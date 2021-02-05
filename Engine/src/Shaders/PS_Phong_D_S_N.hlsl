#include "Light.hlsl"

//Material 
Texture2D diffuseMap : register(t0);
SamplerState diffuseMapSampler : register(s0);

Texture2D specularMap : register(t1);
SamplerState specularMapSampler : register(s1);

Texture2D normalMap : register(t2);
SamplerState normalMapSampler : register(s2);

cbuffer Material : register(b3)
{
    float4 matDiffuseColor;
    float4 matSpecularColor;
    float matShininess;
    float matShininessStrength;
}




float3 CalculateDirectionalLights(float3 normal, float3 viewSpacePosition, float3 diffuseColor, float3 specularColor, float shininess)
{
    float3 totalDiffuse = { 0.0f, 0.0f, 0.0f };
    float3 totalSpecular = { 0.0f, 0.0f, 0.0f };
    
    float3 viewVec = normalize(-viewSpacePosition);
    
    for (uint i = 0; i < DirectionalLightCount && i < MAX_DIRECTIONAL_LIGHT_COUNT; i++)
    {
        float diffuse = max(dot(-directionalLights[i].direction, normal), 0.0f);
        if (diffuse < MINLIGHT)
            continue;
        
        float3 halfwayVec = normalize(-directionalLights[i].direction + viewVec);
        float specular = pow(max(dot(halfwayVec,normal), 0.0f), shininess);
        
        totalDiffuse += diffuse * directionalLights[i].ligth.rgb;
        totalSpecular += specular * directionalLights[i].ligth.rgb;
    }
            
    return totalDiffuse * diffuseColor + totalSpecular * specularColor * matShininessStrength;
}

float3 CalculatePointLights(float3 normal, float3 viewSpacePosition, float3 diffuseColor, float3 specularColor, float shininess)
{
    float3 totalDiffuse = { 0.0f, 0.0f, 0.0f };
    float3 totalSpecular = { 0.0f, 0.0f, 0.0f };
    
    float3 viewVec = normalize(-viewSpacePosition);
    
    for (uint i = 0; i < PointLightCount && i < MAX_POINT_LIGHT_COUNT;i++)
    {
        float3 lightDirection = viewSpacePosition - pointLights[i].position;
        float distance = length(lightDirection); 
        lightDirection /= distance;
        float diffuse = max(dot(-lightDirection, normal), 0.0f);
        if (diffuse < MINLIGHT)
            continue;
           
        float3 halfwayVec = normalize(-lightDirection + viewVec);
        float specular = pow(max(dot(halfwayVec, normal), 0.0f), shininess);
        float attenuation = 1.0f / (PointLight_Constant + PointLight_Linear * distance + PointLight_Quadratic * distance * distance);
        
        totalDiffuse += diffuse * pointLights[i].light.rgb * attenuation;
        totalSpecular += specular * pointLights[i].light.rgb * attenuation;
    }
    
    return totalDiffuse * diffuseColor + totalSpecular * specularColor * matShininessStrength;
}

float4 main(float3 ViewSpacePosition : VIEWSPACEPOSITION, 
            float3 ViewSpaceNormal : VIEWSPACENORMAL,
            float2 TexCoord : TEXCOORD, 
            float3 Tangent : TANGENT,
            float3 Bitangent : BITANGENT) : SV_Target
{
    float4 diffuseColor = matDiffuseColor * diffuseMap.Sample(diffuseMapSampler, TexCoord).rgba;
    
    float4 specularColor = specularMap.Sample(specularMapSampler, TexCoord);
    specularColor.rgb = specularColor.rgb * matSpecularColor.rgb;
    
    float shininess = pow(2.0f, 13.0f * matShininess * specularColor.a);
    
    ViewSpaceNormal = normalize(mul(normalize(normalMap.Sample(normalMapSampler, TexCoord).rgb * 2.0f - 1.0f),
                           float3x3(normalize(Tangent), normalize(Bitangent), normalize(ViewSpaceNormal))));
    
    //Lighting   
    float3 phong = AmbientLight.rgb * AmbientLight.a * diffuseColor.rgb +
               CalculateDirectionalLights(ViewSpaceNormal, ViewSpacePosition, diffuseColor.rgb, specularColor.rgb, shininess) +
               CalculatePointLights(ViewSpaceNormal, ViewSpacePosition, diffuseColor.rgb, specularColor.rgb, shininess);
    
    return saturate(float4(phong, diffuseColor.a));
}