//Material 

#ifdef TEXTURE

Texture2D colorTexture : register(t0);
SamplerState colorTextureSampler : register(s0);

#endif

cbuffer Material : register(b3)
{
    float4 color;
}

#ifdef TEXTURE
float4 main(float2 texCoord : TEXCOORD) : SV_TARGET
#else
float4 main() : SV_TARGET
#endif
{
#ifdef TEXTURE
    return colorTexture.Sample(colorTextureSampler, texCoord) * color;
#else
    return color;
#endif
}