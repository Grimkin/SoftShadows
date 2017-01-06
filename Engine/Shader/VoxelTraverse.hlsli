#include "VoxelDefines.hlsli"

#ifdef ADVANCED_TRAVERSE
struct Element {
	uint3 Value;
	uint Pointer;
};
StructuredBuffer<Element> Tree : register( t1 );
#else
Buffer<uint> Tree : register( t1 );
#endif
StructuredBuffer<uint2> Bricks : register( t2 );

cbuffer GridData : register( b4 ) {
	uint3 viGridResolution;
	uint uiTreeSize;
	float3 vMinGridPos;
	float3 vGridSize;
};

uint GetBrick( uint3 viPos, uint3 viLastPos, inout uint uiDeltaOffset, uint uiLevel, out uint uiSize, out uint3 viValue ) {
	viValue = uint3( 0, 0, 0 );
	uiSize = 1 << uiLevel;
	// highest differing bit decides the delta to be tested
	uint3 viDifference = viPos ^ viLastPos;
	uint uiDiff = viDifference.x | viDifference.y | viDifference.z;

	// test what the highest differing bit is
	int iHighestDifference = min( uiTreeSize - 1, firstbithigh( uiDiff ) );

	// remove all deltas that aren't needed anymore
	uiDeltaOffset >>= 3 * ( iHighestDifference + 1 );
	// calculate offset for finest level
	uint uiOffset = ( 8.0f / 7.0f ) * ( ( 1 << ( ( uiTreeSize - uiLevel - 1 ) * 3 ) ) - 1 );
	uiDeltaOffset = 0;
	// calculate the new delta
	uint uiStart = 1;// int( uiTreeSize ) - iHighestDifference;
	for( uint i = uiStart; i <= uiTreeSize - uiLevel; ++i ) {
		uint uiDeltaPos = 0;
		uiDeltaPos += ( viPos.x >> ( uiTreeSize - i ) ) & 1;
		uiDeltaPos += 2 * ( ( viPos.y >> ( uiTreeSize - i ) ) & 1 );
		uiDeltaPos += 4 * ( ( viPos.z >> ( uiTreeSize - i ) ) & 1 );

		uiDeltaOffset <<= 3;
		uiDeltaOffset += uiDeltaPos;
	}

	// test if brick exists
#ifdef ADVANCED_TRAVERSE
	uint uiBrick = Tree[uiOffset + uiDeltaOffset].Pointer;
	viValue = Tree[uiOffset + uiDeltaOffset].Value;
#else
	uint uiBrick = Tree[uiOffset + uiDeltaOffset];
#endif
	if( uiBrick != -1 ) {
		return uiBrick;
	}

	uint uiTempDelta = uiDeltaOffset;
	// check for finest not empty level
	for( uint k = 0; k < uiTreeSize; ++k ) {
		uiOffset -= 1 << ( 3 * ( uiTreeSize - k - 1 ) );
		uiTempDelta >>= 3;

#ifdef ADVANCED_TRAVERSE
		if( Tree[uiOffset + uiTempDelta].Pointer != -1 ) {
#else
		if( Tree[uiOffset + uiTempDelta] != -1 ) {
#endif
			uiSize = 1 << k;
			break;
		}
	}

	// indicate that no brick was hit
	return -1;
}



bool CheckBrick( uint uiBrickIdx, float3 vPos, float3 vDir, int3 viMirror, int3 viOffset ) {
	uint2 uiBrick = Bricks[uiBrickIdx];
	for( uint i = 0; i < 10; ++i ) {
		int3 viPos = viOffset + viMirror * int3( vPos );
		uint uiBitIdx = viPos.x + 4 * viPos.y + 16 * viPos.z;
		uint uiBitDelta = uiBitIdx & 0x1F;
		uint uiBit = 0;
		if( uiBitIdx < 32 )
			uiBit = 1 & ( uiBrick.x >> uiBitDelta );
		else
			uiBit = 1 & ( uiBrick.y >> uiBitDelta );

		if( uiBit )
			return true;

		vPos += vDir;

		if( any( vPos >= 4 ) )
			break;
	}
	return false;
}

bool TraverseGrid( float3 vPos, float3 vDir, out float3 color ) {
	color = 0;

	uint3 vDirNeg = vDir < 0;
	float3 vMirror = vDirNeg ? -1.f : 1.f;
	int3 viMirror = vMirror;
	int3 viBrickOffset = vDirNeg ? 3 : 0;
	float3 vOffset = vDirNeg ? viGridResolution - ( 1.f / 1024.f ) : 0.f;

	vPos = vDirNeg ? 1 - vPos : vPos;
	vPos *= viGridResolution;

	float fMaxDir = max( abs( vDir.x ), max( abs( vDir.y ), abs( vDir.z ) ) );
	vDir /= fMaxDir;
	float3 vDir_1 = 1.f / vDir;

	vDir = abs( vDir );
	vDir_1 = abs( vDir_1 );


	uint lastOffset = 0;
	uint uiSize = 1;
	uint3 viOldSamplePos = -1;
	uint i;
	for( i = 0; i < 200; ++i ) {
		uint3 viNewSamplePos = vOffset + vMirror * vPos;
		uint3 viValue;
		uint uiBrick = GetBrick( viNewSamplePos, viOldSamplePos, lastOffset,0, uiSize, viValue );
#ifndef ADVANCED_TRAVERSE
		if( uiBrick != -1 ) {
			float3 vInBrickPos = ( vPos - floor( vPos ) ) * 4.f;
			if( CheckBrick( uiBrick, vInBrickPos, vDir, viMirror, viBrickOffset ) ) {
				float val = i / 20.0f;
				color = float3( val, 0, 0 );
				return true;
			}
		}
#endif
		viOldSamplePos = viNewSamplePos;

		float fSize = uiSize;
		float3 vEdges = ( floor( vPos / fSize ) + float3( 1, 1, 1 ) ) * fSize;
		float3 t = ( vEdges - vPos ) * vDir_1;
		float fTMin = min( t.x, min( t.y, t.z ) );

		vPos += vDir * fTMin;
		uint uiTestVal = 0;
		if( fTMin == t.x ) {
			vPos.x = vEdges.x;
			uiTestVal = viValue.x;

		}
		else if( fTMin == t.y ) {
			vPos.y = vEdges.y;
			uiTestVal = viValue.y;
		}
		else {
			vPos.z = vEdges.z;
			uiTestVal = viValue.z;
		}

#ifdef ADVANCED_TRAVERSE
		if( uiBrick != -1 ) {
			float3 vValue = countbits(viValue) / ( fSize * fSize * 16 );
			vValue *= fTMin / fSize;
			color += vValue.x + vValue.y + vValue.z;
			if( color.x > 0.9f ) {
				color = 1;
				return true;
			}
		}
#endif

		// test if still in box
		if( any( vPos >= int3( viGridResolution ) ) )
			break;
	}
	//color = float3( 0, i / 20.0f, 0 );
	return false;
}
#ifdef SMOOTH_TRAVERSE
float GetBitVal( float3 vPos, float3 vDir, uint uiLevel ) {
	uint3 viPos = vPos / 4;

	uint uiPointer = 0;
	uint uiMaxTreeLevel = uiLevel > 2 ? uiTreeSize + 2 - uiLevel : uiTreeSize;

	uint3 viValue = 0;
	// calculate the new delta
	for( uint i = 1; i <= uiMaxTreeLevel; ++i ) {
		uint uiDeltaPos = 0;
		uiDeltaPos += ( viPos.x >> ( uiTreeSize - i ) ) & 1;
		uiDeltaPos += 2 * ( ( viPos.y >> ( uiTreeSize - i ) ) & 1 );
		uiDeltaPos += 4 * ( ( viPos.z >> ( uiTreeSize - i ) ) & 1 );
		
		viValue = Tree[uiPointer + uiDeltaPos].Value;
		uiPointer = Tree[uiPointer + uiDeltaPos].Pointer;
		if( uiPointer == -1 )
			return 0.f;
	}
	if( uiLevel > 1 ) {
		float fSize = float( 1 << ( uiLevel - 2 ) );
		float3 vVal = ( viValue ) / ( fSize * fSize * 16 );
		vVal *= abs( vDir );
		return max( vVal.x, max( vVal.y, vVal.z ) );
	}

	uint2 uiBrick = Bricks[uiPointer];
	if( uiLevel == 0 ) {
		viPos = vPos - (viPos << 2);

		uint uiBitIdx = viPos.x + 4 * viPos.y + 16 * viPos.z;
		uint uiBitDelta = uiBitIdx & 0x1F;
		uint uiBit = 0;
		if( uiBitIdx < 32 )
			uiBit = 1 & ( uiBrick.x >> uiBitDelta );
		else
			uiBit = 1 & ( uiBrick.y >> uiBitDelta );

		if( uiBit )
			return 1.f;
	}
	else {
		viPos = vPos - ( viPos << 1 );

		uint uiBitDelta = 2 * viPos.x + 8 * viPos.y;
		uint uiBit = 0;
		if( viPos.z < 1 )
			uiBit = 0x330033 & ( uiBrick.x >> uiBitDelta );
		else
			uiBit = 0x330033 & ( uiBrick.y >> uiBitDelta );

		if( uiBit )
			return 1.f;
	}
	return 0.f;
}

float GetVal( float3 vPos, float3 vDir, uint uiLevel, float fSize ) {
	// bilinear interpolate between the eight surrounding values	
	float fVal0 = GetBitVal( vPos, vDir, uiLevel );
	float fVal1 = GetBitVal( vPos + float3( fSize, 0, 0 ), vDir, uiLevel );
	float fVal2 = GetBitVal( vPos + float3( 0, fSize, 0 ), vDir, uiLevel );
	float fVal3 = GetBitVal( vPos + float3( fSize, fSize, 0 ), vDir, uiLevel );
	float fVal4 = GetBitVal( vPos + float3( 0, 0, fSize ), vDir, uiLevel );
	float fVal5 = GetBitVal( vPos + float3( fSize, 0, fSize ), vDir, uiLevel );
	float fVal6 = GetBitVal( vPos + float3( 0, fSize, fSize ), vDir, uiLevel );
	float fVal7 = GetBitVal( vPos + float3( fSize, fSize, fSize ), vDir, uiLevel );

	float3 vMinBound = floor( vPos / fSize ) * fSize;
	float3 vCoord = ( vPos - vMinBound ) / fSize;

	float fVal01 = lerp( fVal0, fVal1, vCoord.x );
	float fVal23 = lerp( fVal2, fVal3, vCoord.x );
	float fVal0123 = lerp( fVal01, fVal23, vCoord.y );

	float fVal45 = lerp( fVal4, fVal5, vCoord.x );
	float fVal67 = lerp( fVal6, fVal7, vCoord.x );
	float fVal4567 = lerp( fVal45, fVal67, vCoord.y );

	return lerp( fVal0123, fVal4567, vCoord.z );
}

float SmoothTraverseGrid( float3 vPos, float3 vDir ) {
	float3 vUpperBound = viGridResolution << 2;
	vPos *= vUpperBound;

	float fOutVal = 0;
	uint uiLevel = 3;
	float fSize = 1 << uiLevel;

	float maxDir = max( abs( vDir.x ), max( abs( vDir.y ), abs( vDir.z ) ) );
	vDir /= maxDir;

	for( uint i = 0; i < 200; ++i ) {
		fOutVal += GetVal( vPos, vDir, uiLevel, fSize );

		if( fOutVal >= 0.9f ) {
			fOutVal = 1.f;
			break;
		}

		vPos += vDir * 0.5* fSize;

		// test if still in box
		if( any( vPos >= vUpperBound ) || any( vPos < 0 ) )
			break;
	}

	return saturate( fOutVal );
}
#endif