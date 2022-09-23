// VARIANTS BEGIN
// VARIANTS END

struct IN
{
	float3 position : POSITION;
	int4 BoneIDS : BONEIDS;
    float4 BoneWeights : BONEWEIGHTS;
};

struct OUT
{
	float4 position : SV_POSITION;
};

cbuffer Transform : register(b0)
{
	matrix boneMVPs[80];
};

OUT main(IN In)
{
	OUT Out;
    
	Out.position = float4(0.0f, 0.0f, 0.0f, 1.0f);

    for(int i = 0; i < 4; i++)
    {
        if(In.BoneIDS[i] < 0)
            break;
        
        Out.position.xyz += mul(float4(In.position, 1.0f), boneMVPs[In.BoneIDS[i]]).xyz * In.BoneWeights[i];
    }

	Out.position.w = 1.0f;
	
	if(Out.position.z < 0.0f)
		Out.position.z = 0.0f;

	return Out;
}