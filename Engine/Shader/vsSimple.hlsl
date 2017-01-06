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

float4 main( float3 position : POSITION ) : SV_POSITION {
	return mul( viewProjMat, mul( worldMat, float4( position, 1 ) ) );
}