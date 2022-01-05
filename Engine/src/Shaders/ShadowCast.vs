// VARIANTS BEGIN
// VARIANTS END

struct IN
{
	float3 position : POSITION;
};

struct OUT
{
	float4 position : SV_POSITION;
};

cbuffer Transform : register(b0)
{
	matrix MVP;
};

OUT main(IN In)
{
	OUT Out;
    Out.position = mul(float4(In.position, 1.0f), MVP);
	return Out;
}
