//=================================================================================================
//
//	Query Profiling Sample
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#pragma once

#include <d3d11.h>
#include <string>
#include <map>
#include <array>

#include "Types.h"

class GUIText;

class Profiler {

public:

	static Profiler GlobalProfiler;

	~Profiler();

	void Initialize( ID3D11Device* device, ID3D11DeviceContext* immContext );

	void StartProfile( const std::wstring& name, bool frameTime = true );
	float EndProfile( const std::wstring& name );

	void EndFrame();

protected:

	// Constants
	static const size_t QueryLatency = 5;

	struct ProfileData {
		std::array<ID3D11Query*, QueryLatency> DisjointQuery = { nullptr };
		std::array<ID3D11Query*, QueryLatency> TimestampStartQuery = { nullptr };
		std::array<ID3D11Query*, QueryLatency> TimestampEndQuery = { nullptr };
		BOOL QueryStarted;
		BOOL QueryFinished;
		bool FrameTime;

		ProfileData() : QueryStarted( FALSE ), QueryFinished( FALSE ) {
		}

		~ProfileData();
	};

	float GetProfileTime( ProfileData& data );

	typedef std::map<std::wstring, ProfileData> ProfileMap;

	ProfileMap profiles;

	std::map<std::wstring, GUIText*> guiTexts;
	float2 guiPosition = { 0.005f, 0.1f };

	size_t currFrame;

	ID3D11Device* device;
	ID3D11DeviceContext* context;
};

class ProfileBlock {
public:

	ProfileBlock( const std::wstring& name );
	~ProfileBlock();

protected:

	std::wstring name;
};
