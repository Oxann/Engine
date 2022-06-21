// VARIANTS BEGIN
// DIFFUSETEXTURE
// SPECULARTEXTURE
// NORMALTEXTURE
// SHADOW
// VARIANTS END

#include "TransformBuffers.hlsli"

struct OUT
{
    float3 ViewSpacePosition : VIEWSPACEPOSITION;
    float3 ViewSpaceNormal : VIEWSPACENORMAL;

#if defined(DIFFUSETEXTURE) || defined(SPECULARTEXTURE) || defined(NORMALTEXTURE)
    float2 TexCoord : TEXCOORD;
#endif
    
#if defined(NORMALTEXTURE)
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
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
    
#if defined(DIFFUSETEXTURE) || defined(SPECULARTEXTURE) || defined(NORMALTEXTURE)
    float2 TexCoord : TEXCOORD;
#endif 
    
#ifdef NORMALTEXTURE
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
#endif   
};


OUT main( IN in_ )
{
    OUT Out;
    
    Out.Position = mul(float4(in_.position, 1.0f), modelViewProjection);
    Out.ViewSpacePosition = mul(float4(in_.position, 1.0f), modelView).xyz;
    Out.ViewSpaceNormal = normalize(mul(in_.normal, (float3x3) normalMatrix));

#if defined(DIFFUSETEXTURE) || defined(SPECULARTEXTURE) || defined(NORMALTEXTURE)
    Out.TexCoord = in_.TexCoord;
#endif 
    
#ifdef NORMALTEXTURE
    Out.Tangent = normalize(mul(in_.Tangent, (float3x3)normalMatrix));
    Out.Bitangent = normalize(mul(in_.Bitangent, (float3x3)normalMatrix));
#endif

#ifdef SHADOW
    Out.lightSpacePosition = mul(float4(in_.position, 1.0f), lightSpaceMatrix);
#endif
    return Out;
}