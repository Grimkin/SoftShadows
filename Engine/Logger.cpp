#include "Logger.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cassert>

#include "Game.h"
#include "Application.h"

Logger & Logger::InitMainLogger() {
	static Logger mainLogger;
	Game::SetLogger( mainLogger );
	return mainLogger;
}

Logger::Logger() {
	Init();
	m_StdLogFile = m_LogDir + L"Log.txt";
}

Logger::~Logger() {
}

void Logger::Init() {
	AllocConsole();
	m_ConsoleHandle = GetStdHandle( STD_OUTPUT_HANDLE );
	CreateLogDirectory( m_LogDir );
}

void Logger::Log( const std::wstring & info ) {
	PrintToLogFile( info );
	PrintLine( info );
}

void Logger::Log( const std::wstring & name, const std::wstring & info ) {
	PrintToSeperateFile( name, info );
	PrintLine( name + L": " + info );
}

void Logger::PrintToLogFile( const std::wstring& info ) {
	if( !m_IsLogfileCreated )
		if( !CreateStdLogFile() )
			return;

	m_StdFile << info << std::endl;
}

void Logger::PrintToSeperateFile( const std::wstring& fileName, const std::wstring& info ) {
	auto it = m_OpenLogFiles.find( fileName );
	if( it == m_OpenLogFiles.end() )
		CreateLogFile( fileName );
	it = m_OpenLogFiles.find( fileName );

	it->second << info << std::endl;
}

void Logger::FatalError( const std::wstring& info ) {
	MessageBox( NULL, info.c_str(), L"Fatal Error", MB_OK );
	exit( -1 );
}

void Logger::Print( const std::wstring & info ) {
	std::wstring out = info;
	WriteConsole( m_ConsoleHandle, out.c_str(), static_cast<DWORD>( out.size() ), nullptr, nullptr );
}

void Logger::ResetCursor() {
	Print( L"\r" );
}

void Logger::SetLogDir( const std::wstring & dir ) {
	m_LogDir = dir;
	CreateLogDirectory( m_LogDir );
}

void Logger::CreateLogDirectory( const std::wstring & dir ) {
	// only creates directory if it doesn't already exist
	if( GetFileAttributes( dir.c_str() ) == INVALID_FILE_ATTRIBUTES ) {
		if( !CreateDirectory( dir.c_str(), NULL ) ) {
			FatalError( L"Couldn't create Log directory" );
			return;
		}
	}
}

bool Logger::CreateStdLogFile() {
	if( m_IsLogfileCreated )
		return true;
	std::wofstream file( m_StdLogFile, std::ios::out | std::ios::trunc );
	if( !file.is_open() ) {
		file.close();
		FatalError( L"Failed creating standard log File" );
		return false;
	}
	m_IsLogfileCreated = true;
	m_StdFile.open( m_StdLogFile, std::ios::out | std::ios::trunc );
	return true;
}

bool Logger::CreateLogFile( const std::wstring& fileName ) {
	if( m_OpenLogFiles.find( fileName ) != m_OpenLogFiles.end() )
		return true;

	std::wstring path = m_LogDir + fileName + L".txt";
	std::wofstream file( path, std::ios::out | std::ios::trunc );
	if( !file.is_open() ) {
		file.close();
		FatalError( L"Failed creating log File " + fileName );
		return false;
	}
	file.close();
	m_OpenLogFiles.emplace( fileName, std::wofstream( path, std::ios::out | std::ios::trunc ) );

	return true;
}

void Logger::PrintLine( const std::wstring & info ) {
	Print( info + L"\n" );
}

std::wstring FloatToString( float val, int accuracy ) {
	std::wstringstream stream;
	stream << std::setprecision( accuracy ) << std::fixed << val;
	return stream.str();
}
