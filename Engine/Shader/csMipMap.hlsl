
struct Elem {
	uint3 Value;
	uint Pointer;
};
RWStructuredBuffer<Elem> Tree : register( u1 );
RWStructuredBuffer<uint2> Bricks : register( u2 );

cbuffer Params {
	uint3 viGridSize;
	uint uiTreeSize;
};

uint GetDelta( uint3 viPos, uint level ) {
	uint uiDeltaOffset = 0;
	for( uint i = 1; i <= uiTreeSize - level; ++i ) {
		uint uiDeltaPos = 0;
		uiDeltaPos += ( viPos.x >> ( uiTreeSize - i ) ) & 1;
		uiDeltaPos += 2 * ( ( viPos.y >> ( uiTreeSize - i ) ) & 1 );
		uiDeltaPos += 4 * ( ( viPos.z >> ( uiTreeSize - i ) ) & 1 );

		uiDeltaOffset <<= 3;
		uiDeltaOffset += uiDeltaPos;
	}
	return uiDeltaOffset;
}

uint GetOffset( uint level ) {
	return ( 8.0f / 7.0f ) * ( ( 1 << ( ( uiTreeSize - level - 1 ) * 3 ) ) - 1 );
}

uint GetIdx( uint3 viPos ) {
	return GetOffset( 0 ) + GetDelta( viPos, 0 );
}

[numthreads(16, 16, 1)]
void main( uint2 id : SV_DispatchThreadID ){
	uint uiSetBits = 0;

	uint uiOffset = GetOffset( 0 );
	for( uint uiLevelX = 1; uiLevelX < uiTreeSize; ++uiLevelX ) {
		uint uiLevelSize = 1 << uiLevelX;
		uint uiDeltaOffset = GetDelta( uint3( 0, id ), 0 );
		uiSetBits = Tree[uiOffset + uiDeltaOffset].Value.x;
		++uiDeltaOffset;
		for( uint x = 1; x < viGridSize.x; x++ ) {
			uiSetBits |= Tree[uiOffset + uiDeltaOffset].Value.x;
			if( !( (x+1) & ( uiLevelSize - 1 ) ) ) {
				if( uiSetBits ) {
					uint uiTreeVal = countbits( uiSetBits );
					InterlockedAdd( Tree[GetOffset( uiLevelX ) + ( uiDeltaOffset >> ( 3 * uiLevelX ) )].Value.x, uiTreeVal );
				}
				uiSetBits = 0;
			}
			if( x & 1 ) {
				++uiDeltaOffset;
			}
			else {
				uiDeltaOffset = GetDelta( uint3( x, id ), 0 );
			}
		}
	}
	for( uint uiLevelY = 1; uiLevelY < uiTreeSize; ++uiLevelY ) {
		uint uiLevelSize = 1 << uiLevelY;
		uint uiDeltaOffset = GetDelta( uint3( id.x, 0, id.y ), 0 );
		uiSetBits = Tree[uiOffset + uiDeltaOffset].Value.y;
		++uiDeltaOffset;
		for( uint y = 1; y < viGridSize.y; y++ ) {
			if( !( y & ( uiLevelSize - 1 ) ) ) {
				if( uiSetBits ) {
					uint uiTreeVal = countbits( uiSetBits );
					InterlockedAdd( Tree[GetOffset( uiLevelY ) + ( uiDeltaOffset >> ( 3 * uiLevelY ) )].Value.y, uiTreeVal );
				}
				uiSetBits = 0;
			}
			uiSetBits |= Tree[uiOffset + uiDeltaOffset].Value.y;
			if( y & 1 ) {
				uiDeltaOffset += 2;
			}
			else {
				uiDeltaOffset = GetDelta( uint3( id.x, y, id.y ), 0 );
			}
		}
	}
	for( uint uiLevelZ = 1; uiLevelZ < uiTreeSize; ++uiLevelZ ) {
		uint uiLevelSize = 1 << uiLevelZ;
		uint uiDeltaOffset = GetDelta( uint3( id, 0 ), 0 );
		uiSetBits = Tree[uiOffset + uiDeltaOffset].Value.z;
		++uiDeltaOffset;
		for( uint z = 1; z < viGridSize.z; z++ ) {
			if( !( z & ( uiLevelSize - 1 ) ) ) {
				if( uiSetBits ) {
					uint uiTreeVal = countbits( uiSetBits );
					InterlockedAdd( Tree[GetOffset( uiLevelZ ) + ( uiDeltaOffset >> ( 3 * uiLevelZ ) )].Value.z, uiTreeVal );
				}
				uiSetBits = 0;
			}
			uiSetBits |= Tree[uiOffset + uiDeltaOffset].Value.z;
			if( z & 1 ) {
				uiDeltaOffset += 4;
			}
			else {
				uiDeltaOffset = GetDelta( uint3( id, z ), 0 );
			}
		}
	}
}
