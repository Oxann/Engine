//Material 

Texture2D colorTexture : register(t0);
SamplerState colorTextureSampler : register(s0);


cbuffer Material : register(b3)
{
    float4 color;
}

float4 main(float2 texCoord : TEXCOORD) : SV_TARGET
{
    return colorTexture.Sample(colorTextureSampler, texCoord);
}