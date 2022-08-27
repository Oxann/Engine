// VARIANTS BEGIN
// VARIANTS END

#include "TransformBuffers.hlsli"

struct IN
{
	float3 position : POSITION;
};

struct OUT
{
	float3 texCoord : TEXCOORD;
	float4 position : SV_POSITION;
};

OUT main( IN in_)
{
	OUT Out;
	
	Out.texCoord = in_.position;
	Out.position = mul(float4(in_.position, 0.0f), viewProjectionMatrix);
	Out.position.z = Out.position.w;

	return Out;
}
