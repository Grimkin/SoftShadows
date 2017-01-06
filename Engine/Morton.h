#pragma once

#include "Math.h"

uint32_t MortonEncode( uint3 pos ) {
	// Generate Morten Code
	pos.x = ( pos.x | ( pos.x << 16 ) ) & 0x030000FF;
	pos.x = ( pos.x | ( pos.x << 8 ) ) & 0x0300F00F;
	pos.x = ( pos.x | ( pos.x << 4 ) ) & 0x030C30C3;
	pos.x = ( pos.x | ( pos.x << 2 ) ) & 0x09249249;

	pos.y = ( pos.y | ( pos.y << 16 ) ) & 0x030000FF;
	pos.y = ( pos.y | ( pos.y << 8 ) ) & 0x0300F00F;
	pos.y = ( pos.y | ( pos.y << 4 ) ) & 0x030C30C3;
	pos.y = ( pos.y | ( pos.y << 2 ) ) & 0x09249249;

	pos.z = ( pos.z | ( pos.z << 16 ) ) & 0x030000FF;
	pos.z = ( pos.z | ( pos.z << 8 ) ) & 0x0300F00F;
	pos.z = ( pos.z | ( pos.z << 4 ) ) & 0x030C30C3;
	pos.z = ( pos.z | ( pos.z << 2 ) ) & 0x09249249;

	return pos.x | ( pos.y << 1 ) | ( pos.z << 2 );
}

uint3 MortonDecode( uint32_t code ) {
	uint3 pos;
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

