#include "TreeTraverse.hlsli"
#include "Random.hlsli"

Texture2D<float2> dsvTex : register( t0 );
Texture2D<float4> normalTex : register( t1 );

Texture2D<float> oldShadowTex : register( t5 );

RWTexture2D<float> newShadowTex: register( u1 );

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
	float JitterSep;
	float JitterRad;
	uint bUseNormal;
	uint bUseReprojection;
	uint bUseSmooth;
	uint bRenderShadow;
}

cbuffer LightBuffer : register( b2 ) {
	float4 LightColor;
	float3 InvLightDir;
	uint NumSamples;
	float fJitterRad;
	float fLightAngle;
}

#define JITTER

struct JitterData {
	float MinJitterRad;
	float MaxJitterRad;
	float MinJitterAngle;
	float MaxJitterAngle;
};

JitterData GetJitterData( uint idx, float jitterRad ) {
	const float pi = 3.1415926535f;
	JitterData jitterData = ( JitterData ) 0;

	float sep = jitterRad * JitterSep;

	switch( idx ) {
		case 0:
		{
			jitterData.MinJitterRad = 0.f;
			jitterData.MaxJitterRad = sep;
			jitterData.MinJitterAngle = 0.f;
			jitterData.MaxJitterAngle = 2.f * pi;
			break;
		}
		case 1:
		{
			jitterData.MinJitterRad = sep;
			jitterData.MaxJitterRad = jitterRad;
			jitterData.MinJitterAngle = -pi / 4.f;
			jitterData.MaxJitterAngle = pi / 4.f;
			break;
		}
		case 2:
		{
			jitterData.MinJitterRad = sep;
			jitterData.MaxJitterRad = jitterRad;
			jitterData.MinJitterAngle = pi / 4.f;
			jitterData.MaxJitterAngle = pi * 3.f / 4.f;
			break;
		}
		case 3:
		{
			jitterData.MinJitterRad = sep;
			jitterData.MaxJitterRad = jitterRad;
			jitterData.MinJitterAngle = pi * 3.f / 4.f;
			jitterData.MaxJitterAngle = pi * 5.f / 4.f;
			break;
		}
		case 4:
		{
			jitterData.MinJitterRad = sep;
			jitterData.MaxJitterRad = jitterRad;
			jitterData.MinJitterAngle = pi * 5.f / 4.f;
			jitterData.MaxJitterAngle = pi * 7.f / 4.f;
			break;
		}
		default:
			break;
	}
	return jitterData;
}

float GetShadowVal( float3 vPos, float3 vDir, float2 screenPos ) {

	float3 vInGridPos = ( vPos.xyz - vMinGridPos ) / vGridSize;
	vInGridPos += 2 * vDir / viGridResolution;

	float4 lastScreenPos = mul( mLastFrameMatrix, float4( vPos, 1 ) );
	lastScreenPos.y = -lastScreenPos.y;
	float2 lastTexPos = 0.5f * ( lastScreenPos.xy / lastScreenPos.w ) + 0.5f;

	if( all( vInGridPos > 0 && vInGridPos < 1 ) ) {
		float fShadowVal = 0.f;
#ifdef TEMPORAL
		NumberGenerator randGen;
		randGen.SetSeed( uint2( asuint( vPos.x ) ^ asuint( vPos.y ), asuint( vPos.z ) ^ FilterSeed ) );

		float3 vDir1;
		float3 vDir2;
		if( abs( vDir.x ) < 0.9 )
			vDir1 = normalize( cross( vDir, float3( 1, 0, 0 ) ) );
		else
			vDir1 = normalize( cross( vDir, float3( 0, 1, 0 ) ) );
		vDir2 = normalize( cross( vDir, vDir1 ) );

		JitterData jitterData = GetJitterData( uint( randGen.GetCurrentInt() ) % 5, JitterRad );

		float rad = randGen.GetRandomFloat( jitterData.MinJitterRad, jitterData.MaxJitterRad );
		float angle = randGen.GetRandomFloat( jitterData.MinJitterAngle, jitterData.MaxJitterAngle );

		float2 randVal;
		sincos( angle, randVal.y, randVal.x );
		randVal *= rad;

		vDir = vDir + vDir1 * randVal.x + vDir2 * randVal.y;

		float3 vTestColor;
		float newShadowVal;
		if( bUseNormal > 0 )
			newShadowVal = TraverseTree( vInGridPos, vDir, vTestColor ) ? 0.f : 1.f;
		else
			newShadowVal = ( 1 - SmoothTraverse2( vInGridPos, vDir, fLightAngle ) );

		float fOldShadowVal;
		if( bUseReprojection )
			fOldShadowVal = oldShadowTex.Sample( defaultSampler, lastTexPos );
		else
			fOldShadowVal = oldShadowTex[screenPos];
		float blendFactor = 0.05f;
		fShadowVal = blendFactor * newShadowVal + ( 1.f - blendFactor ) * fOldShadowVal;

		newShadowTex[screenPos] = fShadowVal;
#else
#ifdef SOFTSHADOW
		fShadowVal = ( 1 - SmoothTraverse1( vInGridPos, vDir, fLightAngle ) );
#else
#ifdef JITTER
		NumberGenerator randGen;
		randGen.SetSeed( uint2( asuint( vPos.x ) ^ asuint( vPos.y ), asuint( vPos.z ) ) );

		float3 vDir1;
		float3 vDir2;
		if( abs( vDir.x ) < 0.9 )
			vDir1 = normalize( cross( vDir, float3( 1, 0, 0 ) ) );
		else
			vDir1 = normalize( cross( vDir, float3( 0, 1, 0 ) ) );
		vDir2 = normalize( cross( vDir, vDir1 ) );
		float3 vInitialDir = vDir;

		// make room for maximum value
		float2 p[100];

		int iN = NumSamples;
		float fN = NumSamples;
		for( int j = 0; j < iN; ++j ) {
			for( int i = 0; i < iN; ++i ) {
				float randX = randGen.GetCurrentFloat();
				float randY = randGen.GetCurrentFloat();
				p[j * iN + i].x = ( i + ( j + randX ) / fN ) / fN;
				p[j * iN + i].y = ( j + ( i + randY ) / fN ) / fN;
			}
		}
		for( int j = 0; j < iN; ++j ) {
			float rand = randGen.GetCurrentFloat();
			int k = j + rand * ( iN - j );
			for( int i = 0; i < iN; ++i ) {
				float temp = p[j * iN + i].x;
				p[j * iN + i].x = p[k * iN + i].x;
				p[k * iN + i].x = temp;
			}
		}
		for( int i = 0; i < iN; ++i ) {
			float rand = randGen.GetCurrentFloat();
			int k = i + rand * ( iN - i );
			for( int j = 0; j < iN; ++j ) {
				float temp = p[j * iN + i].y;
				p[j * iN + i].y = p[j * iN + k].y;
				p[j * iN + k].y = temp;
			}
		}

		for( int i = 0; i < iN; ++i ) {
			for( int j = 0; j < iN; ++j ) {
				float randX = ( ( ( p[iN * j + i].x ) * 2.f ) - 1.f ) * fJitterRad;
				float randY = ( ( ( p[iN * j + i].y ) * 2.f ) - 1.f ) * fJitterRad;
				vDir = vInitialDir + vDir1 * randX + vDir2 * randY;
#endif
				float3 vTestColor;
				if( !TraverseTree( vInGridPos, vDir, vTestColor ) )
					fShadowVal += 1;
#ifdef JITTER
			}
		}

		fShadowVal /= fN * fN;
#endif
#endif
#endif
		return fShadowVal;
	}
	return 1.f;
}

void main( float2 texCoord : TEXCOORD, float4 svPos : SV_POSITION ) {
	if( !bRenderShadow ) {
		newShadowTex[svPos.xy] = 1.f;
		return;
	}

	float depth = dsvTex.Sample( defaultSampler, texCoord ).x;
	
	if( depth > 0.9999 ) {
		newShadowTex[svPos.xy] = 1.f;
		return;
	}
	
	float4 normal = normalTex.Sample( defaultSampler, texCoord );
	float fLightingVal = dot( normal.xyz, InvLightDir );
	if( fLightingVal < 0.f ) {
		newShadowTex[svPos.xy] = 0.f;
		return;
	}
	
	float4 vProjPosition = float4( texCoord, depth, 1.f );
	vProjPosition.xy = ( vProjPosition.xy * 2.f ) - 1.f;
	vProjPosition.y = -vProjPosition.y;
	float4 vPosition = mul( invViewProjMat, vProjPosition );
	vPosition.xyz /= vPosition.w;
	vPosition.w = 1.f;

	newShadowTex[svPos.xy] = GetShadowVal( vPosition.xyz, InvLightDir, svPos.xy );
}
