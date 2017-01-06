#pragma once
#include <string>
#include <unordered_map>

#include "Types.h"
#include "Logger.h"

bool StringToInt( const std::wstring& input, int& out );
bool StringToFloat( const std::wstring& input, float& out );

class ConfigManager {
public:
	static ConfigManager& Init( const std::wstring& file );

	void ReloadConfig();

	bool GetBool( const std::wstring& name, bool defaultVal = false );
	int GetInt( const std::wstring& name,int defaultVal = 0 );
	int2 GetInt2( const std::wstring& name, int2 defaultVal = { 0, 0 } );
	int3 GetInt3( const std::wstring& name, int3 defaultVal = { 0, 0, 0 } );
	int4 GetInt4( const std::wstring& name, int4 defaultVal = { 0, 0, 0, 0 } );
	float GetFloat( const std::wstring& name, float defaultVal = 0.f );
	float2 GetFloat2( const std::wstring& name, float2 defaultVal = { 0.f, 0.f } );
	float3 GetFloat3( const std::wstring& name, float3 defaultVal = { 0.f, 0.f, 0.f } );
	float4 GetFloat4( const std::wstring& name, float4 defaultVal = { 0.f, 0.f, 0.f, 0.f } );

	const std::wstring& GetString( const std::wstring& name, const std::wstring& defaultVal = L"" );
	

private:
	ConfigManager( const std::wstring& file);
	virtual ~ConfigManager();

	const std::wstring& LookUpNameInMap( std::unordered_map<std::wstring, std::wstring>& map, const std::wstring& name, const std::wstring& defaultVal );

	template<typename T>
	const T& LookUpNameInMap( std::unordered_map<std::wstring, T>& map, const std::wstring& name, const T& defaultVal );

	std::wstring m_File;

	std::unordered_map<std::wstring, bool> m_Bools;
	std::unordered_map<std::wstring, int> m_Ints;
	std::unordered_map<std::wstring, int2> m_Int2s;
	std::unordered_map<std::wstring, int3> m_Int3s;
	std::unordered_map<std::wstring, int4> m_Int4s;
	std::unordered_map<std::wstring, float> m_Floats;
	std::unordered_map<std::wstring, float2> m_Float2s;
	std::unordered_map<std::wstring, float3> m_Float3s;
	std::unordered_map<std::wstring, float4> m_Float4s;
	std::unordered_map<std::wstring, std::wstring> m_Strings;
};

template<typename T>
inline const T & ConfigManager::LookUpNameInMap( std::unordered_map<std::wstring, T>& map, const std::wstring & name, const T & defaultVal ) {
	if( map.count( name ) == 0 ) {
		Game::GetLogger().Log( L"Config", L"Failed to get value for key \"" + name + L"\". Using default value of " + std::to_wstring( defaultVal ) );
		return defaultVal;
	}
	else
		return map[name];
}