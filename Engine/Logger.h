#pragma once
#include <string>
#include <unordered_map>
#include <Windows.h>
#include <fstream>

#include "Types.h"

class Logger
{
	friend class Game;
public:
	static Logger& InitMainLogger();

	Logger();
	virtual ~Logger();

	void Init();

	// print on console and log to file
	void Log( const std::wstring& info );
	// print on console with name as identifier and to log to file 'name'.txt
	void Log( const std::wstring& name, const std::wstring& info );
	// print in log file 'SystemPart'.txt
	void PrintToLogFile( const std::wstring& info );
	// print in log file 'filename'.txt
	void PrintToSeperateFile( const std::wstring& fileName, const std::wstring& info );

	template<typename T>
	bool SaveToFile( const std::wstring& fileName, const T& object );
	template<typename T>
	bool LoadFromFile( const std::wstring& fileName, T* object );

	bool CreateStdLogFile();
	bool CreateLogFile( const std::wstring& fileName );

	void PrintLine( const std::wstring& info );
	void FatalError( const std::wstring& info );
	void Print( const std::wstring& info );
	void ResetCursor();
	void SetLogDir( const std::wstring& dir );
private:
	void CreateLogDirectory( const std::wstring& dir );

	std::wstring m_StdLogFile;

	std::unordered_map<std::wstring, std::wofstream> m_OpenLogFiles;
	std::wofstream m_StdFile;

	bool m_IsLogfileCreated;

	HANDLE m_ConsoleHandle;
	std::wstring m_LogDir = L"./Log/";
};

namespace std {
	inline wstring to_wstring( float2 vec ) {
		return to_wstring( vec.x ) + L" " + to_wstring( vec.y );
	}
	inline wstring to_wstring( float3 vec ) {
		return to_wstring( vec.x ) + L" " + to_wstring( vec.y ) + L" " + to_wstring( vec.z );
	}
	inline wstring to_wstring( float4 vec ) {
		return to_wstring( vec.x ) + L" " + to_wstring( vec.y ) + L" " + to_wstring( vec.z ) + L" " + to_wstring( vec.w );
	}
	inline wstring to_wstring( int2 vec ) {
		return to_wstring( vec.x ) + L" " + to_wstring( vec.y );
	}
	inline wstring to_wstring( int3 vec ) {
		return to_wstring( vec.x ) + L" " + to_wstring( vec.y ) + L" " + to_wstring( vec.z );
	}
	inline wstring to_wstring( int4 vec ) {
		return to_wstring( vec.x ) + L" " + to_wstring( vec.y ) + L" " + to_wstring( vec.z ) + L" " + to_wstring( vec.w );
	}
}

std::wstring FloatToString( float val, int accuracy );

template<typename T>
inline bool Logger::SaveToFile( const std::wstring & fileName, const T& object ) {
	std::ofstream file( fileName, std::ios::out | std::ios::trunc | std::ios::binary );
	if( !file.is_open() ) {
		Log( L"Logger", L"Saving to file \"" + fileName + L"\" failed" );
		return false;
	}

	file.write( reinterpret_cast<const char*>( &object ), sizeof( T ) );

	return true;
}

template<typename T>
inline bool Logger::LoadFromFile( const std::wstring & fileName, T * object ) {
	std::ifstream file( fileName, std::ios::in | std::ios::binary );
	if( !file.is_open() ) {
		Log( L"Logger", L"Loading file \"" + fileName + L"\" failed" );
		return false;
	}
	
	file.read( reinterpret_cast<char*>( object ), sizeof( T ) );

	if( !file ) {
		Log( L"Logger", L"Loading file \"" + fileName + L"\" failed" );
		return false;
	}

	return true;
}
