#include "VoxelDefines.hlsli"

#include "InputStructs.hlsli"
#include "TreeTraverse.hlsli"
#include "Random.hlsli"

#define JITTER

Texture2D<float4> defaultTex : register( t0 );

SamplerState defaultSampler : register( s0 );

RWTexture2D<uint> filterTex : register( u1 );

cbuffer CameraBuffer : register( b0 ) {
	float4x4 viewMat;
	float4x4 projMat;
	float4x4 viewProjMat;
	float4x4 invViewProjMat;
	float3 CameraPosition;
}

cbuffer ObjectBuffer : register( b1 ) {
	float4x4 worldMat;
	float4 proberties;
	float4 color;
}

cbuffer LightBuffer : register( b2 ) {
	float4 LightColor;
	float3 InvLightDir;
	uint NumSamples;
	float fJitterRad;
}

cbuffer FilterBuffer : register( b3 ) {
	float2 ScreenSize;
	float2 RandVal;
	uint FilterIdx;
}

struct PS_Out {
	float4 Color : SV_Target0;
	float3 Normal : SV_Target1;
};

PS_Out main( PInput pIn )
{
	PS_Out psOut = (PS_Out)0;
	psOut.Color = defaultTex.Sample( defaultSampler, pIn.TexCoord );
	psOut.Normal = saturate( pIn.Normal );

	return psOut;
}