// VARIANTS BEGIN
// VARIANTS END

cbuffer Transform : register(b0)
{
	matrix viewProjection;
};

struct IN
{
	float3 position : POSITION;
};

struct OUT
{
	float3 localPosition : LOCALPOSITION;
	float4 position : SV_POSITION;
};

OUT main( IN in_)
{
	OUT Out;
	
	Out.localPosition = in_.position;
	Out.position = mul(float4(in_.position, 0.0f), viewProjection);
	Out.position.z = Out.position.w;

	return Out;
}