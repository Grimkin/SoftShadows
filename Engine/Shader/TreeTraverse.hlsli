#include "VoxelDefines.hlsli"

#define ACCURATE

cbuffer GridData : register( b4 ) {
	uint3 viGridResolution;
	uint uiTreeSize;
	float3 vMinGridPos;
	float fMaxItVal;
	float3 vGridSize;
};

struct Node {
	uint2 Data;
	uint Pointer;
};

StructuredBuffer<Node> Tree : register( t2 );
Buffer<uint> Pointers : register( t3 );
#ifdef ANISOTROPIC
Buffer<float3> Approx : register( t4 );
#else
Buffer<float> Approx : register( t4 );
#endif

uint Encode( uint3 pos ) {
	// Generate Morten Code
	pos &= 0x3ff;
	pos = ( pos | ( pos << 16 ) ) & 0x030000ff;
	pos = ( pos | ( pos << 8 ) ) & 0x0300f00f;
	pos = ( pos | ( pos << 4 ) ) & 0x030c30c3;
	pos = ( pos | ( pos << 2 ) ) & 0x09249249;

	return pos.x | ( pos.y << 1 ) | ( pos.z << 2 );
}

bool CheckBrick( uint uiBrickIdx, float3 vPos, float3 vDir, float3 vDir_1, int3 viMirror, int3 viOffset ) {
	uint2 uiBrick = Tree[uiBrickIdx].Data;
	for( uint i = 0; i < 100; ++i ) {
#ifdef ACCURATE
		uint3 viPos = viOffset + viMirror * int3( vPos );
		uint uiBitIdx = Encode( viPos );
		uint uiBitDelta = uiBitIdx & 0x1F;
		uint uiBit = 0;
		if( uiBitIdx < 32 )
			uiBit = 1 & ( uiBrick.x >> uiBitDelta );
		else
			uiBit = 1 & ( uiBrick.y >> uiBitDelta );

		if( uiBit )
			return true;

		float3 vEdges = floor( vPos ) + float3( 1, 1, 1 );
		float3 t = ( vEdges - vPos ) * vDir_1;
		float fTMin = min( t.x, min( t.y, t.z ) );

		vPos += vDir * fTMin;

		if( fTMin == t.x ) {
			vPos.x = vEdges.x;
		}
		else if( fTMin == t.y ) {
			vPos.y = vEdges.y;
		}
		else {
			vPos.z = vEdges.z;
		}
#else
		int3 viPos = viOffset + viMirror * int3( vPos );
		uint uiBitIdx = Encode( viPos );
		uint uiBitDelta = uiBitIdx & 0x1F;
		uint uiBit = 0;
		if( uiBitIdx < 32 )
			uiBit = 1 & ( uiBrick.x >> uiBitDelta );
		else
			uiBit = 1 & ( uiBrick.y >> uiBitDelta );

		if( uiBit )
			return true;

		vPos += vDir;
#endif
		if( any( vPos >= 4 ) )
			break;
	}
	return false;
}

uint Traverse( uint3 viPos, uint uiPointer, uint minLevel, uint maxLevel, out uint uiSize ) {
	uint uiPackedPos = Encode( viPos );
	uiSize = 1;
	for( uint level = minLevel; level <= maxLevel; level++ ) {
		Node current = Tree[uiPointer];

		uint uiNodePos = ( uiPackedPos >> ( ( maxLevel - level ) * 6 ) ) & 0x3f;
		uint uiData = uiNodePos < 32 ? current.Data.x : current.Data.y;
		uint uiDataPos = uiNodePos < 32 ? uiNodePos : uiNodePos - 32;

		if( uiData & ( 1 << uiDataPos ) ) {
			uint uiCompVal = 0x7fffffff >> ( 31 - uiDataPos );
			uint offset = countbits( uint( uiData & uiCompVal ) );
			if( uiNodePos > 31 ) {
				offset += countbits( current.Data.x );
			}
			uiPointer = Pointers[current.Pointer + offset];
			if( uiPointer == -1 )
				return 0;
		}
		else {
			uiSize = 1 << ( 2 * ( uiTreeSize - level ) );
			return -1;
		}
	}
	return uiPointer;
}

uint GetBrick( uint3 viPos, out uint uiSize ) {
	uiSize = 1;

	uint uiPointer = Pointers[0];

	if( uiTreeSize > 5 ) {
		uiPointer = Traverse( viPos >> 10, uiPointer, 1, uiTreeSize - 5, uiSize );
		if( uiPointer != -1 ) {
			return Traverse( viPos, uiPointer, uiTreeSize - 4, uiTreeSize, uiSize );
		}
		return -1;
	}
	else {
		return Traverse( viPos, uiPointer, 1, uiTreeSize, uiSize );
	}
}

uint Traverse1( uint3 viPos, uint uiPointer, uint minLevel, uint maxLevel, inout uint uiLevel ) {
	uint uiPackedPos = Encode( viPos );
	for( uint level = minLevel; level <= maxLevel; level++ ) {
		Node current = Tree[uiPointer];

		uint uiNodePos = ( uiPackedPos >> ( ( maxLevel - level ) * 6 ) ) & 0x3f;
		uint uiData = uiNodePos < 32 ? current.Data.x : current.Data.y;
		uint uiDataPos = uiNodePos < 32 ? uiNodePos : uiNodePos - 32;

		if( uiData & ( 1 << uiDataPos ) ) {
			uint uiCompVal = 0x7fffffff >> ( 31 - uiDataPos );
			uint offset = countbits( uint( uiData & uiCompVal ) );
			if( uiNodePos > 31 ) {
				offset += countbits( current.Data.x );
			}
			uiPointer = Pointers[current.Pointer + offset];
			if( uiPointer == -1 )
				return 0;
		}
		else {
			uiLevel = uiTreeSize - level;
			return -1;
		}
	}
	return uiPointer;
}

uint GetBrick1( uint3 viPos, inout uint uiLevel ) {
	uint uiPointer = Pointers[0];

	if( uiTreeSize > 5 ) {
		uiPointer = Traverse1( viPos >> 10, uiPointer, 1, uiTreeSize - 5, uiLevel );
		if( uiPointer != -1 ) {
			return Traverse1( viPos >> ( 2 * uiLevel ), uiPointer, uiTreeSize - 4, uiTreeSize - uiLevel, uiLevel );
		}
		return -1;
	}
	else {
		return Traverse1( viPos >> ( 2 * uiLevel ), uiPointer, 1, uiTreeSize - uiLevel, uiLevel );
	}
}

float GetApproxVal( uint3 viPos, float3 vDir, uint uiLevel ) {
	uint uiOutSize = 1;

	uint uiPointer = Pointers[0];

	if( uiTreeSize > 5 ) {
		uiPointer = Traverse( viPos >> 10, uiPointer, 1, uiTreeSize - 5, uiOutSize );
		if( uiPointer != -1 && uiPointer != 0 ) {
			uiPointer = Traverse( viPos, uiPointer, uiTreeSize - 4, uiTreeSize - uiLevel, uiOutSize );
		}
	}
	else {
		uiPointer = Traverse( viPos, uiPointer, 1, uiTreeSize - uiLevel, uiOutSize );
	}
	if( uiPointer == 0 )
		return 1.f;
#ifdef ANISOTROPIC
	else if( uiPointer != -1 ) {
		float3 approx = Approx[uiPointer];
		float value = 0;
		value += vDir.x * approx.x;
		value += vDir.y * approx.y;
		value += vDir.z * approx.z;
		return saturate( value / ( vDir.x + vDir.y + vDir.z ) );
	}
#else
	else if( uiPointer != -1 )
		return Approx[uiPointer];
#endif
	return 0.f;
}

float GetValue( float3 vPos, float3 vDir, uint uiLevel ) {
	uint3 viPos = vPos / ( 1 << ( 2 * uiLevel ) ) - 0.5f;

	// bilinear interpolate between the eight surrounding values	
	float fVal0 = GetApproxVal( viPos, vDir, uiLevel );
	float fVal1 = GetApproxVal( viPos + uint3( 1, 0, 0 ), vDir, uiLevel );
	float fVal2 = GetApproxVal( viPos + uint3( 0, 1, 0 ), vDir, uiLevel );
	float fVal3 = GetApproxVal( viPos + uint3( 1, 1, 0 ), vDir, uiLevel );
	float fVal4 = GetApproxVal( viPos + uint3( 0, 0, 1 ), vDir, uiLevel );
	float fVal5 = GetApproxVal( viPos + uint3( 1, 0, 1 ), vDir, uiLevel );
	float fVal6 = GetApproxVal( viPos + uint3( 0, 1, 1 ), vDir, uiLevel );
	float fVal7 = GetApproxVal( viPos + uint3( 1, 1, 1 ), vDir, uiLevel );

	float3 vMinBound = float3(viPos) + 0.5f;
	float3 vCoord = ( vPos / ( 1 << ( 2 * ( uiLevel ) ) ) - vMinBound );

	float fVal01 = lerp( fVal0, fVal1, vCoord.x );
	float fVal23 = lerp( fVal2, fVal3, vCoord.x );
	float fVal0123 = lerp( fVal01, fVal23, vCoord.y );

	float fVal45 = lerp( fVal4, fVal5, vCoord.x );
	float fVal67 = lerp( fVal6, fVal7, vCoord.x );
	float fVal4567 = lerp( fVal45, fVal67, vCoord.y );

	return lerp( fVal0123, fVal4567, vCoord.z );
}

float GetValue1( float3 vPos, float3 vInBrickPos, float3 vDir, float3 vDir_1, int3 viMirror, int3 viBrickOffset, float fLevel, out float fDelta, inout int iLevel ) {
	fDelta = 1;
	float fLastVal = 0.f;
	while( iLevel > fLevel - 1 ) {
		fLastVal = GetValue( vPos, vDir, iLevel );
		if( fLastVal < 0.00001f ) {
			fDelta = 1 << ( 2 * ( iLevel - 1 ) );
			return 0.f;
		}
		iLevel--;
	}
	float fNewVal = 0.f;
	/*if( iLevel < 0 ) {
		uint uiSize;
		uint uiBrick = GetBrick( uint3( vPos ), uiSize );
		if( uiBrick == 0 ) {
			fNewVal = 1.f;
		}
		else if( uiBrick != -1 ) {
			fNewVal = 1.f;
			if( CheckBrick( uiBrick, vInBrickPos, vDir, vDir_1, viMirror, viBrickOffset ) )
				fNewVal = 1.f;
			else
				fNewVal = 0.f;
		}
	}
	else {*/
		fNewVal = GetValue( vPos, vDir, iLevel );
	//}
	float fInterpVal = fLevel - floor( fLevel );
	fInterpVal *= fInterpVal;
#ifdef ANISOTROPIC
	fDelta = pow( 2.f, ( fLevel - 1.f ) * 2.f ) / 2.f;
	return ( 1.f / 2.f ) * lerp( fNewVal, fLastVal, fInterpVal );
#else
	fDelta = pow( 2.f, ( fLevel - 1.f ) * 2.f ) / 3.f;
	return ( 1.f / 3.f ) * lerp( fNewVal, fLastVal, fInterpVal );
#endif
}

float SmoothTraverse( float3 vPos, float3 vDir ) {
	uint3 pos = vPos;

	uint3 vDirNeg = vDir < 0;
	float3 vMirror = vDirNeg ? -1.f : 1.f;
	int3 viMirror = vMirror;
	int3 viBrickOffset = vDirNeg ? 3 : 0;
	float3 vOffset = vDirNeg ? viGridResolution - ( 1.f / 1024.f ) : 0.f;

	vPos = vDirNeg ? 1 - vPos : vPos;
	vPos *= viGridResolution;

	//float fMaxDir = max( abs( vDir.x ), max( abs( vDir.y ), abs( vDir.z ) ) );
	vDir = normalize( vDir );
	float3 vDir_1 = 1.f / vDir;

	vDir = abs( vDir );
	vDir_1 = abs( vDir_1 );

	int iLevel = uiTreeSize;
	
	uint uiPointer = 0;
	uint3 viSamplePos = vOffset + vMirror * floor( vPos );
	uint uiLowMortenPos = Encode( viSamplePos );
	uint uiHighMortenPos = Encode( viSamplePos >> 10 );
	uint Stack[5];
	Stack[iLevel - 1] = uiPointer;
	float3 vMaxPos = float3( viGridResolution );
	uint i = 0;

	int iTestLevel = 1;
	float fTestVal = 0.f;
	float fDistance = 0.f;

	while( true ) {
		i++;
		uint uiSize = 1;
		if( iLevel > 5 ) {
			uiPointer = Traverse( uiHighMortenPos, uiPointer, 1, uiTreeSize - 5, uiSize );
			iLevel = 5;
		}
		if( uiPointer != -1 ) {
			Node current = Tree[uiPointer];

			uint uiNodePos = ( uiLowMortenPos >> ( ( iLevel - 1 ) * 6 ) ) & 0x3f;
			uint uiData = uiNodePos < 32 ? current.Data.x : current.Data.y;
			uint uiDataPos = uiNodePos < 32 ? uiNodePos : uiNodePos - 32;

			if( uiData & ( 1 << uiDataPos ) ) {
				uint uiCompVal = 0x7fffffff >> ( 31 - uiDataPos );
				uint offset = countbits( uint( uiData & uiCompVal ) );
				if( uiNodePos > 31 ) {
					offset += countbits( current.Data.x );
				}
				uiPointer = Pointers[current.Pointer + offset];

				if( iLevel == iTestLevel ) {
					//return GetValue( vPos, 0 );
					fTestVal += GetValue( vOffset + vMirror * vPos, vDir, max( iTestLevel - 2, 0.f ) );
					if( fTestVal > 0.99f )
						return 1.f;
					uiSize = 1 << ( 2 * ( iLevel - 1 ) );
				}
				else {
					--iLevel;
					Stack[iLevel - 1] = uiPointer;
					continue;
				}
			}
			else
				uiSize = 1 << ( 2 * ( iLevel - 1 ) );
		}
		float fSize = uiSize;
		float3 vEdges = ( floor( vPos / fSize ) + float3( 1, 1, 1 ) ) * fSize;
		float3 t = ( vEdges - vPos ) * vDir_1;
		float fTMin = min( t.x, min( t.y, t.z ) );

		vPos += vDir * fTMin;
		uint uiTestVal = 0;
		if( fTMin == t.x ) {
			vPos.x = vEdges.x;
		}
		else if( fTMin == t.y ) {
			vPos.y = vEdges.y;
		}
		else {
			vPos.z = vEdges.z;
		}

		if( any( vPos >= vMaxPos ) )
			break;
		
		fDistance += fTMin;
		float fAngle = radians( 5.0 );
		iTestLevel = clamp( log2( fDistance * fAngle ), 0.f, uiTreeSize - 1 ) + 1;

		uint3 viNewSamplePos = vOffset + vMirror * floor( vPos );
		uint uiNewLowMortenPos = Encode( viNewSamplePos );
		uint uiNewHighMortenPos = Encode( viNewSamplePos >> 10 );
		if( any( uiNewHighMortenPos ^ uiHighMortenPos ) ) {
			iLevel = uiTreeSize;
			uiPointer = 0;
		}
		else {
			uint3 viDifference = viSamplePos ^ viNewSamplePos;
			uint uiDiff = viDifference.x | viDifference.y | viDifference.z;

			iLevel = min( uiTreeSize, ( firstbithigh( uiDiff ) >> 1 ) + 1 );
			uiPointer = Stack[iLevel - 1];
		}
		viSamplePos = viNewSamplePos;
		uiLowMortenPos = uiNewLowMortenPos;
		uiHighMortenPos = uiNewHighMortenPos;
	}
	return fTestVal;
}

bool TraverseTree( float3 vPos, float3 vDir, out float3 color ) {
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


	uint uiSize = 1;
	uint i;
	for( i = 0; true; ++i ) {
		uint3 viSamplePos = vOffset + vMirror * floor( vPos );
		uint uiBrick = GetBrick( viSamplePos, uiSize );
		if( uiBrick == 0 ) {
			float val = i / fMaxItVal;
			color = float3( val, 0, 0 );
			return true;
		}
		if( uiBrick != -1 ) {
			float3 vInBrickPos = ( vPos - floor( vPos ) ) * 4.f;
			if( CheckBrick( uiBrick, vInBrickPos, vDir, vDir_1, viMirror, viBrickOffset ) ) {
				float val = i / fMaxItVal;
				color = float3( val, 0, 0 );
				return true;
			}
		}

		float fSize = uiSize;
		float3 vEdges = ( floor( vPos / fSize ) + float3( 1, 1, 1 ) ) * fSize;
		float3 t = ( vEdges - vPos ) * vDir_1;
		float fTMin = min( t.x, min( t.y, t.z ) );

		vPos += vDir * fTMin;
		uint uiTestVal = 0;
		if( fTMin == t.x ) {
			vPos.x = vEdges.x;
		}
		else if( fTMin == t.y ) {
			vPos.y = vEdges.y;
		}
		else {
			vPos.z = vEdges.z;
		}

		// test if still in box
		if( any( vPos >= int3( viGridResolution ) ) )
			break;
	}
	//color = float3( 0, i / 20.0f, 0 );
	return false;
}

float SmoothTraverse1( float3 vPos, float3 vDir, float fLightAngle ) {
	uint3 vDirNeg = vDir < 0;
	float3 vMirror = vDirNeg ? -1.f : 1.f;
	int3 viMirror = vMirror;
	int3 viBrickOffset = vDirNeg ? 3 : 0;
	float3 vOffset = vDirNeg ? viGridResolution - ( 1.f / 1024.f ) : 0.f;

	vPos = vDirNeg ? 1 - vPos : vPos;
	vPos *= viGridResolution;

	vDir = normalize( vDir );
	float3 vDir_1 = 1.f / vDir;

	vDir = abs( vDir );
	vDir_1 = abs( vDir_1 );


	uint uiLevel = 0;
	uint i;
	float fAngleToDistance = 2.f * tan( 0.5f * fLightAngle );

	float fLevel = 0.f;
	float fDistance = 0.f;

	
	/*for( i = 0; i < 200; ++i ) {
		uint3 viSamplePos = vOffset + vMirror * floor( vPos );
		uint uiSize;
		uint uiBrick = GetBrick( viSamplePos, uiSize );
		if( uiBrick != -1 ) {
			float3 vInBrickPos = ( vPos - floor( vPos ) ) * 4.f;
			if( CheckBrick( uiBrick, vInBrickPos, vDir, vDir_1, viMirror, viBrickOffset ) ) {
				return 1.f;
			}
		}

		float fSize = uiSize;
		float3 vEdges = ( floor( vPos / fSize ) + float3( 1, 1, 1 ) ) * fSize;
		float3 t = ( vEdges - vPos ) * vDir_1;
		float fTMin = min( t.x, min( t.y, t.z ) );

		vPos += vDir * fTMin;
		uint uiTestVal = 0;
		if( fTMin == t.x ) {
			vPos.x = vEdges.x;
		}
		else if( fTMin == t.y ) {
			vPos.y = vEdges.y;
		}
		else {
			vPos.z = vEdges.z;
		}

		// test if still in box
		if( any( vPos >= float3( viGridResolution ) ) )
			return 0.f;

		fDistance += fTMin;
		fLevel = log2( fDistance * fAngleToDistance );

		if( fLevel > 1.f )
			break;
	}*/
	
	fLevel = 1.f;
	float fOutVal = 0.f;
	int iLevel = uiTreeSize;
	while( all( vPos < float3( viGridResolution ) ) ) {
		float fDelta;
		float3 vInBrickPos = ( vPos - floor( vPos ) ) * 4.f;
		fOutVal += ( 1.f - fOutVal ) * GetValue1( vOffset + vMirror * vPos, vInBrickPos, vDir, vDir_1, viMirror, viBrickOffset, fLevel, fDelta, iLevel );
		if( fOutVal > 0.99f )
			break;

		vPos += fDelta * vDir;
		fDistance += fDelta;

		fLevel = max( log2( fDistance * fAngleToDistance ) / 2.f + 1.f, 1.f );
		iLevel = min( iLevel + 1, uiTreeSize );
	}

	return saturate( fOutVal );
}

float SmoothTraverse2( float3 vPos, float3 vDir, float fLightAngle ) {
	uint3 vDirNeg = vDir < 0;
	float3 vMirror = vDirNeg ? -1.f : 1.f;
	int3 viMirror = vMirror;
	int3 viBrickOffset = vDirNeg ? 3 : 0;
	float3 vOffset = vDirNeg ? viGridResolution - ( 1.f / 1024.f ) : 0.f;

	vPos = vDirNeg ? 1 - vPos : vPos;
	vPos *= viGridResolution;

	vDir = normalize( vDir );
	float3 vDir_1 = 1.f / vDir;

	vDir = abs( vDir );
	vDir_1 = abs( vDir_1 );

	float fDistance = 0.f;
	int iLevel = 0;
	float fAngleToDistance = 2.f * tan( 0.25f * fLightAngle );
	uint uiSize;
	for( uint i = 0; true; ++i ) {
		uint3 viSamplePos = vOffset + vMirror * floor( vPos );
		uint uiSampleLevel = max( iLevel - 1, 0 );
		uint uiBrick = GetBrick1( viSamplePos, uiSampleLevel );
		if( uiBrick == 0 ) {
			return 1.f;
		}

		float fSize = 1 << ( 2 * uiSampleLevel );
		float3 vEdges = ( floor( vPos / fSize ) + float3( 1, 1, 1 ) ) * fSize;
		float3 t = ( vEdges - vPos ) * vDir_1;
		float fTMin = min( t.x, min( t.y, t.z ) );

		float fWeight = fTMin / fSize;

		if( uiBrick != -1 ) {
			if( iLevel == 0 ) {
				float3 vInBrickPos = ( vPos - floor( vPos ) ) * 4.f;
				if( CheckBrick( uiBrick, vInBrickPos, vDir, vDir_1, viMirror, viBrickOffset ) ) {
					return 1.f;
				}
			}
			else {
				return 1.f;
			}
		}

		vPos += vDir * fTMin;
		uint uiTestVal = 0;
		if( fTMin == t.x ) {
			vPos.x = vEdges.x;
		}
		else if( fTMin == t.y ) {
			vPos.y = vEdges.y;
		}
		else {
			vPos.z = vEdges.z;
		}

		fDistance += fTMin;
		iLevel = max( log2( fDistance * fAngleToDistance ) / 2.f + 1.f, 0.f );

		// test if still in box
		if( any( vPos >= int3( viGridResolution ) ) )
			break;
	}
	return 0.f;
}