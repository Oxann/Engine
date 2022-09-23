// VARIANTS BEGIN
// DIFFUSETEXTURE
// NORMALTEXTURE
// ROUGHNESSTEXTURE
// METALNESSTEXTURE
// AMBIENTOCCLUSIONTEXTURE
// SHADOW
// SKINNING
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
    float3 Tangent : WORLDSPACETANGENT;
    float3 Bitangent : WORLDSPACEBITANGENT;
#endif

#ifdef SKINNING
    int4 BoneIDS : BONEIDS;
    float4 BoneWeights : BONEWEIGHTS;
#endif
};


OUT main( IN in_ )
{
    OUT Out;
    
#ifdef SKINNING
    Out.WorldSpacePosition = float3(0.0f,0.0f,0.0f);
    Out.WorldSpaceNormal = float3(0.0f,0.0f,0.0f);

    for(int i = 0; i < 4; i++)
    {
        if(in_.BoneIDS[i] < 0)
            break;
        
        Out.WorldSpacePosition += mul(float4(in_.position, 1.0f), bones[in_.BoneIDS[i]]).xyz * in_.BoneWeights[i];
        Out.WorldSpaceNormal += normalize(mul(in_.normal, (float3x3)bones[in_.BoneIDS[i]]).xyz) * in_.BoneWeights[i];

    #ifdef NORMALTEXTURE
        Out.WorldSpaceTangent = normalize(mul(in_.Tangent, (float3x3)bones[in_.BoneIDS[i]]).xyz) * in_.BoneWeights[i];
        Out.WorldSpaceBitangent = normalize(mul(in_.Bitangent, (float3x3)bones[in_.BoneIDS[i]]).xyz) * in_.BoneWeights[i];
    #endif
    }

    Out.Position = mul(float4(Out.WorldSpacePosition, 1.0f), viewProjectionMatrix);


#else
    Out.Position = mul(float4(in_.position, 1.0f), modelViewProjection);
    Out.WorldSpacePosition = mul(float4(in_.position, 1.0f), model).xyz;
    Out.WorldSpaceNormal = normalize(mul(in_.normal, (float3x3) normalMatrix));
    
    #ifdef NORMALTEXTURE
    Out.WorldSpaceTangent = normalize(mul(in_.Tangent, (float3x3)normalMatrix));
    Out.WorldSpaceBitangent = normalize(mul(in_.Bitangent, (float3x3)normalMatrix));
    #endif
#endif


#if defined(DIFFUSETEXTURE) || defined(ROUGHNESSTEXTURE) || defined(METALNESSTEXTURE) || defined(NORMALTEXTURE) || defined(AMBIENTOCCLUSIONTEXTURE)
    Out.TexCoord = in_.TexCoord;
#endif 
    


#ifdef SHADOW
    Out.lightSpacePosition = mul(float4(in_.position, 1.0f), lightSpaceMatrix);
#endif
    return Out;
}