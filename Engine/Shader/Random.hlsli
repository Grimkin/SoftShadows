#define RANDOM_IA 16807
#define RANDOM_IM 2147483647
#define RANDOM_AM (1.0f/float(0xffffffff))
#define RANDOM_IQ 127773u
#define RANDOM_IR 2836
#define RANDOM_MASK 123459876

#define TEA_ROUNDS 4

struct NumberGenerator {
	uint2 seed; // Used to generate values.

			  // Returns the current random float.
	float GetCurrentFloat() {
		Cycle();
		return RANDOM_AM * seed.y;
	}

	// Returns the current random int.
	int GetCurrentInt() {
		Cycle();
		return seed.y;
	}

	// Generates the next number in the sequence.
	void Cycle() {
		uint2 v = seed;
		uint s = 0x9E3779B9u;
		for( int i = 0; i < TEA_ROUNDS; ++i ) {
			v.x += ( ( v.y << 4u ) + 0xA341316Cu ) ^ ( v.y + s ) ^ ( ( v.y >> 5u ) + 0xC8013EA4u );
			v.y += ( ( v.x << 4u ) + 0xAD90777Du ) ^ ( v.x + s ) ^ ( ( v.x >> 5u ) + 0x7E95761Eu );
			s += 0x9E3779B9u;
		}
		seed = v;
		/*
		seed ^= RANDOM_MASK;
		int k = seed / RANDOM_IQ;
		seed = RANDOM_IA * ( seed - k * RANDOM_IQ ) - RANDOM_IR * k;

		if( seed < 0 )
			seed += RANDOM_IM;

		seed ^= RANDOM_MASK;
		*/
	}

	// Cycles the generator based on the input count. Useful for generating a thread unique seed.
	// PERFORMANCE - O(N)
	void Cycle( const uint _count ) {
		for( uint i = 0; i < _count; ++i )
			Cycle();
	}

	// Returns a random float within the input range.
	float GetRandomFloat( const float low, const float high ) {
		float v = GetCurrentFloat();
		return low * ( 1.0f - v ) + high * v;
	}

	// Sets the seed
	void SetSeed( const uint2 value ) {
		seed = value;
		Cycle();
	}
};

// TEA random generator taken from https://umbcgaim.wordpress.com/2010/07/01/gpu-random-numbers/



uint2 RandTEA( uint stream, uint sequence ) {
	uint2 v = uint2( stream, sequence );
	uint s = 0x9E3779B9u;
	for( int i = 0; i < TEA_ROUNDS; ++i ) {
		v.x += ( ( v.y << 4u ) + 0xA341316Cu ) ^ ( v.y + s ) ^ ( ( v.y >> 5u ) + 0xC8013EA4u );
		v.y += ( ( v.x << 4u ) + 0xAD90777Du ) ^ ( v.x + s ) ^ ( ( v.x >> 5u ) + 0x7E95761Eu );
		s += 0x9E3779B9u;
	}
	return v;
}

float RandTEA_float( uint stream, uint sequence ) {
	uint2 v = RandTEA( stream, sequence );
	return float( v.x ) / 0xffffffff;
}
