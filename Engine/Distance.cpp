#include "Distance.h"

#include <vector>

#include "Math.h"

int3 Decode( uint32_t code ) {
	int3 pos;
	pos.x = code;
	pos.y = code >> 1;
	pos.z = code >> 2;

	pos.x = code;
	pos.y = ( code >> 1 );
	pos.z = ( code >> 2 );
	pos.x &= 0x09249249;
	pos.y &= 0x09249249;
	pos.z &= 0x09249249;
	pos.x |= ( pos.x >> 2 );
	pos.y |= ( pos.y >> 2 );
	pos.z |= ( pos.z >> 2 );
	pos.x &= 0x030c30c3;
	pos.y &= 0x030c30c3;
	pos.z &= 0x030c30c3;
	pos.x |= ( pos.x >> 4 );
	pos.y |= ( pos.y >> 4 );
	pos.z |= ( pos.z >> 4 );
	pos.x &= 0x0300f00f;
	pos.y &= 0x0300f00f;
	pos.z &= 0x0300f00f;
	pos.x |= ( pos.x >> 8 );
	pos.y |= ( pos.y >> 8 );
	pos.z |= ( pos.z >> 8 );
	pos.x &= 0x030000ff;
	pos.y &= 0x030000ff;
	pos.z &= 0x030000ff;
	pos.x |= ( pos.x >> 16 );
	pos.y |= ( pos.y >> 16 );
	pos.z |= ( pos.z >> 16 );
	pos.x &= 0x000003ff;
	pos.y &= 0x000003ff;
	pos.z &= 0x000003ff;

	return pos;
}

float GetDistance( int3 a, int3 b ) {
	float dX = static_cast<float>( a.x - b.x );
	float dY = static_cast<float>( a.y - b.y );
	float dZ = static_cast<float>( a.z - b.z );

	return sqrtf( dX * dX + dY * dY + dZ * dZ );
}

float GetMinDistance( uint2 brick, uint32_t idx ) {
	float distance = INFINITY;

	if( idx < 32 ) {
		if( brick.x & 1 << idx )
			return 0.f;
	}
	else {
		if( brick.y & 1 << ( idx - 32 ) )
			return 0.f;
	}

	int3 pos = Decode( idx );

	for( uint32_t i = 0; i < 32; i++ ) {
		if( brick.x & 1 << i ) {
			distance = min( distance, GetDistance( pos, Decode( i ) ) );
		}
		if( brick.y & 1 << ( i - 32 ) )
			distance = min( distance, GetDistance( pos, Decode( i + 32 ) ) );
	}

	return distance;
}

int ConvertPos( int code ) {
	int3 pos = Decode( code );
	return pos.x + pos.y * 4 + pos.z * 16;
}

int Map( int x, int y, int z ) {
	return x + 4 * y + 16 * z;
}

inline int Map( const int3& pos ) {
	return pos.x + 4 * pos.y + 16 * pos.z;
}

void TestDist( const int3& pos, const int3& testPos, float d, std::vector<float>& vec ) {
	if( testPos.x < 0 || testPos.y < 0 || testPos.z < 0 || testPos.x > 3 || testPos.y > 3 || testPos.z > 3 )
		return;

	if( vec[Map( testPos )] + d < vec[Map( pos )] )
		vec[Map( pos )] = vec[Map( testPos )] + d;
}

std::vector<float> FillDistances( uint2 brick ) {
	std::vector<float> out;
	out.resize( 64 );

	for( uint32_t i = 0; i < 64; i++ ) {
		out[i] = GetMinDistance( brick, i );
	}

	return out;

	for( uint32_t i = 0; i < 32; i++ ) {
		out[ConvertPos( i )] = brick.x & 1 << i ? 0.f : INFINITY;
		out[ConvertPos( i + 32 )] = brick.y & 1 << i ? 0.f : INFINITY;		
	}

	float d1 = 1.f;
	float d2 = sqrtf( 2.f );
	float d3 = sqrtf( 3.f );
	for( int z = 0; z < 4; z++ ) {
		for( int y = 0; y < 4; y++ ) {
			for( int x = 0; x < 4; x++ ) {
				TestDist( { x, y, z }, { x - 1, y - 1, z - 1 }, d3, out );
				TestDist( { x, y, z }, { x, y - 1, z - 1 }, d2, out );
				TestDist( { x, y, z }, { x + 1, y - 1, z - 1 }, d3, out );
				TestDist( { x, y, z }, { x - 1, y, z - 1 }, d2, out );
				TestDist( { x, y, z }, { x, y, z - 1 }, d1, out );
				TestDist( { x, y, z }, { x + 1, y, z - 1 }, d2, out );
				TestDist( { x, y, z }, { x - 1, y + 1, z - 1 }, d3, out );
				TestDist( { x, y, z }, { x, y + 1, z - 1 }, d2, out );
				TestDist( { x, y, z }, { x + 1, y + 1, z - 1 }, d3, out );
				TestDist( { x, y, z }, { x - 1, y - 1, z }, d2, out );
				TestDist( { x, y, z }, { x, y - 1, z }, d1, out );
				TestDist( { x, y, z }, { x + 1, y - 1, z }, d2, out );
				TestDist( { x, y, z }, { x - 1, y, z }, d1, out );
			}
		}
	}

	for( int z = 3; z <= 0; z++ ) {
		for( int y = 3; y <= 0; y++ ) {
			for( int x = 3; x <= 0; x++ ) {
				TestDist( { x, y, z }, { x + 1, y + 1, z + 1 }, d3, out );
				TestDist( { x, y, z }, { x, y + 1, z + 1 }, d2, out );
				TestDist( { x, y, z }, { x - 1, y + 1, z + 1 }, d3, out );
				TestDist( { x, y, z }, { x + 1, y, z + 1 }, d2, out );
				TestDist( { x, y, z }, { x, y, z + 1 }, d1, out );
				TestDist( { x, y, z }, { x - 1, y, z + 1 }, d2, out );
				TestDist( { x, y, z }, { x + 1, y - 1, z + 1 }, d3, out );
				TestDist( { x, y, z }, { x, y - 1, z + 1 }, d2, out );
				TestDist( { x, y, z }, { x - 1, y - 1, z + 1 }, d3, out );
				TestDist( { x, y, z }, { x + 1, y + 1, z }, d2, out );
				TestDist( { x, y, z }, { x, y + 1, z }, d1, out );
				TestDist( { x, y, z }, { x - 1, y + 1, z }, d2, out );
				TestDist( { x, y, z }, { x + 1, y, z }, d1, out );
			}
		}
	}

	return out;
}


float BrickDistance( uint2 brickA, uint2 brickB ) {
	std::vector<float> vecA = FillDistances( brickA );
	std::vector<float> vecB = FillDistances( brickB );

	float distance = 0;

	for( uint32_t i = 0; i < 64; i++ ) {
		float newVal = vecA[i] - vecB[i];
		distance += newVal * newVal;
	}

	return sqrt( distance );
}
