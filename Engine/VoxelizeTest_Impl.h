
#include "Math.h"

#include <set>

static const uint3 gridSize = { 16, 16, 16 };
static const uint3 numTexels = { 4, 4, 8 };
static const float3 deltaGrid = { .125f, .125f, .125f };
static const float3 invDeltaGrid = { 8.f, 8.f, 8.f };
static const uint3 numBits = { 4, 4, 2 };
static const float3 minBoxPos = { -1.f, -1.f, -1.f };

static std::set<uint32_t> TestSet;

struct TriangleVals {
	float3 n;
	float d1;
	float d2;
	float d1T;
	float d2T;
	float2 n_xy[3];
	float d_xy[3];
	float d_xyT[3];
	float2 n_xz[3];
	float d_xz[3];
	float d_xzT[3];
	float2 n_yz[3];
	float d_yz[3];
	float d_yzT[3];
};

TriangleVals CalculateTriangleVals( float3 tri[3] ) {
	TriangleVals vals;

	// triangle edges
	float3 e[3];
	e[0] = tri[1] - tri[0];
	e[1] = tri[2] - tri[1];
	e[2] = tri[0] - tri[2];

	// normal of the triangle
	vals.n = Normalize( Cross( e[0], -e[2] ) );

	// critical point
	float3 c = float3( 0, 0, 0 );
	if( vals.n.x > 0 )
		c.x = deltaGrid.x;
	if( vals.n.y > 0 )
		c.y = deltaGrid.y;
	if( vals.n.z > 0 )
		c.z = deltaGrid.z;

	// d for bit width
	vals.d1 = Dot( vals.n, c - tri[0] );
	vals.d2 = Dot( vals.n, ( deltaGrid - c ) - tri[0] );
	// d for texel width
	float3 deltaTex = deltaGrid * float3( numBits.x, numBits.y, numBits.z );
	c *= float3( numBits.x, numBits.y, numBits.z );
	vals.d1T = Dot( vals.n, c - tri[0] );
	vals.d2T = Dot( vals.n, ( deltaTex - c ) - tri[0] );

	for( uint32_t i = 0; i < 3; ++i ) {
		// ns and ds for bit width
		vals.n_xy[i] = float2( -e[i].y, e[i].x );
		if( vals.n.z < 0 )
			vals.n_xy[i] *= -1;
		vals.d_xy[i] = -Dot( vals.n_xy[i], float2( tri[i].x, tri[i].y ) ) + max( 0, deltaGrid.x * vals.n_xy[i].x ) + max( 0, deltaGrid.y * vals.n_xy[i].y );

		vals.n_xz[i] = float2( -e[i].z, e[i].x );
		if( vals.n.y < 0 )
			vals.n_xz[i] *= -1;
		vals.d_xz[i] = -Dot( vals.n_xz[i], float2( tri[i].x, tri[i].z ) ) + max( 0, deltaGrid.x * vals.n_xz[i].x ) + max( 0, deltaGrid.z * vals.n_xz[i].y );

		vals.n_yz[i] = float2( -e[i].z, e[i].y );
		if( vals.n.x < 0 )
			vals.n_yz[i] *= -1;
		vals.d_yz[i] = -Dot( vals.n_yz[i], float2( tri[i].y, tri[i].z ) ) + max( 0, deltaGrid.y * vals.n_yz[i].x ) + max( 0, deltaGrid.z * vals.n_yz[i].y );

		// ds for texel width
		vals.d_xyT[i] = -Dot( vals.n_xy[i], float2( tri[i].x, tri[i].y ) ) + max( 0, deltaTex.x * vals.n_xy[i].x ) + max( 0, deltaTex.y * vals.n_xy[i].y );
		vals.d_xzT[i] = -Dot( vals.n_xz[i], float2( tri[i].x, tri[i].z ) ) + max( 0, deltaTex.x * vals.n_xz[i].x ) + max( 0, deltaTex.z * vals.n_xz[i].y );
		vals.d_yzT[i] = -Dot( vals.n_yz[i], float2( tri[i].y, tri[i].z ) ) + max( 0, deltaTex.y * vals.n_yz[i].x ) + max( 0, deltaTex.z * vals.n_yz[i].y );
	}


	return vals;
}

void CalculateTriangleBox( float3 tri[3], float3& triBoxMin, float3& triBoxMax ) {
	triBoxMin.x = min( tri[0].x, min( tri[1].x, tri[2].x ) );
	triBoxMax.x = max( tri[0].x, max( tri[1].x, tri[2].x ) );
	triBoxMin.y = min( tri[0].y, min( tri[1].y, tri[2].y ) );
	triBoxMax.y = max( tri[0].y, max( tri[1].y, tri[2].y ) );
	triBoxMin.z = min( tri[0].z, min( tri[1].z, tri[2].z ) );
	triBoxMax.z = max( tri[0].z, max( tri[1].z, tri[2].z ) );
}

bool HitElement( float3 pos, float3 n, float d1, float d2, float2 n_xy[3], float d_xy[3],
				 float2 n_xz[3], float d_xz[3], float2 n_yz[3], float d_yz[3] ) {
	float n_dot_p = Dot( n, pos );
	if( ( n_dot_p + d1 ) * ( n_dot_p + d2 ) > 0 )
		return false;

	for( uint32_t i = 0; i < 3; ++i ) {
		if( Dot( n_xy[i], float2( pos.x, pos.y ) ) + d_xy[i] < 0 )
			return false;
	}
	for( uint32_t j = 0; j < 3; ++j ) {
		if( Dot( n_xz[j], float2( pos.x, pos.z ) ) + d_xz[j] < 0 )
			return false;
	}
	for( uint32_t k = 0; k < 3; ++k ) {
		if( Dot( n_yz[k], float2( pos.y, pos.z ) ) + d_yz[k] < 0 )
			return false;
	}
	return true;
}

bool HitTexel( float3 pos, uint3 start, uint3 end, TriangleVals vals, uint32_t& bits ) {
	bits = 0;
	bool hit = false;
	if( HitElement( pos, vals.n, vals.d1T, vals.d2T, vals.n_xy, vals.d_xyT, vals.n_xz, vals.d_xzT, vals.n_yz, vals.d_yzT ) ) {
		for( uint32_t z = start.z; z <= end.z; ++z ) {
			for( uint32_t y = start.y; y <= end.y; ++y ) {
				for( uint32_t x = start.x; x <= end.x; ++x ) {
					float3 bitPos = pos + float3( x, y, z ) * deltaGrid;
					if( HitElement( bitPos, vals.n, vals.d1, vals.d2, vals.n_xy, vals.d_xy, vals.n_xz, vals.d_xz, vals.n_yz, vals.d_yz ) ) {
						uint32_t bit = x + y * numBits.x + z * numBits.x * numBits.y;
						bits |= 1 << bit;
						hit = true;
					}
				}
			}
		}
	}
	return hit;
}

void ProcessTexel( uint32_t x, uint32_t y, uint32_t z, uint3 bitStart, uint3 bitEnd, TriangleVals vals, uint32_t* grid ) {
	float3 pos = minBoxPos + float3( x, y, z ) * deltaGrid * float3( numBits.x, numBits.y, numBits.z );
	uint32_t bufferPos = x + y * numTexels.x + z * numTexels.x * numTexels.y;

	uint32_t bits = 0;
	if( HitTexel( pos, bitStart, bitEnd, vals, bits ) )
		grid[bufferPos] |= bits;
}

void voxelize( uint32_t numTriangles, float3* vertices, uint32_t* indices, uint32_t* grid ) {
	float3 tri[3];

	for( uint32_t id = 0; id < numTriangles; ++id ) {
		for( uint32_t i = 0; i < 3; ++i )
			tri[i] = vertices[indices[id * 3 + i]];

		// calculate Bounding box of triangle
		float3 triBoxMin;
		float3 triBoxMax;
		CalculateTriangleBox( tri, triBoxMin, triBoxMax );

		uint3 start;
		start.x = max( ( triBoxMin.x - minBoxPos.x ) * invDeltaGrid.x, 0 );
		start.y = max( ( triBoxMin.y - minBoxPos.y ) * invDeltaGrid.y, 0 );
		start.z = max( ( triBoxMin.z - minBoxPos.z ) * invDeltaGrid.z, 0 );
		uint3 end;
		end.x = min( ( triBoxMax.x - minBoxPos.x ) * invDeltaGrid.x, gridSize.x - 1 );
		end.y = min( ( triBoxMax.y - minBoxPos.y ) * invDeltaGrid.y, gridSize.y - 1 );
		end.z = min( ( triBoxMax.z - minBoxPos.z ) * invDeltaGrid.z, gridSize.z - 1 );
		uint3 bitStart;
		bitStart.x = start.x & ( numBits.x - 1 );
		bitStart.y = start.y & ( numBits.y - 1 );
		bitStart.z = start.z & ( numBits.z - 1 );
		uint3 bitEnd;
		bitEnd.x = end.x & ( numBits.x - 1 );
		bitEnd.y = end.y & ( numBits.y - 1 );
		bitEnd.z = end.z & ( numBits.z - 1 );
		start.x = start.x >> 2;
		start.y = start.y >> 2;
		start.z = start.z >> 1;
		end.x = end.x >> 2;
		end.y = end.y >> 2;
		end.z = end.z >> 1;

		TriangleVals vals = CalculateTriangleVals( tri );

		uint3 startBit = bitStart;
		uint3 endBit = uint3( numBits.x - 1, numBits.y - 1, numBits.z - 1 );
		for( uint32_t z = start.z; z <= end.z; ++z ) {
			if( z == end.z )
				endBit.z = bitEnd.z;
			for( uint32_t y = start.y; y <= end.y; ++y ) {
				if( y == end.y )
					endBit.y = bitEnd.y;
				for( uint32_t x = start.x; x <= end.x; ++x ) {
					if( x == end.x )
						endBit.x = bitEnd.x;
					ProcessTexel( x, y, z, startBit, endBit, vals, grid );
					startBit.x = 0;
				}
				startBit.x = bitStart.x;
				endBit.x = numBits.x - 1;
				startBit.y = 0;
			}
			startBit.y = bitStart.y;
			endBit.y = numBits.y - 1;
			startBit.z = 0;
		}
	}

}