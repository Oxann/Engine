//Phong vertex shader with texture mapping.

struct OUT
{
    float3 ViewSpacePosition : VIEWSPACEPOSITION;
    float3 ViewSpaceNormal : VIEWSPACENORMAL;
    float2 TexCoord : TEXCOORD;
    float4 Position : SV_POSITION;
};

cbuffer Transform : register(b0)
{
    matrix modelView;
    matrix normalMatrix;
    matrix MVP;
};


OUT main(float3 position : POSITION, float3 normal : NORMAL, float2 texCoord : TEXCOORD)
{
    OUT Out;
    Out.Position = mul(float4(position, 1.0f), MVP);
    Out.ViewSpacePosition = mul(float4(position, 1.0f), modelView).xyz;
    Out.ViewSpaceNormal = normalize(mul(normal, (float3x3) modelView));
    Out.TexCoord = texCoord;
    return Out;
}