#include "Shader\VoxelDefines.hlsli"

Texture2D<float2> DepthTex : register( t0 );
Texture2D<float4> ColorTex : register( t1 );
Texture2D<float4> NormalTex : register( t2 );

RWTexture2D<float> ShadowTex : register( u1 );

SamplerState defaultSampler : register( s0 );

cbuffer CameraBuffer : register( b0 ) {
	float4x4 viewMat;
	float4x4 projMat;
	float4x4 viewProjMat;
	float4x4 invViewProjMat;
	float3 CameraPosition;
}

cbuffer FilterBuffer : register( b1 ) {
	float4x4 mLastFrameMatrix;
	uint FilterSeed;
	uint FilterIdx;
	float JitterRad;
	uint bUseNormal;
	uint bUseReprojection;
	uint bUseSmooth;
	uint bRenderShadow;
	float fRadiusMod;
	float fDepthThreshold;
}

cbuffer LightBuffer : register( b2 ) {
	float4 vLightColor;
	float3 vInvLightDir;
	uint uiNumSamples;
	float fJitterRad;
	float fLightAngle;
}

//#define MEDIAN

float4 main( float2 texCoord : TEXCOORD, float4 vPos : SV_POSITION ) : SV_TARGET
{
	float fDepth = DepthTex[vPos.xy].x;
	float4 vColor = saturate( ColorTex[vPos.xy] );	

	if( fDepth > 0.9999 )
		return vColor;

	float fSum = 0;
	float fWSum = 0;

	float4 vProjPosition = float4( texCoord, fDepth, 1.f );
	vProjPosition.xy = ( vProjPosition.xy * 2.f ) - 1.f;
	vProjPosition.y = -vProjPosition.y;
	float4 vPosition = mul( invViewProjMat, vProjPosition );
	vPosition.xyz /= vPosition.w;
	vPosition.w = 1.f;

	float fShadowVal = ShadowTex[vPos.xy];
#ifdef TEMPORAL

	if( bUseSmooth && bRenderShadow ) {
		float3 vCamDist = vPosition - CameraPosition;

		float fCamDist = sqrt( dot( vCamDist, vCamDist ) );
#ifdef MEDIAN
		const int iRadius = 2;
		const uint size = ( 2 * iRadius + 1 ) * ( 2 * iRadius + 1 );
		float Samples[size];

		uint idx = 0;
		for( float x = -iRadius; x <= iRadius; x++ ) {
			for( float y = -iRadius; y <= iRadius; y++ ) {
				int2 viSamplePoint = int2( vPos.xy ) + int2( x, y );
				Samples[idx++] = ShadowTex[viSamplePoint];
			}
		}
		uint n = size;
		do {
			uint newn = 1;
			for( uint i = 0; i < size - 1; ++i ) {
				if( Samples[i] > Samples[i + 1] ) {
					float temp = Samples[i];
					Samples[i] = Samples[i + 1];
					Samples[i + 1] = temp;
					newn = i + 1;
				}
			}
			n = newn;
		} while( n > 1 );

		ShadowTex[vPos.xy] = Samples[size / 2];

		fShadowVal = Samples[size / 2];
#else
		const int maxRad = 5;
		const int iRadius = clamp( ( fRadiusMod / fCamDist ) * maxRad, 1, maxRad );

		const float fSigma = clamp( ( fRadiusMod / fCamDist ) * maxRad, 1, 5 ) / 2.f;
		for( float x = -iRadius; x <= iRadius; x++ ) {
			for( float y = -iRadius; y <= iRadius; y++ ) {
				int2 viSamplePoint = int2( vPos.xy ) + int2( x, y );
				float fSampleDepth = DepthTex[viSamplePoint].x;

				float fDepthDiff = abs( fDepth - fSampleDepth );
				float fDist = sqrt( float( x*x + y*y ) );

				float gs = 1.0f / ( fSigma * sqrt( 2.0f * 3.14159f ) ) * exp( -fDist * fDist / ( 2.0f * fSigma * fSigma ) );
				float gr = 1.0f / ( fDepthThreshold * sqrt( 2.0f * 3.14159f ) ) * exp( -fDepthDiff * fDepthDiff / ( 2.0f * fDepthThreshold * fDepthThreshold ) );

				float w = gs * gr;
				fSum += ShadowTex[viSamplePoint] * w;
				fWSum += w;
			}
		}
		if( fWSum > 0.0 ) {
			fSum /= fWSum;
		}

		ShadowTex[vPos.xy] = fSum;
		fShadowVal = fSum;
#endif
	}
#endif
	float3 vNormal = NormalTex[vPos.xy].xyz;
	float fLightIntensity = saturate( dot( vNormal, vInvLightDir ) );
	return float4( vColor.xyz * fLightIntensity * fShadowVal, 1.f ) + 0.3;
}
