#include "VoxelDefines.hlsli"
#include "TreeTraverse.hlsli"


cbuffer CameraData : register( b0 ) {
	float3 vStartPos;
	float3 vLookDir;
	float3 vLookUp;
	float3 vLookRight;
	float2 fMaxScreenVal;
	float2 vCamSize;
};

cbuffer LightBuffer : register( b2 ) {
	float4 LightColor;
	float3 InvLightDir;
	uint NumSamples;
	float fJitterRad;
	float fLightAngle;
}

float4 main( float2 texCoord : TEXCOORD ) : SV_TARGET{
	//if( Grid[uint3( texCoord * 64, 64 )] )
		//return float4( 1, 0, 0, 1 );
	//return float4( 0,1,0,1 );

	float3 vDir = vLookDir;
#ifdef PERSPECTIVE_VIEW
	vDir += fMaxScreenVal.x * ( texCoord.x * 2.0f - 1.0f ) * vLookRight + fMaxScreenVal.y * ( -texCoord.y * 2.0 + 1.0f ) * vLookUp;
#endif
	float2 camSize = vCamSize;
	float3 vPos = vStartPos;
#ifndef PERSPECTIVE_VIEW
	vPos += camSize.x * ( texCoord.x * 2.f - 1.f ) * vLookRight + camSize.y * ( ( 1 - texCoord.y ) * 2.f - 1.f ) * vLookUp;
#endif
	uint3 vDirNeg = vDir < 0;
	float3 vMirror = vDirNeg ? -1.f : 1.f;

	float3 vBoxPos = vMinGridPos + 0.5f * vGridSize;
	float3 vHExtent = 0.5f * vGridSize;

	float3 vNearBound = vBoxPos - ( vHExtent * vMirror );
	float3 vFarBound = vBoxPos + ( vHExtent * vMirror );

	float3 vDir_1 = 1.f / vDir;

	// calculate box entry with ray box test
	float3 vNearTs = ( vNearBound - vPos ) * vDir_1;
	float3 vFarTs = ( vFarBound - vPos ) * vDir_1;

	float fEntryT = max( vNearTs.x, max( vNearTs.y, vNearTs.z ) );
	float fExitT = min( vFarTs.x, min( vFarTs.y, vFarTs.z ) );

	// Grid not hit
	if( fExitT < fEntryT || fExitT < 0.f )
		return float4( .3f, .3f, 1.f, 1.f );

	// update pos if startPos is outside
	if( fEntryT > 0.f )
		vPos = vPos + vDir * fEntryT;

	// transfer global pos to gridPos
	float3 vGridSize = float3( viGridResolution );
	float3 vBoxMin = vBoxPos - vHExtent;
	vPos = saturate( ( vPos - vBoxMin ) / ( 2.f * vHExtent ) );
	
#ifdef SOFTSHADOW
	float val = SmoothTraverse1( vPos, vDir, fLightAngle );
	if( val > 0.f )
		return float4( val, 0.f, 0.f, 1.f );
	return float4( .3f, .3f, 1.f, 1.f );
#else
	float3 color;
	if( TraverseTree( vPos, vDir, color ) )
		return float4( color, 1.f );
	return float4( .3f, .3f, 1.f, 1.f );
#endif
}