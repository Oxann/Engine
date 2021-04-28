// VARIANTS BEGIN
// COLORTEXTURE
// VARIANTS END


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

cbuffer Transform : register(b0)
{
    matrix MVP;
};

OUT main(IN in_)
{
    OUT Out;
    Out.Position = mul(float4(in_.position, 1.0f), MVP);
    
#ifdef COLORTEXTURE
    Out.TexCoord = in_.texCoord;
#endif
    
    return Out;
}