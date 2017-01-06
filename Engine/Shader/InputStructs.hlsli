struct VInput {
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD0;
};

struct PInput {
	float4 Position : SV_POSITION;
	float4 WorldPosition : POSITION0;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD0;
};
