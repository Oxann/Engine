//Material 

#ifdef TEXTURE

Texture2D colorTexture : register(t0);
SamplerState colorTextureSampler : register(s0);

#endif

cbuffer Material : register(b3)
{
    float4 color;
}

float4 main(float2 texCoord : TEXCOORD) : SV_TARGET
{
#ifdef TEXTURE
    return colorTexture.Sample(colorTextureSampler, texCoord) * color;
#else
    return color;
#endif
}