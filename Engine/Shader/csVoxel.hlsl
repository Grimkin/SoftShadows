#include "VoxelDefines.hlsli"


ByteAddressBuffer vertices : register( t0 );
Buffer<uint> indices : register( t1 );

struct Brick {
	uint2 Data;
	uint Position;
};

RWStructuredBuffer<Brick> Bricks : register( u1 );
RWBuffer<uint> Counter : register( u2 );

cbuffer Constants : register( c0 ) {
	uint3 gridSize;
	uint3 numTexels;
	float3 deltaGrid;
	float3 invDeltaGrid;
	uint3 numBits;
	float3 minBoxPos;
	float3 boxSize;
};

cbuffer ObjectData : register( c1 ) {
	float4x4 worldMat;
	uint numTriangles;
};

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
	vals.n = normalize( cross( e[0], -e[2] ) );

	// critical point
	float3 c = float3( 0, 0, 0 );
	if( vals.n.x > 0 )
		c.x = deltaGrid.x;
	if( vals.n.y > 0 )
		c.y = deltaGrid.y;
	if( vals.n.z > 0 )
		c.z = deltaGrid.z;

	// d for bit width
	vals.d1 = dot( vals.n, c - tri[0] );
	vals.d2 = dot( vals.n, ( deltaGrid - c ) - tri[0] );
	// d for texel width
	float3 deltaTex = deltaGrid * numBits;
	c *= numBits;
	vals.d1T = dot( vals.n, c - tri[0] );
	vals.d2T = dot( vals.n, ( deltaTex - c ) - tri[0] );

	[unroll]
	for( uint i = 0; i < 3; ++i ) {
		// ns and ds for bit width
		vals.n_xy[i] = float2( -e[i].y, e[i].x );
		if( vals.n.z < 0 )
			vals.n_xy[i] *= -1;
		vals.d_xy[i] = -dot( vals.n_xy[i], tri[i].xy ) + max( 0, deltaGrid.x * vals.n_xy[i].x ) + max( 0, deltaGrid.y * vals.n_xy[i].y );

		vals.n_xz[i] = float2( e[i].z, -e[i].x );
		if( vals.n.y < 0 )
			vals.n_xz[i] *= -1;
		vals.d_xz[i] = -dot( vals.n_xz[i], tri[i].xz ) + max( 0, deltaGrid.x * vals.n_xz[i].x ) + max( 0, deltaGrid.z * vals.n_xz[i].y );

		vals.n_yz[i] = float2( -e[i].z, e[i].y );
		if( vals.n.x < 0 )
			vals.n_yz[i] *= -1;
		vals.d_yz[i] = -dot( vals.n_yz[i], tri[i].yz ) + max( 0, deltaGrid.y * vals.n_yz[i].x ) + max( 0, deltaGrid.z * vals.n_yz[i].y );

		// ds for texel width
		vals.d_xyT[i] = -dot( vals.n_xy[i], tri[i].xy ) + max( 0, deltaTex.x * vals.n_xy[i].x ) + max( 0, deltaTex.y * vals.n_xy[i].y );
		vals.d_xzT[i] = -dot( vals.n_xz[i], tri[i].xz ) + max( 0, deltaTex.x * vals.n_xz[i].x ) + max( 0, deltaTex.z * vals.n_xz[i].y );
		vals.d_yzT[i] = -dot( vals.n_yz[i], tri[i].yz ) + max( 0, deltaTex.y * vals.n_yz[i].x ) + max( 0, deltaTex.z * vals.n_yz[i].y );
	}


	return vals;
}

void CalculateTriangleBox( float3 tri[3], out float3 triBoxMin, out float3 triBoxMax ) {
	triBoxMin.x = min( tri[0].x, min( tri[1].x, tri[2].x ) );
	triBoxMax.x = max( tri[0].x, max( tri[1].x, tri[2].x ) );
	triBoxMin.y = min( tri[0].y, min( tri[1].y, tri[2].y ) );
	triBoxMax.y = max( tri[0].y, max( tri[1].y, tri[2].y ) );
	triBoxMin.z = min( tri[0].z, min( tri[1].z, tri[2].z ) );
	triBoxMax.z = max( tri[0].z, max( tri[1].z, tri[2].z ) );
}

bool HitElement( float3 pos, float3 n, float d1, float d2, float2 n_xy[3], float d_xy[3],
				 float2 n_xz[3], float d_xz[3], float2 n_yz[3], float d_yz[3] ) {
	float n_dot_p = dot( n, pos );
	if( ( n_dot_p + d1 ) * ( n_dot_p + d2 ) > 0 )
		return false;

	[unroll]
	for( uint i = 0; i < 3; ++i ) {
		if( dot( n_xy[i], pos.xy ) + d_xy[i] < 0 )
			return false;
	}
	[unroll]
	for( uint j = 0; j < 3; ++j ) {
		if( dot( n_xz[j], pos.xz ) + d_xz[j] < 0 )
			return false;
	}
	[unroll]
	for( uint k = 0; k < 3; ++k ) {
		if( dot( n_yz[k], pos.yz ) + d_yz[k] < 0 )
			return false;
	}
	return true;
}

uint Encode( uint3 pos ) {
	// Generate Morten Code
	pos &= 0x3ff;
	pos = ( pos | ( pos << 16 ) ) & 0x030000ff;
	pos = ( pos | ( pos << 8 ) ) & 0x0300f00f;
	pos = ( pos | ( pos << 4 ) ) & 0x030c30c3;
	pos = ( pos | ( pos << 2 ) ) & 0x09249249;

	return pos.x | ( pos.y << 1 ) | ( pos.z << 2 );
}

void HitTexel( float3 pos, uint3 bitStart, uint3 bitEnd, TriangleVals vals, out uint2 bits ) {
	bits = 0;
	// test texel for hit, only if hit test the single voxel bits
	if( HitElement( pos, vals.n, vals.d1T, vals.d2T, vals.n_xy, vals.d_xyT, vals.n_xz, vals.d_xzT, vals.n_yz, vals.d_yzT ) ) {
		for( uint z = bitStart.z; z <= bitEnd.z; ++z ) {
			for( uint y = bitStart.y; y <= bitEnd.y; ++y ) {
				for( uint x = bitStart.x; x <= bitEnd.x; ++x ) {
					float3 bitPos = pos + float3( x, y, z ) * deltaGrid;
					if( HitElement( bitPos, vals.n, vals.d1, vals.d2, vals.n_xy, vals.d_xy, vals.n_xz, vals.d_xz, vals.n_yz, vals.d_yz ) ) {
						if( z < 2 ) {
							uint bit = Encode( uint3( x, y, z ) );
							bits.x |= 1 << bit;
						}
						else {
							uint bit = Encode( uint3( x, y, z - 2 ) );
							bits.y |= 1 << bit;
						}
					}
				}
			}
		}
	}
}

void ProcessTexel( uint x, uint y, uint z, uint3 bitStart, uint3 bitEnd, TriangleVals vals ) {
	float3 pos = minBoxPos + float3( x, y, z ) * deltaGrid * numBits;
	uint bufferPos = x + y * numTexels.x + z * numTexels.x * numTexels.y;

	uint2 bits;
	HitTexel( pos, bitStart, bitEnd, vals, bits );
	if( any( bits > 0 ) ) {
		uint idx;
		InterlockedAdd( Counter[0], 1, idx );
		Brick brick;
		brick.Data = bits;
		brick.Position = Encode( uint3( x, y, z ) );
		Bricks[idx] = brick;
	}
}

[numthreads( 128, 1, 1 )]
void main( uint id : SV_DispatchThreadID ) {
	if( id >= numTriangles )
		return;

	float3 tri[3];

	[unroll]
	for( uint i = 0; i < 3; ++i ) {
		uint idx = indices[id * 3 + i] * 12;
		tri[i].x = asfloat( vertices.Load( idx ) );
		tri[i].y = asfloat( vertices.Load( idx + 4 ) );
		tri[i].z = asfloat( vertices.Load( idx + 8 ) );
		tri[i] = mul( worldMat, float4( tri[i], 1 ) ).xyz;
	}

	// calculate Bounding box of triangle
	float3 triBoxMin;
	float3 triBoxMax;
	CalculateTriangleBox( tri, triBoxMin, triBoxMax );

	if( any( triBoxMin > minBoxPos + boxSize ) || any( triBoxMax < minBoxPos ) )
		return;

	// calculate start and end voxel
	uint3 start = max( ( triBoxMin - minBoxPos ) * invDeltaGrid, 0 );
	uint3 end = min( ( triBoxMax - minBoxPos ) * invDeltaGrid, gridSize - 1 );
	// start and endbit in the boarder texels
	uint3 bitStart = start & ( numBits - 1 );
	uint3 bitEnd = end & ( numBits - 1 );
	// to avoid division manually use shift vals
	uint3 shift = uint3( 2, 2, 2 );

	start = start >> shift;
	end = end >> shift;

	TriangleVals vals = CalculateTriangleVals( tri );

	uint3 startBit = bitStart;
	uint3 endBit = numBits - 1;
	// iterate over all texels, that overlap the bb of the triangle
	for( uint z = start.z; z <= end.z; ++z ) {
		if( z == end.z )
			endBit.z = bitEnd.z;
		for( uint y = start.y; y <= end.y; ++y ) {
			if( y == end.y )
				endBit.y = bitEnd.y;
			for( uint x = start.x; x <= end.x; ++x ) {
				if( x == end.x )
					endBit.x = bitEnd.x;
				ProcessTexel( x, y, z, startBit, endBit, vals );
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