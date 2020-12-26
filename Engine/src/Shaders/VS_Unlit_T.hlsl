struct OUT
{
    float2 TexCoord : TEXCOORD;
    float4 Position : SV_POSITION;
};

cbuffer Transform : register(b0)
{
    matrix MVP;
};


OUT main(float3 position : POSITION, float2 texCoord : TEXCOORD)
{
    OUT Out;
    Out.Position = mul(float4(position, 1.0f), MVP);
    Out.TexCoord = texCoord;
    return Out;
}