#pragma once
#include "Math.h"

#include <algorithm>
#include <set>
#include <map>
#include <functional>

#include "Game.h"
#include "Logger.h"
#include "Time.h"
#include "Distance.h"
#include "emd.h"
#include "Math.h"
#include "Voxelizer.h"
#include "ConfigManager.h"
#include "json.hpp"

struct DebugData {
	DebugData( uint32_t level ) {
		TotalNodes.resize( level );
		IndvNodes.resize( level );
		NumPointers.resize( level - 1 );
	}
	std::vector<uint32_t> TotalNodes;
	std::vector<uint32_t> IndvNodes;
	std::vector<uint32_t> NumPointers;
	uint32_t CompressedLeaves = 0;
	float SortingTime = 0.f;
	float TreeBuildTime = 0.f;
	float ClusteringTime = 0.f;
	float LeaveAddingTime = 0.f;
	float DoubleNodeRemovelTime = 0.f;
};

uint32_t PackPosition( uint3 pos ) {
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

float3 UintToFloat3( uint32_t val ) {
	float3 out;
	out.x = ( val & 1023 ) / 1023.f;
	out.y = ( ( val >> 10 ) & 1023 ) / 1023.f;
	out.z = ( ( val >> 20 ) & 1023 ) / 1023.f;
	return out;
}

uint32_t Float3ToUint( float3 val ) {
	uint32_t x = static_cast<uint32_t>( Min( val.x, 1.f ) * 1023.f );
	uint32_t y = static_cast<uint32_t>( Min( val.y, 1.f ) * 1023.f );
	uint32_t z = static_cast<uint32_t>( Min( val.z, 1.f ) * 1023.f );

	return ( z << 20 ) + ( y << 10 ) + x;
}

uint32_t GetAnisotropicValue( uint2 uiBrick ) {
	float3 value = float3( 0.f, 0.f, 0.f );
	// X-Face
	for( uint32_t i : { 0, 2, 4, 6, 16, 20, 18, 22 } ) {
		if( uiBrick.x & ( 0x303 << i ) )
			value.x += 1.f / 16.f;
		if( uiBrick.y & ( 0x303 << i ) )
			value.x += 1.f / 16.f;
	}
	//Y-Face
	for( uint32_t j : { 0, 1, 4, 5, 8, 9, 12, 13 } ) {
		if( uiBrick.x & ( 0x50005 << j ) )
			value.y += 1.f / 16.f;
		if( uiBrick.y & ( 0x50005 << j ) )
			value.y += 1.f / 16.f;
	}
	// Z-Face
	for( uint32_t k : { 0, 1, 2, 3, 8, 9, 10, 11, 16, 17, 18, 19, 24, 25, 26, 27 } ) {
		if( uiBrick.x & ( 0x11 << k ) || uiBrick.y & ( 0x11 << k ) )
			value.z += 1.f / 16.f;
	}

	return Float3ToUint( value );
}



uint32_t GetInnerApproxVal( const Node& node, uint32_t *approx, const uint32_t *pointer ) {
	float3 approxVal = { 0.f, 0.f, 0.f };
	for( uint32_t z = 0; z < 4; z++ ) {
		for( uint32_t y = 0; y < 4; y++ ) {
			float sum = 0.f;
			for( uint32_t x = 0; x < 4; x++ ) {
				uint32_t mortonPos = MortonEncode( { x,y,z } );

				uint32_t data = mortonPos < 32 ? node.Data.x : node.Data.y;
				uint32_t dataPos = mortonPos < 32 ? mortonPos : mortonPos - 32;
				if( data & ( 1 << dataPos ) ) {
					// offset determined via counting of previous set bits
					uint32_t compVal = 0x7fffffff >> ( 31 - dataPos );
					uint32_t offset = __popcnt( data & compVal );
					if( mortonPos > 31 )
						offset += __popcnt( node.Data.x );

					uint32_t ptr = pointer[node.Pointer + offset];
					if( ptr == -1 )
						sum = 1.f;
					else
						sum = Max( UintToFloat3( approx[ptr] ).x, sum );
				}
			}
			approxVal.x += 1.f / 16.f * Min( sum, 1.f );
		}
	}

	for( uint32_t z = 0; z < 4; z++ ) {
		for( uint32_t x = 0; x < 4; x++ ) {
			float sum = 0.f;
			for( uint32_t y = 0; y < 4; y++ ) {
				uint32_t mortonPos = MortonEncode( { x,y,z } );

				uint32_t data = mortonPos < 32 ? node.Data.x : node.Data.y;
				uint32_t dataPos = mortonPos < 32 ? mortonPos : mortonPos - 32;
				if( data & ( 1 << dataPos ) ) {
					// offset determined via counting of previous set bits
					uint32_t compVal = 0x7fffffff >> ( 31 - dataPos );
					uint32_t offset = __popcnt( data & compVal );
					if( mortonPos > 31 )
						offset += __popcnt( node.Data.x );

					uint32_t ptr = pointer[node.Pointer + offset];
					if( ptr == -1 )
						sum = 1.f;
					else
						sum = Max( UintToFloat3( approx[ptr] ).y, sum );
				}
			}
			approxVal.y += 1.f / 16.f * Min( sum, 1.f );
		}
	}

	for( uint32_t y = 0; y < 4; y++ ) {
		for( uint32_t x = 0; x < 4; x++ ) {
			float sum = 0.f;
			for( uint32_t z = 0; z < 4; z++ ) {
				uint32_t mortonPos = MortonEncode( { x,y,z } );

				uint32_t data = mortonPos < 32 ? node.Data.x : node.Data.y;
				uint32_t dataPos = mortonPos < 32 ? mortonPos : mortonPos - 32;
				if( data & ( 1 << dataPos ) ) {
					// offset determined via counting of previous set bits
					uint32_t compVal = 0x7fffffff >> ( 31 - dataPos );
					uint32_t offset = __popcnt( data & compVal );
					if( mortonPos > 31 )
						offset += __popcnt( node.Data.x );

					uint32_t ptr = pointer[node.Pointer + offset];
					if( ptr == -1 )
						sum = 1.f;
					else
						sum = Max( UintToFloat3( approx[ptr] ).z, sum );
				}
			}
			approxVal.z += 1.f / 16.f * Min( sum, 1.f );
		}
	}
	return Float3ToUint( approxVal );
}

void SortAndOptimize( Node* bricks, uint32_t& numBricks ) {
	// sort the voxel bricks by position
	std::sort( &bricks[0], &bricks[numBricks], []( Node a, Node b )->bool {
		return a.Pointer < b.Pointer;
	} );

	// combine bricks with same position
	Node* lastBrick = std::unique( &bricks[0], &bricks[numBricks], []( Node& a, Node& b )->bool {
		if( a.Pointer == b.Pointer ) {
			a.Data.x = b.Data.x = a.Data.x | b.Data.x;
			a.Data.y = b.Data.y = a.Data.y | b.Data.y;
			return true;
		}
		return false;
	} );

	numBricks = static_cast<uint32_t>( lastBrick - bricks );
}

Node* Traverse( Node* nodes, uint32_t* pointers, uint32_t node, uint32_t level ) {
	Node* current = &nodes[0];
	for( uint32_t curLevel = 1; curLevel < level; curLevel++ ) {
		// the position of the node in the current level
		uint32_t nodePos = ( node >> ( 6 * ( level - curLevel - 1 ) ) ) & 0x3f;

		uint32_t data = nodePos < 32 ? current->Data.x : current->Data.y;
		uint32_t dataPos = nodePos < 32 ? nodePos : nodePos - 32;
		if( data & ( 1 << dataPos ) ) {
			// offset determined via counting of previous set bits
			uint32_t compVal = 0x7fffffff >> ( 31 - dataPos );
			uint32_t offset = __popcnt( data & compVal );
			if( nodePos > 31 )
				offset += __popcnt( current->Data.x );

			uint32_t pointer = pointers[current->Pointer + offset];
			current = &nodes[pointer];
		}
		else {
			return nullptr;
		}
	}
	return current;
}

uint32_t MaxTraverse( Node* nodes, uint32_t* pointers, const uint3& pos, uint32_t startPtr, uint32_t minLevel, uint32_t maxLevel, uint32_t& outLevel ) {
	uint32_t mortonPos = MortonEncode( pos );
	outLevel = maxLevel;
	uint32_t curPtr = startPtr;
	Node* current = &nodes[curPtr];
	for( uint32_t level = minLevel; level <= maxLevel; level++ ) {
		// the position of the node in the current level
		uint32_t nodePos = ( mortonPos >> ( 6 * ( maxLevel - level ) ) ) & 0x3f;

		uint32_t data = nodePos < 32 ? current->Data.x : current->Data.y;
		uint32_t dataPos = nodePos < 32 ? nodePos : nodePos - 32;
		if( data & ( 1 << dataPos ) ) {
			// offset determined via counting of previous set bits
			uint32_t compVal = 0x7fffffff >> ( 31 - dataPos );
			uint32_t offset = __popcnt( data & compVal );
			if( nodePos > 31 )
				offset += __popcnt( current->Data.x );

			curPtr = pointers[current->Pointer + offset];
			current = &nodes[curPtr];
		}
		else {
			outLevel = level - 1;
			return curPtr;
		}
	}
	return curPtr;
}

uint32_t GetNode( Node* nodes, uint32_t* pointers, const uint3& pos, uint32_t maxLevel, uint32_t& outLevel ) {
	outLevel = maxLevel;
	Node* current = &nodes[0];
	uint32_t curPtr = 0;

	if( maxLevel > 5 ) {
		curPtr = MaxTraverse( nodes, pointers, pos >> 10, curPtr, 1, maxLevel - 5, outLevel );
		if( outLevel == maxLevel - 5 )
			return MaxTraverse( nodes, pointers, pos, curPtr, maxLevel - 4, maxLevel, outLevel );
		else
			return curPtr;
	}
	else {
		return MaxTraverse( nodes, pointers, pos, curPtr, 1, maxLevel, outLevel );
	}
}

bool SimilarityTest( uint2 a, uint2 b, uint32_t differing ) {
	static const uint32_t block = 0xff;
	static const uint32_t right = 0x5500;
	static const uint32_t left = 0xaa;
	static const uint32_t up = 0xcc;
	static const uint32_t down = 0x330000;
	static const uint32_t corner = 0x88;
	static const uint32_t botRow = 0x33330000;
	static const uint32_t topRow = 0xcccc;
	static const uint32_t middle = 0x11224488;

	if( a.x == b.x && a.y == b.y )
		return true;

	uint2 difference;
	difference.x = a.x ^ b.x;
	difference.y = a.y ^ b.y;

	uint32_t differences = __popcnt( difference.x ) + __popcnt( difference.y );
	if( differences > differing )
		return false;

	std::vector<uint32_t> setBits;
	for( uint32_t i = 0; i < 32; i++ ) {
		if( difference.x & 1 << i )
			setBits.push_back( i );
		if( difference.y & 1 << i )
			setBits.push_back( i + 32 );
	}

	for( uint32_t bit : setBits ) {
		uint2 aVal, bVal;
		uint32_t idx, secMask;
		if( bit < 32 ) {
			aVal = a;
			bVal = b;
			idx = bit;
			secMask = 0xf0f0f0f;
		}
		else {
			aVal.x = a.y;
			aVal.y = a.x;
			bVal.x = b.y;
			bVal.y = b.x;
			idx = bit - 32;
			secMask = 0xf0f0f0f0;
		}
		uint2 testData;
		// determine if set bit is at a or at b
		if( aVal.x & ( 1 << idx ) ) {
			// set bit is at b, test whether any surrounding bit is set at a
			testData = bVal;
		}
		else {
			// set bit is at a, test whether any surrounding bit is set at b
			testData = aVal;
		}

		uint32_t compVal = 0;
		uint32_t blockIdx = idx >> 3;

		compVal |= block << ( blockIdx * 8 );
		// test if bit is set in block
		if( testData.x & compVal )
			continue;

		// test if bit is set in neighbor in x dir
		if( blockIdx & 1 ) {
			if( !( idx & 1 ) ) {
				if( blockIdx < 2 ) {
					compVal |= left;
					if( testData.x & compVal )
						continue;
				}
				else {
					compVal |= left << 16;
					if( testData.x & compVal )
						continue;
				}
			}
		}
		else if( idx & 1 ) {
			if( blockIdx < 2 ) {
				compVal |= right;
				if( testData.x & compVal )
					continue;
			}
			else {
				compVal |= right << 16;
				if( testData.x & compVal )
					continue;
			}
		}

		uint32_t testVal = 1 << idx;

		// test if bit is set in neighbor in y dir
		if( testVal & topRow ) {
			if( blockIdx == 0 ) {
				compVal |= down;
				if( testData.x & compVal )
					continue;
			}
			else {
				compVal |= down << 8;
				if( testData.x & compVal )
					continue;
			}
		}
		else if( testVal & botRow ) {
			if( blockIdx == 2 ) {
				compVal |= up;
				if( testData.x & compVal )
					continue;
			}
			else {
				compVal |= up << 8;
				if( testData.x & compVal )
					continue;
			}
		}

		// test for values in the middle
		if( testVal & middle ) {
			uint32_t shift = 0;
			if( blockIdx < 2 )
				shift += 14;
			if( !( blockIdx & 1 ) )
				shift += 7;
			compVal |= corner << shift;
			if( testData.x & compVal )
				continue;
		}

		// test neighbors in other block
		// test if testing bit is at border
		if( testVal & ~secMask ) {
			if( testData.y & ( compVal & secMask ) )
				continue;
		}

		return false;
	}

	return true;
}

void AddBrickToHistogramm( uint2 brick, std::array<uint32_t, 64>& histogramm ) {
	for( uint32_t i = 0; i < 32; i++ ) {
		if( brick.x & 1 << i )
			++histogramm[i];
		if( brick.y & 1 << i )
			++histogramm[i + 32];
	}
}

template<typename T>
uint32_t GetNextDiffering( T* vec, uint32_t idx, uint32_t maxIdx, std::function<bool( T&, T& )> comp ) {
	if( idx == maxIdx )
		return maxIdx + 1;
	uint32_t testSize = 1;
	uint32_t testIdx = idx + testSize;
	uint32_t equalIdx = idx;
	while( true ) {
		if( !comp( vec[idx], vec[testIdx] ) ) {
			if( comp( vec[idx], vec[testIdx - 1] ) ) {
				break;
			}
			else {
				testSize /= 2;
				testSize = Max( testSize, 1u );
				testIdx -= testSize;
			}
		}
		else {
			equalIdx = testIdx;
			testSize *= 2;
			testIdx += testSize;
			if( testIdx > maxIdx ) {
				if( comp( vec[maxIdx], vec[idx] ) ) {
					testIdx = maxIdx + 1;
					break;
				}
				testIdx = equalIdx + Max( ( maxIdx - equalIdx ) >> 1, 1u );
				testSize = Max( ( maxIdx - equalIdx ) >> 2, 1u );
			}
		}
	}
	return testIdx;
}

struct StackElement {
	uint3 Position;
	uint32_t Level;
	uint2 Mask;
};

bool HasBitSet( const uint2& brick, uint32_t pos ) {
	uint32_t bitDelta = pos & 0x1F;
	uint32_t bit = 0;
	if( pos < 32 )
		bit = 1 & ( brick.x >> bitDelta );
	else
		bit = 1 & ( brick.y >> bitDelta );

	return bit > 0;
}

void SetBit( uint2& brick, uint32_t pos ) {
	uint32_t bitDelta = pos & 0x1F;
	uint32_t bit = 0;
	if( pos < 32 )
		brick.x |= 1 << bitDelta;
	else
		brick.y |= 1 << bitDelta;
}

void FillNode( const Node& inNode, Node& filledNode, uint3 pos ) {
	uint32_t mortonPos = MortonEncode( pos );
	if( HasBitSet( filledNode.Data, mortonPos ) || HasBitSet( inNode.Data, mortonPos ) )
		return;

	SetBit( filledNode.Data, mortonPos );

	if( pos.x > 0 )
		FillNode( inNode, filledNode, { pos.x - 1, pos.y, pos.z } );
	if( pos.x < 3 )
		FillNode( inNode, filledNode, { pos.x + 1, pos.y, pos.z } );
	if( pos.y > 0 )
		FillNode( inNode, filledNode, { pos.x, pos.y - 1, pos.z } );
	if( pos.y < 3 )
		FillNode( inNode, filledNode, { pos.x, pos.y + 1, pos.z } );
	if( pos.z > 0 )
		FillNode( inNode, filledNode, { pos.x, pos.y, pos.z - 1 } );
	if( pos.z < 3 )
		FillNode( inNode, filledNode, { pos.x, pos.y, pos.z + 1 } );
}

void PushNeighbors( uint3 pos, uint32_t level, std::vector<StackElement>& stack ) {
	const uint2 rightMask = { 0xaa00aa00, 0xaa00aa00 };
	const uint2 leftMask = { 0x550055, 0x550055 };
	const uint2 fwdMask = { 0xcccc0000, 0xcccc0000 };
	const uint2 backMask = { 0x3333, 0x3333 };
	const uint2 topMask = { 0xf0f0f0f, 0 };
	const uint2 botMask = { 0, 0xf0f0f0f0 };

	uint32_t maxValue = ( 1 << ( 2 * ( level + 1 ) ) ) - 1;

	if( pos.x > 0 )
		stack.push_back( { { pos.x - 1, pos.y, pos.z }, level, rightMask } );
	if( pos.x < maxValue )
		stack.push_back( { { pos.x + 1, pos.y, pos.z }, level, leftMask } );
	if( pos.y > 0 )
		stack.push_back( { { pos.x, pos.y - 1, pos.z }, level, fwdMask } );
	if( pos.y < maxValue )
		stack.push_back( { { pos.x, pos.y + 1, pos.z }, level, backMask } );
	if( pos.z > 0 )
		stack.push_back( { { pos.x, pos.y, pos.z - 1 }, level, botMask } );
	if( pos.z < maxValue )
		stack.push_back( { { pos.x, pos.y, pos.z + 1 }, level, topMask } );
}

void SolidifyTree( Node* nodes, uint32_t* pointers, uint32_t numInnerNodes, uint32_t numLeaves, uint32_t maxLevel ) {
	const uint2 rightMask = { 0xaa00aa00, 0xaa00aa00 };
	const uint2 leftMask = { 0x550055, 0x550055 };
	const uint2 fwdMask = { 0xcccc0000, 0xcccc0000 };
	const uint2 backMask = { 0x3333, 0x3333 };
	const uint2 topMask = { 0xf0f0f0f, 0 };
	const uint2 botMask = { 0, 0xf0f0f0f0 };

	std::vector<Node> solidNodes;
	solidNodes.resize( numInnerNodes + numLeaves );
	
	Game::GetLogger().Print( L"Solidifying Tree" );

	float start = Game::GetTime().GetRealTime();

	for( uint32_t i = 0; i < numInnerNodes; ++i ) {
		solidNodes[i].Data = { 0, 0 };
		solidNodes[i].Pointer = 1;
	}
	for( uint32_t i = numInnerNodes; i < numInnerNodes + numLeaves; i++ ) {
		solidNodes[i].Data = { 0, 0 };
		solidNodes[i].Pointer = 0;
	}

	std::vector<StackElement> stack;

	stack.push_back( { { 0,0,0 }, 0, leftMask } );

	size_t maxStackSize = 0;

	while( !stack.empty() ) {
		maxStackSize = Max( maxStackSize, stack.size() );
		uint32_t level = stack.back().Level;
		uint2 mask = stack.back().Mask;
		uint3 pos = stack.back().Position;
		stack.pop_back();

		uint32_t realLevel = level;
		uint32_t curPtr = GetNode( nodes, pointers, pos >> 2, level, realLevel );
		if( realLevel != level ) {
			pos >>= 2 * ( level - realLevel );
		}
		uint32_t inNodePos = MortonEncode( pos ) & 63;
		// push children if any
		if( solidNodes[curPtr].Pointer != 0 && HasBitSet( nodes[curPtr].Data, inNodePos ) ) {
			pos <<= 2;
			for( uint32_t i = 0; i < 32; i++ ) {
				if( mask.x & ( 1 << i ) ) {
					uint3 deltaPos = MortonDecode( i );
					stack.push_back( { pos + deltaPos, level + 1, mask } );
				}
				if( mask.y & ( 1 << i ) ) {
					uint3 deltaPos = MortonDecode( i + 32 );
					stack.push_back( { pos + deltaPos, level + 1, mask } );
				}
			}
		}
		// push the neighbors on the stack if the current part is empty and the bit hasn't been set yet for the solid part
		else if( !HasBitSet( nodes[curPtr].Data, inNodePos ) ) {
			if( !HasBitSet( solidNodes[curPtr].Data, inNodePos ) ) {
				SetBit( solidNodes[curPtr].Data, inNodePos );
				PushNeighbors( pos, realLevel, stack );
			}
		}
	}

	uint32_t ptr = 1;
	uint32_t ptrValue = 1;
	for( size_t i = 0; i < solidNodes.size(); i++ ) {
		if( solidNodes[i].Pointer != 0 )
			nodes[i].Pointer = ptr;
		for( uint32_t j = 0; j < 64; j++ ) {
			if( HasBitSet( nodes[i].Data, j ) ) {
				if( solidNodes[i].Pointer != 0 )
					pointers[ptr++] = ptrValue++;
			}
			else if( !HasBitSet( solidNodes[i].Data, j ) ) {
				SetBit( nodes[i].Data, j );
				if( solidNodes[i].Pointer != 0 )
					pointers[ptr++] = -1;
			}
		}
	}

	float end = Game::GetTime().GetRealTime();

	Game::GetLogger().Log( L"Voxelizer", L"Time needed for solidifying " + std::to_wstring( ( end - start ) * 1000.f ) + L" ms. Max Stack Size " + std::to_wstring( maxStackSize ) );
}

void BuildTree( Node* nodes, uint32_t* pointers, uint32_t &numBricks, uint32_t maxLevel, uint32_t& nodeSize, uint32_t& pointerSize, DebugData& debugData ) {
	uint32_t maxApproxVal = 64;
	float start = Game::GetTime().GetRealTime();
	SortAndOptimize( nodes, numBricks );

	std::vector<Node> bricks;
	bricks.reserve( numBricks );

	for( uint32_t i = 0; i < numBricks; i++ ) {
		bricks.push_back( nodes[i] );
	}

	float end = Game::GetTime().GetRealTime();

	float sortingTime = ( end - start ) * 1000.f;

	Game::GetLogger().Log( L"Voxelizer", L"Time needed for combining and sorting: " + std::to_wstring( sortingTime ) + L" ms" );

	std::vector<uint32_t> levelPointer;

	uint32_t pointer = 0;
	pointers[0] = 0;
	// create root node
	Node* root = &nodes[pointers[0]];
	root->Data.x = 0;
	root->Data.y = 0;
	root->Pointer = ++pointer;

	start = Game::GetTime().GetRealTime();

	for( uint32_t level = 1; level <= maxLevel; level++ ) {
		uint32_t numNodes = 1 << ( 2 * ( level - 1 ) );
		numNodes = numNodes * numNodes * numNodes;
		uint32_t idx = 0;
		levelPointer.push_back( pointer );
		for( uint32_t node = 0; node < numNodes; node++ ) {
			// step through already built tree
			Node* current = Traverse( nodes, pointers, node, level );
			if( !current )
				continue;

			current->Data.x = 0;
			current->Data.y = 0;
			current->Pointer = pointer;

			pointers[pointer] = pointer;

			uint32_t setBits = 0;
			uint32_t nodePos = node * 64;
			// set the bits for occupied nodes
			for( uint32_t i = 0; i < 64; ++i, ++nodePos ) {
				uint32_t brickPos = ( bricks[idx].Pointer >> ( 6 * ( maxLevel - level ) ) );
				while( brickPos < nodePos && idx < numBricks - 1 ) {
					brickPos = ( bricks[++idx].Pointer >> ( 6 * ( maxLevel - level ) ) );
				}
				if( brickPos == nodePos ) {
					if( i < 32 )
						current->Data.x |= 1 << i;
					else
						current->Data.y |= 1 << ( i - 32 );
					++setBits;
					if( idx < numBricks - 1 )
						++idx;
				}
			}

			for( uint32_t i = 1; i < setBits; i++ ) {
				pointers[pointer + i] = pointers[pointer] + i;
			}

			pointer += setBits;
		}
	}

	pointer = levelPointer.back();

	end = Game::GetTime().GetRealTime();

	float treeBuildTime = ( end - start ) * 1000.f;

	Game::GetLogger().Log( L"Voxelizer", L"Time needed for building inner tree: " + std::to_wstring( treeBuildTime ) + L" ms" );

	for( size_t i = 0; i < bricks.size(); ++i ) {
		nodes[pointer + i] = bricks[i];
	}
#ifdef SOFTSHADOW
	SolidifyTree( nodes, pointers, pointer, static_cast<uint32_t>( bricks.size() ), maxLevel );
#endif // SOFTSHADOW

	for( size_t i = 0; i < bricks.size(); ++i ) {
		bricks[i] = nodes[pointer + i];
		uint32_t ptrPos = pointer + uint32_t( i );
		while( pointers[ptrPos] != pointer + i ) {
			ptrPos++;
		}
		bricks[i].Pointer = ptrPos;
	}

	auto sortFunc = []( Node& a, Node& b )->bool {
		uint32_t numBitsA = __popcnt( a.Data.x ) + __popcnt( a.Data.y );
		uint32_t numBitsB = __popcnt( b.Data.x ) + __popcnt( b.Data.y );

		if( numBitsA == numBitsB ) {
			if( a.Data.x == b.Data.x )
				return a.Data.y < b.Data.y;
			return a.Data.x < b.Data.x;
		}
		return numBitsA < numBitsB;
	};

	std::sort( bricks.begin(), bricks.end(), sortFunc );

	std::array<uint32_t, 64> allBricksHistogramm;
	allBricksHistogramm.assign( 0 );
	std::array<uint32_t, 64> individBricksHistogramm;
	individBricksHistogramm.assign( 0 );

	uint2 currentData = bricks[0].Data;
	uint32_t numIndividualLeaves = 1;
	for( auto& brick : bricks ) {
		if( brick.Data.x != currentData.x || brick.Data.y != currentData.y ) {
			++numIndividualLeaves;
			currentData = brick.Data;
			AddBrickToHistogramm( brick.Data, individBricksHistogramm );
		}
		AddBrickToHistogramm( brick.Data, allBricksHistogramm );
	}

	nlohmann::json jsHisto;
	jsHisto["AllBricks"] = allBricksHistogramm;
	jsHisto["IndividualBricks"] = individBricksHistogramm;

	Game::GetLogger().PrintToSeperateFile( L"Histogram", s2ws( jsHisto.dump( 4 ) ) );

	start = Game::GetTime().GetRealTime();

	std::vector<std::pair<uint2, std::vector<Node>>> clusters;

	std::function<bool( const uint2&, const uint2& )> simFunc;
	int sim = Game::GetConfig().GetInt( L"SimilarityTest", 0 );
	float similarity = Game::GetConfig().GetFloat( L"Similarity", .5f );

	if( sim == 0 ) {
		uint32_t numIndividualLeaves = 0;
		uint2 currentData = bricks[0].Data;
		nodes[pointer] = bricks[0];
		for( size_t i = 0; i < bricks.size(); ) {
			uint32_t endIdx = GetNextDiffering<Node>( bricks.data(), static_cast<uint32_t>( i ), static_cast<uint32_t>( bricks.size() - 1 ), [&]( Node& a, Node& b ) {
				return a.Data == b.Data;
			} );
			
			currentData = bricks[i].Data;
			nodes[pointer] = { currentData, 0 };

			for( ; i < endIdx; ++i ) {
				pointers[bricks[i].Pointer] = pointer;
			}
			++pointer;
		}
	}
	else {
		switch( sim ) {
			case 1:
			{
				simFunc = [&]( const uint2& a, const uint2& b ) {
					return a == b || SimilarityTest( a, b, static_cast<uint32_t>( similarity ) );
				};
				break;
			}
			case 2:
			{
				simFunc = [&]( const uint2& a, const uint2& b ) {
					return a == b || emdBrick( a, b ) < similarity;
				};
				break;
			}
			case 3:
			{
				simFunc = [&]( const uint2& a, const uint2& b ) {
					return a == b || BrickDistance( a, b ) < similarity;
				};
				break;
			}
			default:
				break;
		}

		int lastPercent = 0;
		Game::GetLogger().Print( L"Clustering complete: 0 %" );
		for( uint32_t i = 0; i < bricks.size(); ) {
			bool newBrick = true;

			uint32_t endIdx = GetNextDiffering<Node>( bricks.data(), i, static_cast<uint32_t>( bricks.size() - 1 ), []( Node& a, Node&b ) {
				return a.Data == b.Data;
			} );

			for( auto it = clusters.rbegin(); it != clusters.rend(); ++it ) {
				if( simFunc( it->first, bricks[i].Data ) ) {
					for( ; i < endIdx; ++i ) {
						it->second.push_back( bricks[i] );
					}
					newBrick = false;
					break;
				}
			}

			if( newBrick ) {
				clusters.push_back( { bricks[i].Data, {} } );
				for( ; i < endIdx; ++i ) {
					clusters.back().second.push_back( bricks[i] );
				}
			}
			int currentPercent = int( float( i ) / float( bricks.size() ) * 100.f );
			if( currentPercent > lastPercent ) {
				Game::GetLogger().ResetCursor();
				Game::GetLogger().Print( L"Clustering complete: " + std::to_wstring( currentPercent ) + L" %" );
				lastPercent = currentPercent;
			}
		}

		end = Game::GetTime().GetRealTime();

		float clusteringTime = ( end - start ) * 1000.f;

		Game::GetLogger().ResetCursor();
		Game::GetLogger().PrintLine( L"Clustering complete: 100 %" );

		Game::GetLogger().Log( L"Voxelizer", L"Time needed for clustering leaves: " + std::to_wstring( clusteringTime ) + L" ms" );

		Game::GetLogger().Log( L"Voxelizer", L"Number of clusters: " + std::to_wstring( clusters.size() ) );

		bricks.clear();

		start = Game::GetTime().GetRealTime();

		for( auto& cluster : clusters ) {
			uint2 combinedBrick = cluster.first;
			for( auto& brick : cluster.second ) {
				combinedBrick.x |= brick.Data.x;
				combinedBrick.y |= brick.Data.y;

				pointers[brick.Pointer] = pointer;
			}
			nodes[pointer++] = { combinedBrick, 0 };
		}
	}

	end = Game::GetTime().GetRealTime();

	float leaveAddingTime = ( end - start ) * 1000.f;

	float clusteringTime = leaveAddingTime;

	Game::GetLogger().Log( L"Voxelizer", L"Time needed for adding leaves: " + std::to_wstring( leaveAddingTime ) + L" ms" );

	auto equalFunc = []( Node& a, Node& b, const std::vector<uint32_t>& pointers, uint32_t offset )->bool {
		if( a.Data.x == b.Data.x && a.Data.y == b.Data.y ) {
			uint32_t numBits = __popcnt( a.Data.x ) + __popcnt( a.Data.y );
			for( uint32_t i = 0; i < numBits; i++ ) {
				if( pointers[a.Pointer + i - offset] != pointers[b.Pointer + i - offset] )
					return false;
			}
			return true;
		}
		return false;
	};

	struct SortElement {
		SortElement( const Node& node, uint32_t pos ) : Node( node ), OldPos( pos ) {
		}
		Node Node;
		uint32_t OldPos;
	};

	auto sortFunc1 = [&]( SortElement& a, SortElement& b )->bool {
		uint32_t numBitsA = __popcnt( a.Node.Data.x ) + __popcnt( a.Node.Data.y );
		uint32_t numBitsB = __popcnt( b.Node.Data.x ) + __popcnt( b.Node.Data.y );

		if( numBitsA == numBitsB ) {
			if( a.Node.Data.x == b.Node.Data.x ) {
				if( a.Node.Data.y == b.Node.Data.y ) {
					for( uint32_t i = 0; i < numBitsA; i++ ) {
						if( pointers[a.Node.Pointer + i] != pointers[b.Node.Pointer + i] )
							return  pointers[a.Node.Pointer + i] < pointers[b.Node.Pointer + i];
					}
					return false;
				}
				return a.Node.Data.y < b.Node.Data.y;
			}
			return a.Node.Data.x < b.Node.Data.x;
		}
		return numBitsA < numBitsB;
	};

	start = Game::GetTime().GetRealTime();

	std::vector<uint32_t> removedNodes;
	std::vector<uint32_t> nodesAtLevel;

	nodesAtLevel.push_back( numIndividualLeaves );

	std::vector<uint32_t> pointersAtLevel;
	pointersAtLevel.resize( maxLevel );

	std::vector<uint32_t> initialPtrAtLevel;
	initialPtrAtLevel.resize( maxLevel - 1 );

	for( uint32_t i = 1; i < maxLevel; i++ ) {
		uint32_t idx = maxLevel - i;

		uint32_t numNodes = levelPointer[idx] - levelPointer[idx - 1];

		std::vector<SortElement> levelNodes;
		levelNodes.reserve( numNodes );

		for( uint32_t j = levelPointer[idx - 1]; j < levelPointer[idx]; j++ ) {
			uint32_t ptrPos = j;
			while( pointers[ptrPos] != j ) {
				ptrPos++;
			}
			levelNodes.emplace_back( nodes[j], ptrPos );
		}
		uint32_t firstPtr = levelNodes[0].Node.Pointer;
		uint32_t lastPtr = levelNodes.back().Node.Pointer + __popcnt( levelNodes.back().Node.Data.x ) + __popcnt( levelNodes.back().Node.Data.y );

		std::vector<uint32_t> levelPointers;
		levelPointers.reserve( lastPtr - firstPtr );
		for( uint32_t j = firstPtr; j < lastPtr; ++j ) {
			levelPointers.emplace_back( pointers[j] );
		}
		initialPtrAtLevel[maxLevel - i - 1] = lastPtr - firstPtr;

		std::sort( levelNodes.begin(), levelNodes.end(), sortFunc1 );

		uint32_t currentIdx = levelPointer[idx - 1];
		uint32_t pointersPtr = firstPtr;

		for( uint32_t k = 0; k < levelNodes.size(); ) {
			uint32_t endIdx = GetNextDiffering<SortElement>( levelNodes.data(), k, static_cast<uint32_t>( levelNodes.size() - 1 ), [&]( SortElement& a, SortElement& b ) {
				return equalFunc( a.Node, b.Node, levelPointers, firstPtr );
			} );

			SortElement& elem = levelNodes[k];

			uint32_t curPointer = pointersPtr;
			uint32_t numPointers = __popcnt( elem.Node.Data.x ) + __popcnt( elem.Node.Data.y );
			for( uint32_t j = 0; j < numPointers; j++ ) {
				pointers[pointersPtr++] = levelPointers[elem.Node.Pointer - firstPtr + j];
			}
			nodes[currentIdx] = elem.Node;
			nodes[currentIdx].Pointer = curPointer;

			for( ; k < endIdx; ++k ) {
				pointers[levelNodes[k].OldPos] = currentIdx;
			}

			++currentIdx;
		}

		--currentIdx;

		pointersAtLevel[idx] = pointersPtr - firstPtr;

		removedNodes.push_back( numNodes - ( currentIdx - levelPointer[idx - 1] ) - 1 );
		nodesAtLevel.push_back( currentIdx - levelPointer[idx - 1] + 1 );
	}

	pointersAtLevel[0] = nodesAtLevel.back();

	end = Game::GetTime().GetRealTime();

	float doubleNodeRemovalTime = ( end - start ) * 1000.f;

	Game::GetLogger().Log( L"Voxelizer", L"Time needed for removing double nodes: " + std::to_wstring( doubleNodeRemovalTime ) + L" ms" );

	std::vector<std::pair<uint32_t, uint32_t>> pointerPos;

	for( uint32_t i = 0; i < maxLevel - 1; i++ ) {
		uint32_t start = nodes[levelPointer[i]].Pointer;
		Node& endNode = nodes[levelPointer[i] + nodesAtLevel[maxLevel - i - 1] - 1];
		uint32_t end = endNode.Pointer + __popcnt( endNode.Data.x ) + __popcnt( endNode.Data.y );
		pointerPos.push_back( { start,end } );
	}

	// shift pointer value of the pointers
	uint32_t shiftValue = 0;
	for( uint32_t i = 1; i < maxLevel; i++ ) {
		shiftValue += removedNodes[maxLevel - i - 1];
		if( shiftValue > 0 ) {
			uint32_t startPtr = nodes[levelPointer[i - 1]].Pointer;
			Node& endNode = nodes[levelPointer[i - 1] + nodesAtLevel[maxLevel - i] - 1];
			uint32_t endPtr = endNode.Pointer + __popcnt( endNode.Data.x ) + __popcnt( endNode.Data.y ) + 1;
			for( uint32_t j = startPtr; j < endPtr; j++ ) {
				if( pointers[j] != -1 )
					pointers[j] -= shiftValue;
			}
		}
	}

	shiftValue = 0;
	for( uint32_t i = 1; i < maxLevel - 1; i++ ) {
		shiftValue += initialPtrAtLevel[i - 1] - pointersAtLevel[i];
		if( shiftValue > 0 ) {
			uint32_t startPtr = levelPointer[i];
			uint32_t endPtr = levelPointer[i] + nodesAtLevel[maxLevel - i - 1];
			for( uint32_t j = startPtr; j < endPtr; j++ ) {
				nodes[j].Pointer -= shiftValue;
			}
		}
	}

	// shift the nodes to fill the spaces
	uint32_t targetPos = nodesAtLevel[maxLevel - 1] + 1;
	for( uint32_t i = 1; i < maxLevel; i++ ) {
		uint32_t startPos = levelPointer[i];
		uint32_t endPos = levelPointer[i] + nodesAtLevel[maxLevel - i - 1];
		for( uint32_t j = startPos; j < endPos; ++j, ++targetPos ) {
			std::swap( nodes[targetPos], nodes[j] );
		}
	}

	nodeSize = targetPos;

	targetPos = pointerPos[0].first;
	for( uint32_t i = 1; i < maxLevel; i++ ) {
		uint32_t startPos = pointerPos[i - 1].first;
		uint32_t endPos = pointerPos[i - 1].second;
		for( uint32_t j = startPos; j < endPos; ++j, ++targetPos ) {
			std::swap( pointers[targetPos], pointers[j] );
		}
	}

	pointerSize = targetPos;

	std::wstring remNodes = L"Removed Nodes: ";
	for( uint32_t num : removedNodes ) {
		remNodes += std::to_wstring( num ) + L" ";
	}
	Game::GetLogger().Log( L"Voxelizer", remNodes );

	std::wstring numNodes = L"Number of Nodes: ";
	for( uint32_t num : nodesAtLevel ) {
		numNodes += std::to_wstring( num ) + L" ";
	}
	Game::GetLogger().Log( L"Voxelizer", numNodes );

	std::wstring lvlPointer = L"LevelPointer: ";
	for( uint32_t num : levelPointer ) {
		lvlPointer += std::to_wstring( num ) + L" ";
	}
	Game::GetLogger().Log( L"Voxelizer", lvlPointer );

	Game::GetLogger().Log( L"Voxelizer", L"Number of bricks after combining: " + std::to_wstring( numBricks ) );
	Game::GetLogger().Log( L"Voxelizer", L"Number of individual bricks: " + std::to_wstring( numIndividualLeaves ) );

	for( size_t i = 0; i < removedNodes.size(); ++i ) {
		Game::GetLogger().Log( L"Voxelizer", L"Number of removed nodes at level " + std::to_wstring( i + 1 ) + L": " + std::to_wstring( removedNodes[i] ) );
		Game::GetLogger().Log( L"Voxelizer", L"Number of nodes at level " + std::to_wstring( i + 1 ) + L": " + std::to_wstring( nodesAtLevel[i + 1] ) );
		size_t idx = maxLevel - i;
		Game::GetLogger().Log( L"Voxelizer", L"Initial Number of nodes at level: " + std::to_wstring( i + 1 ) + L": " + std::to_wstring( levelPointer[idx - 1] - levelPointer[idx - 2] ) );
	}


	debugData.TotalNodes[0] += 1;
	for( uint32_t i = 0; i < maxLevel - 1; i++ ) {
		debugData.TotalNodes[i + 1] += levelPointer[i + 1] - levelPointer[i];
	}
	debugData.TotalNodes[maxLevel] += numBricks;
	debugData.IndvNodes[0] += 1;
	for( uint32_t i = 0; i < maxLevel; i++ ) {
		debugData.IndvNodes[i + 1] += nodesAtLevel[maxLevel - i - 1];
	}
	for( size_t i = 0; i < pointersAtLevel.size(); i++ ) {
		debugData.NumPointers[i] += pointersAtLevel[i];
	}
	debugData.ClusteringTime += clusteringTime;
	debugData.CompressedLeaves += static_cast<uint32_t>( clusters.size() );
	debugData.DoubleNodeRemovelTime += doubleNodeRemovalTime;
	debugData.LeaveAddingTime += leaveAddingTime;
	debugData.SortingTime += sortingTime;
	debugData.TreeBuildTime += treeBuildTime;
}

void MergeTrees( Node* nodes1, uint32_t* pointers1, Node* nodes2, uint32_t* pointers2, uint32_t maxLevel, std::vector<Node>& nodes, std::vector<uint32_t>& pointers ) {
	std::multimap<uint32_t, uint32_t> freeMap;

	Node root;
	root.Data = nodes1[0].Data | nodes2[0].Data;
	root.Pointer = 1;
	pointers.push_back( 0 );
	nodes.push_back( root );
	uint32_t numPointers = __popcnt( root.Data.x ) + __popcnt( root.Data.y );
	pointers.resize( pointers.size() + numPointers );

	struct Element {
		Node* main, *first, *second;
	};

	std::vector<Element> nextNodes;
	std::vector<Element> curNodes;

	nextNodes.push_back( { &nodes[0], &nodes1[0], &nodes2[0] } );

	for( uint32_t level = 1; level <= maxLevel; ++level ) {
		curNodes.clear();
		std::swap( curNodes, nextNodes );

		for( size_t i = 0; i < curNodes.size(); i++ ) {
			const Node& curNode = *curNodes.back().main;
			const Node* node1 = curNodes.back().first;
			const Node* node2 = curNodes.back().second;
			curNodes.pop_back();

			uint32_t ptr = 0;
			uint32_t ptr1 = 0;
			uint32_t ptr2 = 0;
			for( uint32_t i = 0; i < 32; i++ ) {
				if( node1 && node1->Data.x & 1 << i ) {
					if( node2 && node2->Data.x & 1 << i ) {
						Node* nextNode1 = &nodes1[pointers1[node1->Pointer + ptr1]];
						Node* nextNode2 = &nodes2[pointers2[node2->Pointer + ptr2]];
						Node tempNode;
						tempNode.Data = nextNode1->Data | nextNode2->Data;
						tempNode.Pointer = static_cast<uint32_t>( pointers.size() );
						int32_t numPointers = __popcnt( tempNode.Data.x ) + __popcnt( tempNode.Data.y );
						pointers.resize( pointers.size() + numPointers );
						nodes.push_back( tempNode );
						nextNodes.push_back( { &nodes.back(), nextNode1, nextNode2 } );

						++ptr2;
					}
					else {
						Node* nextNode = &nodes1[pointers1[node1->Pointer + ptr1]];
						Node tempNode;
						tempNode.Data = nextNode->Data;
						pointers[curNode.Pointer + ptr] = static_cast<uint32_t>( nodes.size() );
						tempNode.Pointer = static_cast<uint32_t>( pointers.size() );
						uint32_t numPointers = __popcnt( tempNode.Data.x ) + __popcnt( tempNode.Data.y );
						pointers.resize( pointers.size() + numPointers );
						nodes.push_back( tempNode );
						nextNodes.push_back( { &nodes.back(), nextNode, nullptr } );
					}
					++ptr1;
					++ptr;
				}
				else if( node2 && node2->Data.x & 1 << i ) {
					Node* nextNode = &nodes2[pointers2[node2->Pointer + ptr2]];
					Node tempNode;
					tempNode.Data = nextNode->Data;
					pointers[curNode.Pointer + ptr] = static_cast<uint32_t>( nodes.size() );
					tempNode.Pointer = static_cast<uint32_t>( pointers.size() );
					uint32_t numPointers = __popcnt( tempNode.Data.x ) + __popcnt( tempNode.Data.y );
					pointers.resize( pointers.size() + numPointers );
					nodes.push_back( tempNode );
					nextNodes.push_back( { &nodes.back(), nullptr, nextNode } );

					++ptr2;
					++ptr;
				}
			}
		}
	}
}

#ifdef ANISOTROPIC
void ComputeApproximation( const Node* nodes, uint32_t numNodes, const uint32_t* pointers, uint32_t* approx ) {

	for( uint32_t i = numNodes - 1; i != -1; i-- ) {
		if( nodes[i].Pointer == 0 ) {
			approx[i] = GetAnisotropicValue( nodes[i].Data );
		}
		else {
			approx[i] = GetInnerApproxVal( nodes[i], approx, pointers );
		}
	}
}
#else
void ComputeApproximation( const Node* nodes, uint32_t numNodes, const uint32_t* pointers, float* approx ) {
	for( uint32_t i = numNodes - 1; i != -1; i-- ) {
		if( nodes[i].Pointer == 0 ) {
			approx[i] = Min( __popcnt( nodes[i].Data.x ) + __popcnt( nodes[i].Data.y ), 64u ) / 64.f;
		}
		else {
			uint32_t numChildren = __popcnt( nodes[i].Data.x ) + __popcnt( nodes[i].Data.y );
			float val = 0.f;
			uint32_t ptr = nodes[i].Pointer;
			for( uint32_t j = 0; j < numChildren; j++ ) {
				if( pointers[ptr + j] == -1 )
					val += 1.f;
				else
					val += approx[pointers[ptr + j]];
			}
			approx[i] = val / 64.f;
		}
	}
}
#endif // ANISOTROPIC
