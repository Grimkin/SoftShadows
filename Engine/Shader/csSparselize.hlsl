#include "VoxelDefines.hlsli"

Texture3D<uint> Grid : register( t0 );

#ifdef ADVANCED_TRAVERSE
struct Elem {
	uint3 Value;
	uint Pointer;
};
RWStructuredBuffer<Elem> Tree : register( u1 );
#else
RWBuffer<uint> Tree : register( u1 );
#endif
RWStructuredBuffer<uint2> Bricks : register( u2 );

cbuffer Params {
	uint3 viGridSize;
	uint uiTreeSize;
};

#ifdef ADVANCED_TRAVERSE
uint3 CalcValue( uint2 uiBrick ) {
	uint3 viValue = uint3( 0, 0, 0 );
	// X-Face
	for( uint i = 0; i < 8; ++i ) {
		if( uiBrick.x  & ( 0xf << ( i * 4 ) ) )
			viValue.x |= 1 << i;
		if( uiBrick.y  & ( 0xf << ( i * 4 ) ) )
			viValue.x |= 1 << ( i + 8 );
	}
	//Y-Face
	for( uint j = 0; j < 4; ++j ) {
		if( uiBrick.x & ( 0x1111 << j ) )
			viValue.y |= 1 << i;
		if( uiBrick.y & ( 0x1111 << j ) )
			viValue.y |= 1 << ( i + 4 );
		if( uiBrick.x & ( 0x1111 << ( j + 16 ) ) )
			viValue.y |= 1 << ( i + 8 );
		if( uiBrick.y & ( 0x1111 << ( j + 16 ) ) )
			viValue.y |= 1 << ( i + 12 );
	}
	// Z-Face
	for( uint k = 0; k < 16; ++k ) {
		if( uiBrick.x & ( 0x10001 << k ) || uiBrick.y & ( 0x10001 << k ) )
			viValue.z |= 1 << i;
	}
	return viValue;
}
#endif

[numthreads( 4, 4, 4 )]
void main( uint3 id : SV_DispatchThreadID ) {

	if( id.x >= viGridSize.x || id.y >= viGridSize.y || id.z >= viGridSize.z )
		return;

	// combine two 4*4*2 bricks to one 4*4*4 brick
	uint3 viBrickPos1 = uint3( id.xy, id.z << 1 );
	uint3 viBrickPos2 = uint3( id.xy, (id.z << 1) + 1 );
	uint2 uiBrick = uint2( Grid[viBrickPos1], Grid[viBrickPos2] );
	// do nothin if brick is empty
	uint brickIdx;
	if( uiBrick.x || uiBrick.y ) {
		// store brick in brickbuffer
		brickIdx = Bricks.IncrementCounter();
		Bricks[brickIdx] = uiBrick;
	}
#ifdef ADVANCED_TRAVERSE
		uint3 uiBrickValue = CalcValue( uiBrick );
#endif

		uint uiOffset = 0;
		uint uiDeltaOffset = 0;
		// iterate through whole tree and update the pointers
		for( uint i = 1; i < uiTreeSize; i++ ) {
			// calc deltaPos in current level
			uint uiDeltaPos = 0;
			uiDeltaPos += ( id.x >> ( uiTreeSize - i ) ) & 1;
			uiDeltaPos += 2 * ( ( id.y >> ( uiTreeSize - i ) ) & 1 );
			uiDeltaPos += 4 * ( ( id.z >> ( uiTreeSize - i ) ) & 1 );

			// calc offset to children: old offset + 8^treedepth

			uint uiNewOffset = uiOffset + ( 1 << ( i * 3 ) );
			uint uiNewDeltaOffset = ( uiDeltaOffset + uiDeltaPos ) << 3;

			// update pointer, so it points to the children, no atomic needed, 
			// because every access to that pointer will be the same
#ifdef ADVANCED_TRAVERSE
			if( uiBrick.x || uiBrick.y )
				Tree[uiOffset + uiDeltaOffset + uiDeltaPos].Pointer = uiNewOffset + uiNewDeltaOffset;
			Tree[uiOffset + uiDeltaOffset + uiDeltaPos].Value = uint3( 0,0,0);
#else
			if( uiBrick.x || uiBrick.y )
				Tree[uiOffset + uiDeltaOffset + uiDeltaPos] = uiNewOffset + uiNewDeltaOffset;
#endif

			uiOffset = uiNewOffset;
			uiDeltaOffset = uiNewDeltaOffset;
		}

		uint uiDeltaPos = 0;
		uiDeltaPos += ( id.x >> ( uiTreeSize - i ) ) & 1;
		uiDeltaPos += 2 * ( ( id.y >> ( uiTreeSize - i ) ) & 1 );
		uiDeltaPos += 4 * ( ( id.z >> ( uiTreeSize - i ) ) & 1 );

#ifdef ADVANCED_TRAVERSE
		if( uiBrick.x || uiBrick.y )
			Tree[uiOffset + uiDeltaOffset + uiDeltaPos].Pointer = brickIdx;
		Tree[uiOffset + uiDeltaOffset + uiDeltaPos].Value = uiBrickValue;
#else
		if( uiBrick.x || uiBrick.y )
			Tree[uiOffset + uiDeltaOffset + uiDeltaPos] = brickIdx;
#endif
/*	}
#ifdef ADVANCED_TRAVERSE
	else {
		uint uiOffset = ( 8.0f / 7.0f ) * ( ( 1 << ( ( uiTreeSize - 1 ) * 3 ) ) - 1 );
		uint uiDeltaOffset = 0;
		for( uint i = 1; i <= uiTreeSize; ++i ) {
			uint uiDeltaPos = 0;
			uiDeltaPos += ( id.x >> ( uiTreeSize - i ) ) & 1;
			uiDeltaPos += 2 * ( ( id.y >> ( uiTreeSize - i ) ) & 1 );
			uiDeltaPos += 4 * ( ( id.z >> ( uiTreeSize - i ) ) & 1 );

			uiDeltaOffset <<= 3;
			uiDeltaOffset += uiDeltaPos;
		}
		Tree[uiOffset + uiDeltaOffset].Value = uint3( 0, 0, 0 );
	}
#endif*/
}
