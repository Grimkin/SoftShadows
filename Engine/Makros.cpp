#include "Makros.h"

void Update2DArray( void * array, uint2 arraySize, void * newData, uint2 dataPos, uint2 dataSize ) {
	char* cArray = reinterpret_cast<char*>( array );
	char* cData = reinterpret_cast<char*>( newData );
	size_t xEnd = std::min( dataPos.x + dataSize.x, arraySize.x );
	size_t yEnd = std::min( dataPos.y + dataSize.y, arraySize.y );
	size_t dataY = 0;
	for( size_t y = dataPos.y; y < yEnd; ++y,++dataY ) {
		size_t dataX = 0;
		for( size_t x = dataPos.x; x < xEnd; ++x, ++dataX ) {
			cArray[x + y * arraySize.x] = cData[dataX + dataY * dataSize.x];
		}
	}
}

std::vector<std::wstring> SplitString( const std::wstring & str, const std::wstring & delimiter, bool trimEmpty ) {
	std::string::size_type pos, lastPos = 0;

	std::vector<std::wstring> tokens;

	while( true ) {
		pos = str.find( delimiter, lastPos );
		if( pos == std::string::npos ) {
			pos = str.size();

			if( pos != lastPos || !trimEmpty )
				tokens.push_back( std::wstring( str.data() + lastPos, pos - lastPos ) );

			break;
		}
		else {
			if( pos != lastPos || !trimEmpty )
				tokens.push_back( std::wstring( str.data() + lastPos, pos - lastPos ) );
		}

		lastPos = pos + 1;
	}
	return tokens;
}
