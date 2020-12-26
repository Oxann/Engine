struct OUT
{
    float4 Position : SV_POSITION;
};

cbuffer Transform : register(b0)
{
    matrix MVP;
};


OUT main(float3 position : POSITION) : SV_POSITION
{
    OUT Out;
    Out.Position = mul(float4(position, 1.0f), MVP);   
	return Out;
}