#include "ConfigManager.h"

#include <sstream>
#include <iostream>
#include <fstream>

#include "Game.h"
#include "Logger.h"
#include "Makros.h"

ConfigManager &ConfigManager::Init( const std::wstring& file ) {
	static ConfigManager configManager( file );
	return configManager;
}

ConfigManager::ConfigManager( const std::wstring& file )
	: m_File( file ) {
	Game::SetConfig( *this );
	ReloadConfig();
}


ConfigManager::~ConfigManager() {
}

const std::wstring & ConfigManager::LookUpNameInMap( std::unordered_map<std::wstring, std::wstring>& map, const std::wstring & name, const std::wstring & defaultVal ) {
	if( map.count( name ) == 0 ) {
		Game::GetLogger().Log( L"Config", L"Failed to get value for key \"" + name + L"\". Using default value of " + defaultVal );
		return defaultVal;
	}
	else
		return map[name];
}

void ConfigManager::ReloadConfig() {
	std::wifstream file( m_File, std::ios::in );

	if( !file.is_open() ) {
		Game::GetLogger().Log( L"Config", L" Couldn't open config File \"" + m_File + L"\"." );
		return;
	}

	int lineNumber = 0;
	while( !file.eof() ) {
		++lineNumber;

		wchar_t input[100];
		file.getline( input, 100 );

		std::wstring line( input );
		// return if empty line or comment
		if( line.size() == 0 || line[0] == L'#' )
			continue;

		std::vector<std::wstring> splitLine = SplitString( line, L" ", true );

		if( splitLine[0] == L"Bool" ) {
			if( splitLine.size() >= 3 ) {
				if( splitLine[2] == L"false" || splitLine[2] == L"False" || splitLine[2] == L"FALSE" || splitLine[2] == L"0" ) {
					m_Bools.emplace( splitLine[1], false );
				}
				else if( splitLine[2] == L"true" || splitLine[2] == L"True" || splitLine[2] == L"TRUE" || splitLine[2] == L"1" ) {
					m_Bools.emplace( splitLine[1], true );
				}
				else {
					Game::GetLogger().Log( L"Config", L"Error in line " + std::to_wstring( lineNumber ) + L" for Bool type. " + 
						L"Unrecognized value: " + splitLine[2] );
				}
			}
			else {
				Game::GetLogger().Log( L"Config", L"Error in line " + std::to_wstring( lineNumber ) + L" for Bool type." );
				continue;
			}
		}
		else if( splitLine[0] == L"Int" ) {
			if( splitLine.size() >= 3 ) {
				int val;
				if( StringToInt( splitLine[2], val ) ) {
					m_Ints.emplace( splitLine[1], val );
					continue;
				}
			}
			Game::GetLogger().Log( L"Config", L"Error in line " + std::to_wstring( lineNumber ) + L" for Int type." );
		}
		else if( splitLine[0] == L"Int2" ) {
			if( splitLine.size() >= 4 ) {
				int2 val;
				if( StringToInt( splitLine[2], val.x ) && StringToInt( splitLine[3], val.y ) ) {
					m_Int2s.emplace( splitLine[1], val );
					continue;
				}
			}
			Game::GetLogger().Log( L"Config", L"Error in line " + std::to_wstring( lineNumber ) + L" for Int2 type." );
		}
		else if( splitLine[0] == L"Int3" ) {
			if( splitLine.size() >= 5 ) {
				int3 val;
				if( StringToInt( splitLine[2], val.x ) && StringToInt( splitLine[3], val.y ) &&	StringToInt( splitLine[4], val.z ) ) {
					m_Int3s.emplace( splitLine[1], val );
					continue;
				}
			}
			Game::GetLogger().Log( L"Config", L"Error in line " + std::to_wstring( lineNumber ) + L" for Int3 type." );
		}
		else if( splitLine[0] == L"Int4" ) {
			if( splitLine.size() >= 6 ) {
				int4 val;
				if( StringToInt( splitLine[2], val.x ) && StringToInt( splitLine[3], val.y ) && 
					StringToInt( splitLine[4], val.z ) && StringToInt( splitLine[5], val.w ) ) {
					m_Int4s.emplace( splitLine[1], val );
					continue;
				}
			}
			Game::GetLogger().Log( L"Config", L"Error in line " + std::to_wstring( lineNumber ) + L" for Int4 type." );
		}
		else if( splitLine[0] == L"Float" ) {
			if( splitLine.size() >= 3 ) {
				float val;
				if( StringToFloat( splitLine[2], val ) ) {
					m_Floats.emplace( splitLine[1], val );
					continue;
				}
			}
			Game::GetLogger().Log( L"Config", L"Error in line " + std::to_wstring( lineNumber ) + L" for Float type." );
		}
		else if( splitLine[0] == L"Float2" ) {
			if( splitLine.size() >= 4 ) {
				float2 val;
				if( StringToFloat( splitLine[2], val.x ) && StringToFloat( splitLine[3], val.y ) ) {
					m_Float2s.emplace( splitLine[1], val );
					continue;
				}
			}
			Game::GetLogger().Log( L"Config", L"Error in line " + std::to_wstring( lineNumber ) + L" for Float2 type." );
		}
		else if( splitLine[0] == L"Float3" ) {
			if( splitLine.size() >= 5 ) {
				float3 val;
				if( StringToFloat( splitLine[2], val.x ) && StringToFloat( splitLine[3], val.y ) &&	StringToFloat( splitLine[4], val.z ) ) {
					m_Float3s.emplace( splitLine[1], val );
					continue;
				}
			}
			Game::GetLogger().Log( L"Config", L"Error in line " + std::to_wstring( lineNumber ) + L" for Float3 type." );
		}
		else if( splitLine[0] == L"Float4" ) {
			if( splitLine.size() >= 6 ) {
				float4 val;
				if( StringToFloat( splitLine[2], val.x ) && StringToFloat( splitLine[3], val.y ) &&
					StringToFloat( splitLine[4], val.z ) && StringToFloat( splitLine[5], val.w ) ) {
					m_Float4s.emplace( splitLine[1], val );
					continue;
				}
			}
			Game::GetLogger().Log( L"Config", L"Error in line " + std::to_wstring( lineNumber ) + L" for Float4 type." );
		}
		else if( splitLine[0] == L"String" ) {
			if( splitLine.size() >= 3 ) {
				m_Strings.emplace( splitLine[1], splitLine[2] );
			}
			else {
				Game::GetLogger().Log( L"Config", L"Error in line " + std::to_wstring( lineNumber ) + L" for String type." );
				continue;
			}
		}
		else {
			Game::GetLogger().Log( L"Config", L"Error in line " + std::to_wstring( lineNumber ) + L". No type recognized or comment." );
			continue;
		}
	}

}

bool ConfigManager::GetBool( const std::wstring & name, bool defaultVal ) {
	return LookUpNameInMap( m_Bools, name, defaultVal );
}

int ConfigManager::GetInt( const std::wstring & name, int defaultVal ) {
	return LookUpNameInMap( m_Ints, name, defaultVal );
}

int2 ConfigManager::GetInt2( const std::wstring & name, int2 defaultVal ) {
	return LookUpNameInMap( m_Int2s, name, defaultVal );
}

int3 ConfigManager::GetInt3( const std::wstring & name, int3 defaultVal ) {
	return LookUpNameInMap( m_Int3s, name, defaultVal );
}

int4 ConfigManager::GetInt4( const std::wstring & name, int4 defaultVal ) {
	return LookUpNameInMap( m_Int4s, name, defaultVal );
}

float ConfigManager::GetFloat( const std::wstring & name, float defaultVal ) {
	return LookUpNameInMap( m_Floats, name, defaultVal );
}

float2 ConfigManager::GetFloat2( const std::wstring & name, float2 defaultVal ) {
	return LookUpNameInMap( m_Float2s, name, defaultVal );
}

float3 ConfigManager::GetFloat3( const std::wstring & name, float3 defaultVal ) {
	return LookUpNameInMap( m_Float3s, name, defaultVal );
}

float4 ConfigManager::GetFloat4( const std::wstring & name, float4 defaultVal ) {
	return LookUpNameInMap( m_Float4s, name, defaultVal );
}

const std::wstring & ConfigManager::GetString( const std::wstring & name, const std::wstring & defaultVal ) {
	return LookUpNameInMap( m_Strings, name, defaultVal );
}

bool StringToInt( const std::wstring & input, int & out ) {
	try {
		out = std::stoi( input, nullptr, 0 );
	}
	catch( const std::invalid_argument& ) {
		return false;
	}
	return true;
}

bool StringToFloat( const std::wstring & input, float & out ) {
	try {
		out = std::stof( input, nullptr );
	}
	catch( const std::invalid_argument& ) {
		return false;
	}
	return true;
}
