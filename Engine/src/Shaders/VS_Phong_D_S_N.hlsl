struct OUT
{
    float3 ViewSpacePosition : VIEWSPACEPOSITION;
    float3 ViewSpaceNormal : VIEWSPACENORMAL;
    float2 TexCoord : TEXCOORD;
    float3 Tangent : TANGENT;
    float3 Bitangent : BITANGENT;
	float4 Position : SV_POSITION;
};

cbuffer Transform : register(b0)
{
    matrix modelView;
    matrix normalMatrix;
    matrix MVP;
};

OUT main(float3 pos : POSITION,
         float2 texCoord : TEXCOORD, 
         float3 normal : NORMAL, 
         float3 tangent : TANGENT, 
         float3 bitangent : BITANGENT)
{  
    OUT Out; 
    Out.ViewSpacePosition = (float3) mul(float4(pos, 1.0f), modelView);
    Out.Position = mul(float4(pos,1.0f), MVP);
    Out.TexCoord = texCoord;
    Out.ViewSpaceNormal = normalize(mul(normal, (float3x3) normalMatrix));
    Out.Tangent = normalize(mul(tangent, (float3x3) normalMatrix));
    Out.Bitangent = normalize(mul(bitangent, (float3x3) normalMatrix));
    return Out;
}