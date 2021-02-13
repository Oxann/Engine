struct OUT
{
    float4 Position : SV_POSITION;

#ifdef TEXTURE
        float2 TexCoord : TEXCOORD;
#endif
};

cbuffer Transform : register(b0)
{
    matrix MVP;
};

#ifdef TEXTURE
OUT main(float3 position : POSITION, float2 texCoord : TEXCOORD)
#else
OUT main(float3 position : POSITION)
#endif
{
    OUT Out;
    Out.Position = mul(float4(position, 1.0f), MVP);
    
#ifdef TEXTURE
    Out.TexCoord = texCoord;
#endif
    
    return Out;
}