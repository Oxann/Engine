// VARIANTS BEGIN
// DIFFUSETEXTURE
// NORMALTEXTURE
// ROUGHNESSTEXTURE
// METALNESSTEXTURE
// AMBIENTOCCLUSIONTEXTURE
// SHADOW
// VARIANTS END

#include "TransformBuffers.hlsli"

struct OUT
{
    float3 WorldSpacePosition : WORLDSPACEPOSITION;
    float3 WorldSpaceNormal : WORLDSPACENORMAL;

#if defined(DIFFUSETEXTURE) || defined(ROUGHNESSTEXTURE) || defined(METALNESSTEXTURE) || defined(NORMALTEXTURE) || defined(AMBIENTOCCLUSIONTEXTURE)
    float2 TexCoord : TEXCOORD;
#endif
    
#if defined(NORMALTEXTURE)
    float3 WorldSpaceTangent : WORLDSPACETANGENT;
    float3 WorldSpaceBitangent : WORLDSPACEBITANGENT;
#endif    

#ifdef SHADOW
    float4 lightSpacePosition : LIGHTSPACEPOSITION;
#endif

    float4 Position : SV_POSITION;
};

struct IN
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    
#if defined(DIFFUSETEXTURE) || defined(ROUGHNESSTEXTURE) || defined(METALNESSTEXTURE) || defined(NORMALTEXTURE) || defined(AMBIENTOCCLUSIONTEXTURE)
    float2 TexCoord : TEXCOORD;
#endif 
    
#ifdef NORMALTEXTURE
    float3 WorldSpaceTangent : WORLDSPACETANGENT;
    float3 WorldSpaceBitangent : WORLDSPACEBITANGENT;
#endif   
};


OUT main( IN in_ )
{
    OUT Out;
    
    Out.Position = mul(float4(in_.position, 1.0f), modelViewProjection);
    Out.WorldSpacePosition = mul(float4(in_.position, 1.0f), model).xyz;
    Out.WorldSpaceNormal = normalize(mul(in_.normal, (float3x3) normalMatrix));

#if defined(DIFFUSETEXTURE) || defined(ROUGHNESSTEXTURE) || defined(METALNESSTEXTURE) || defined(NORMALTEXTURE) || defined(AMBIENTOCCLUSIONTEXTURE)
    Out.TexCoord = in_.TexCoord;
#endif 
    
#ifdef NORMALTEXTURE
    Out.WorldSpaceTangent = normalize(mul(in_.WorldSpaceTangent, (float3x3)normalMatrix));
    Out.WorldSpaceBitangent = normalize(mul(in_.WorldSpaceBitangent, (float3x3)normalMatrix));
#endif

#ifdef SHADOW
    Out.lightSpacePosition = mul(float4(in_.position, 1.0f), lightSpaceMatrix);


#endif
    return Out;
}