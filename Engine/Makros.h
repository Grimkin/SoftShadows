#pragma once

#include <cassert>
#include <functional>
#include <vector>
#include <codecvt>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <array>

#include "Types.h"

#undef max
#undef min

#define SDELETE(p) {if(p){delete p; p = nullptr;}}



template<typename T>
inline void SRelease( T*& p ) {
	if( p ) {
		p->Release();
		p = nullptr;
	}
}

template<typename T>
inline void SRelease( T& p ) {
	p.Release();
}

template<typename T>
inline void SReleaseVec( T& vec ) {
	for( auto elem : vec ) {
		SRelease( elem );
	}
	vec.clear();
}

template<typename T,int size>
inline void SReleaseArray( std::array<T, size> array ) {
	for( auto elem : array )
		SRelease( elem );
}

template<typename K, typename T>
inline void SReleaseMap( T& vec ) {
	for( auto elem : vec ) {
		SRelease( elem.second );
	}
	vec.clear();
}

template<typename T>
inline void SDeleteVec( T& vec ) {
	for( auto elem : vec ) {
		delete elem;
	}
	vec.clear();
}

template<typename T>
inline void SDeleteVec( std::vector<T*>& vec, void( *Deleter )( T* ) ) {
	for( auto elem : vec ) {
		Deleter( elem );
	}
	vec.clear();
}

template<typename H, typename T>
inline void SDeleteMap( std::map<H,T*>& map ) {
	for( auto elem : map ) {
		delete elem.second;
	}
	map.clear();
}

template<typename H, typename T>
inline void SDeleteMap( std::unordered_map<H, T*>& map ) {
	for( auto elem : map ) {
		delete elem.second;
	}
	map.clear();
}

template<typename H, typename T>
inline void SDeleteMap( std::multimap<H, T*>& map ) {
	for( auto elem : map ) {
		delete elem.second;
	}
	map.clear();
}

template<typename H, typename T>
inline void SDeleteMap( std::unordered_multimap<H, T*>& map ) {
	for( auto elem : map ) {
		delete elem.second;
	}
	map.clear();
}

template<typename H, typename T>
inline void SDeleteMap( std::map<H, T*>& map, void( *Deleter )( T* ) ) {
	for( auto elem : map ) {
		Deleter( elem.second );
	}
	map.clear();
}
template<typename H, typename T>
inline void SDeleteMap( std::unordered_map<H, T*>& map, void( *Deleter )( T* ) ) {
	for( auto elem : map ) {
		Deleter( elem.second );
	}
	map.clear();
}
template<typename H, typename T>
inline void SDeleteMap( std::multimap<H, T*>& map, void( *Deleter )( T* ) ) {
	for( auto elem : map ) {
		Deleter( elem.second );
	}
	map.clear();
}

template<typename H, typename T>
inline void SDeleteMap( std::unordered_multimap<H, T*>& map, void( *Deleter )( T* ) ) {
	for( auto elem : map ) {
		Deleter( elem.second );
	}
	map.clear();
}

template<typename T>
inline T Min( T a, T b ) {
	return a < b ? a : b;
}

template<typename T>
inline T Max( T a, T b ) {
	return a > b ? a : b;
}

template<typename T>
inline T clamp( T val, T min, T max ) {
	return val < min ? min : ( val > max ? max : val );
}

inline float Round( float val, float accuracy = 1.0f ) {
	if( accuracy == 1.0f ) {
		return std::round( val );
	}
	return std::round( val / accuracy ) * accuracy;
}

template<typename T>
inline bool CheckFlag( T a, T b ) {
	typedef std::underlying_type<WriteEnable>::type enum_type;
	return ( static_cast<enum_type>( a ) & static_cast<enum_type>( b ) ) != 0;
}

inline std::wstring s2ws( const std::string& str ) {
	typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes( str );
}

inline std::string ws2s( const std::wstring& wstr ) {
	typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes( wstr );
}

void Update2DArray( void* array, uint2 arraySize, void* newData, uint2 dataPos, uint2 dataSize );

std::vector<std::wstring> SplitString( const std::wstring& str, const std::wstring& delimiter = L" ", bool trimEmpty = false );
