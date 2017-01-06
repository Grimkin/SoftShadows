//=================================================================================================
//
//	Query Profiling Sample
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#include "Profiler.h"
#include "GUI.h"
#include "Makros.h"

// == Profiler ====================================================================================

Profiler Profiler::GlobalProfiler;

Profiler::~Profiler() {
	profiles.clear();
}

void Profiler::Initialize( ID3D11Device* device, ID3D11DeviceContext* immContext ) {
	this->device = device;
	this->context = immContext;
}

void Profiler::StartProfile( const std::wstring& name, bool frameTime ) {
	
	ProfileData& profileData = profiles[name];
	_ASSERT( profileData.QueryStarted == FALSE );
	_ASSERT( profileData.QueryFinished == FALSE );

	profileData.FrameTime = frameTime;

	if( frameTime && guiTexts.count( name ) == 0 ) {
		guiTexts[name] = GUIText::Create( L"", guiPosition, 0, 12, Alignment::Left, { 0.f,1.f,0.f,1.f } );
		guiPosition.y += 0.025f;
	}

	if( profileData.DisjointQuery[currFrame] == NULL ) {
		// Create the queries
		D3D11_QUERY_DESC desc;
		desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
		desc.MiscFlags = 0;
		device->CreateQuery( &desc, &profileData.DisjointQuery[currFrame] );

		desc.Query = D3D11_QUERY_TIMESTAMP;
		device->CreateQuery( &desc, &profileData.TimestampStartQuery[currFrame] );
		device->CreateQuery( &desc, &profileData.TimestampEndQuery[currFrame] );
	}

	// Start a disjoint query first
	context->Begin( profileData.DisjointQuery[currFrame] );

	// Insert the start timestamp    
	context->End( profileData.TimestampStartQuery[currFrame] );

	profileData.QueryStarted = TRUE;
}

float Profiler::EndProfile( const std::wstring& name ) {
	ProfileData& profileData = profiles[name];
	_ASSERT( profileData.QueryStarted == TRUE );
	_ASSERT( profileData.QueryFinished == FALSE );

	// Insert the end timestamp    
	context->End( profileData.TimestampEndQuery[currFrame] );

	// End the disjoint query
	context->End( profileData.DisjointQuery[currFrame] );

	profileData.QueryStarted = FALSE;
	profileData.QueryFinished = TRUE;

	if( !profileData.FrameTime )
		return GetProfileTime( profileData );
	
	return 0.f;
}

float Profiler::GetProfileTime( ProfileData& data ) {
	if( data.QueryFinished == FALSE )
		return 0.f;

	data.QueryFinished = FALSE;

	if( data.DisjointQuery[currFrame] == NULL )
		return 0.f;

	// Get the query data
	UINT64 startTime = 0;
	while( context->GetData( data.TimestampStartQuery[currFrame], &startTime, sizeof( startTime ), 0 ) != S_OK );

	UINT64 endTime = 0;
	while( context->GetData( data.TimestampEndQuery[currFrame], &endTime, sizeof( endTime ), 0 ) != S_OK );

	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
	while( context->GetData( data.DisjointQuery[currFrame], &disjointData, sizeof( disjointData ), 0 ) != S_OK );

	float time = 0.0f;
	if( disjointData.Disjoint == FALSE ) {
		UINT64 delta = endTime - startTime;
		float frequency = static_cast<float>( disjointData.Frequency );
		time = ( delta / frequency ) * 1000.0f;
	}

	return time;
}

void Profiler::EndFrame() {
	currFrame = ( currFrame + 1 ) % QueryLatency;

	float queryTime = 0.0f;

	// Iterate over all of the profiles
	for( auto& profile : profiles ) {
		if( profile.second.FrameTime )
			guiTexts[profile.first]->SetText( profile.first + L": " + std::to_wstring( GetProfileTime( profile.second ) ) + L" ms" );
	}
}

// == ProfileBlock ================================================================================

ProfileBlock::ProfileBlock( const std::wstring& name ) : name( name ) {
	Profiler::GlobalProfiler.StartProfile( name );
}

ProfileBlock::~ProfileBlock() {
	Profiler::GlobalProfiler.EndProfile( name );
}

Profiler::ProfileData::~ProfileData() {
	SReleaseArray( DisjointQuery );
	SReleaseArray( TimestampStartQuery );
	SReleaseArray( TimestampEndQuery );
}
