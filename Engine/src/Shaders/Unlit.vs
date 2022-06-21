// VARIANTS BEGIN
// COLORTEXTURE
// VARIANTS END

#include "TransformBuffers.hlsli"

struct IN
{
    float3 position : POSITION;

#ifdef COLORTEXTURE
    float2 texCoord : TEXCOORD;
#endif
};

struct OUT
{    
#ifdef COLORTEXTURE
    float2 TexCoord : TEXCOORD;
#endif
    
    float4 Position : SV_POSITION;
};


OUT main(IN in_)
{
    OUT Out;
    Out.Position = mul(float4(in_.position, 1.0f), modelViewProjection);
    
#ifdef COLORTEXTURE
    Out.TexCoord = in_.texCoord;
#endif
    
    return Out;
}