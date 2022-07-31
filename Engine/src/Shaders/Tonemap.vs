// VARIANTS BEGIN
// VARIANTS END

struct IN
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
};

struct OUT
{
	float2 texCoord : TEXCOORD;
	float4 position : SV_POSITION;
};


OUT main( IN In )
{
	OUT Out;
	Out.position = float4(In.position * 2.0f,1.0f);
	Out.texCoord = In.texCoord;
	return Out;
}