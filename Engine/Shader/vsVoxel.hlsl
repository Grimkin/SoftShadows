struct VInput {
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD;
};

struct GInput {
	float3 Position : POSITION;
	float3 Normal : NORMAL;
};

cbuffer CameraBuffer : register( b0 ) {
	float4x4 viewMat;
	float4x4 projMat;
	float4x4 viewProjMat;
	float4x4 invViewProjMat;
}

cbuffer ObjectBuffer : register( b1 ) {
	float4x4 worldMat;
	float4 proberties;
	float4 color;
}

GInput main( VInput vIn ) {
	GInput vsOut = (GInput)0;
	vsOut.Position = vIn.Position;
	vsOut.Normal = vIn.Normal;
	return vsOut;
}