#include "InputStructs.hlsli"

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

PInput main( VInput vIn ) {
	PInput vOut;
	vOut.WorldPosition = mul( worldMat, float4( vIn.Position, 1 ) );
	vOut.Position = mul( viewProjMat, vOut.WorldPosition );
	vOut.Normal = normalize( mul( worldMat, float4(vIn.Normal,0) ).xyz );
	vOut.TexCoord = vIn.TexCoord;
	
	return vOut;
}