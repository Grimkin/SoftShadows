#include "Voxelizer.h"
//#define CPUVOXEL
#ifdef CPUVOXEL
#include "VoxelizeTest_Impl.h"
#endif

#include "Shader\VoxelDefines.hlsli"

#include "Game.h"
#include "RenderBackend.h"
#include "D3DWrapper.h"
#include "Logger.h"
#include "Makros.h"
#include "RenderPass.h"
#include "Shader.h"
#include "Camera.h"
#include "GameObjectManager.h"
#include "Texture.h"
#include "Renderer.h"
#include "Time.h"
#include "Slider.h"
#include "Profiler.h"
#include "imgui.h"
#include "Morton.h"
#include "FileLoader.h"
#include "Window.h"

#include "TreeBuild_Impl.h"

//#define PREVOXELIZE

float GetHaltonNumber( int index, int base ) {
	float result = 0.f;
	float f = 1.f;
	int i = index;
	while( i > 0 ) {
		f = f / base;
		result = result + f * ( i % base );
		i = i / base;
	}
	return result;
}

Voxelizer::Voxelizer( const float3& position, const float3& size, const uint32_t resolution )
	: m_ResolutionMultiplier( resolution > 4096 ? resolution / 4096 : 1 )
	, m_Width( resolution / m_ResolutionMultiplier )
	, m_Height( resolution / m_ResolutionMultiplier )
	, m_Depth( resolution / m_ResolutionMultiplier )
	, m_Position( position )
	, m_Size( size ) {
	if( !CreateBuffers() )
		return;

	if( !CreateRenderPass() )
		return;

	if( !CreateCamera() )
		return;

	VoxelizeData data;
	data.gridSize = { m_Width, m_Height, m_Depth };
	data.numBits = { 4, 4, 4 };
	data.numTexels = { m_Width / data.numBits.x, m_Height / data.numBits.y, m_Depth / data.numBits.z };
	data.deltaGrid = { size.x / static_cast<float>( m_Width ),size.y / static_cast<float>( m_Height ), size.z / static_cast<float>( m_Depth ) };
	data.invDeltaGrid = { 1.0f / data.deltaGrid.x, 1.0f / data.deltaGrid.y, 1.0f / data.deltaGrid.z };
	data.minBoxPos = make_float3a( position - 0.5f * size );
	data.boxSize = make_float3a( size );

	m_VoxelizeDataBuffer.Update( data );

	GridData gridData;
	gridData.GridResolution = { m_Width / 4, m_Height / 4, m_Depth / 4 };
	gridData.TreeSize = static_cast<uint32_t>( ceil( log2( m_Width ) / 2.f ) - 1 );
	gridData.MinGridPosition = data.minBoxPos;
	gridData.GridSize = make_float3a( size );

	m_GridDataBuffer.Update( gridData );

	//m_VerticalLightSlider = Slider::Create( L"Vertical Light Angle", { 0.15f, 0.8f }, 1.5f, 0.f, 90.f, 1.f, 45.f, 0, L"degree" );
	//m_HorizontalLightSlider = Slider::Create( L"Horizontal Light Angle", { 0.15f, 0.85f }, 1.5f, 0.f, 360.f, 1.f, 315.f, 0, L"degree" );
	//m_SampleSlider = Slider::Create( L"LightSamples", { 0.15f, 0.9f }, 1.5f, 1.f, 10.f, 1.f, 4.f, 0, L"^2 Sample(s)" );
	//m_AngleSlider = Slider::Create( L"OpeningAngle", { 0.15f, 0.95f }, 1.5f, 0.f, 20.f, 0.25f, 4.f, 2, L"degree" );

	std::srand( uint32_t( time( 0 ) ) );
}


Voxelizer::~Voxelizer() {
	Game::GetRenderBackend().UnRegisterCallBacks( this );

	SRelease( m_TreeSRV );
	SRelease( m_TreeUAV );
	SRelease( m_BrickSRV );
	SRelease( m_BrickUAV );
	SRelease( m_CountUAV );
	SRelease( m_PointerSRV );
	SRelease( m_PointerUAV );
	SRelease( m_ApproxSRV );
}

void Voxelizer::VoxelizeObject( GameObject& gameObject ) {
#ifdef PREVOXELIZE
	const Geometry& geometry = *gameObject.GetRenderable()->GetGeometry();
	const Matrix& transform = gameObject.GetTransform().GetWorldTransMat();

	RenderBackend* renderBackend = &Game::GetRenderBackend();

	uint32_t clearVal[4] = { 0,0,0,0 };
	renderBackend->ClearUAV( m_TreeUAV, clearVal );
	renderBackend->ClearUAV( m_CountUAV, clearVal );

	Profiler::GlobalProfiler.StartProfile( L"Voxelize", false );

	m_VoxelShader->SetShader();
	m_VoxelizeDataBuffer.Bind( ShaderFlag::ComputeShader, 0 );
	m_ObjectDataBuffer.Bind( ShaderFlag::ComputeShader, 1 );

	VoxelObjectData objData;
	objData.numTriangles = geometry.GetTriangleCount();
	objData.worldMat = transform;
	m_ObjectDataBuffer.Update( objData );

	renderBackend->SetUAVCS( 1, { m_TreeUAV, m_CountUAV }, { 0 } );
	renderBackend->SetSRVsCS( 0, { geometry.GetPositionSRV(), geometry.GetIndexSRV() } );

	uint32_t numTriangles = geometry.GetTriangleCount();
	uint32_t groupSize = 128;
	uint32_t numGroups = numTriangles / groupSize;
	if( numTriangles & ( groupSize - 1 ) )
		numGroups++;

	renderBackend->Dispatch( numGroups, 1, 1 );
	renderBackend->SetUAVCS( 1, { nullptr, nullptr }, { 0 } );

	float time = Profiler::GlobalProfiler.EndProfile( L"Voxelize" );

	Game::GetLogger().Log( L"Voxelizer", L"Voxelization Time: " + std::to_wstring( time ) + L" ms" );

	uint32_t numBricks;
	renderBackend->ReadBuffer( m_CountBuffer, sizeof( uint32_t ), &numBricks, sizeof( uint32_t ), 0 );

	Game::GetLogger().Log( L"Voxelizer", L"Number of Brick voxelized: " + std::to_wstring( numBricks ) );

	if( numBricks > m_NumTreeNodes )
		Game::GetLogger().FatalError( L"Not enough space for voxelization" );

	Node* nodes = nullptr;

	ID3D11DeviceContext *context = &Game::GetContext();
	ID3D11Device* device = &Game::GetDevice();

	BufferDesc desc;
	desc.ByteWidth = sizeof( Node ) * m_NumTreeNodes;
	desc.CPUAccessFlags = CPUAccessFlag::Read | CPUAccessFlag::Write;
	desc.Usage = Usage::Staging;

	Buffer* tempNodeBuffer;
	device->CreateBuffer( reinterpret_cast<D3D11_BUFFER_DESC*>( &desc ), nullptr, &tempNodeBuffer );

	desc.ByteWidth = sizeof( uint32_t ) * m_NumTreeNodes;

	std::vector<uint32_t> data;
	data.resize( m_NumTreeNodes );

	renderBackend->CopyResource( tempNodeBuffer, m_TreeBuffer );

	renderBackend->MapBuffer( tempNodeBuffer, reinterpret_cast<void**>( &nodes ), 0, MapType::ReadWrite );

	SortAndOptimize( nodes, numBricks );

	m_VoxelizedObjects[&gameObject] = {};
	m_VoxelizedObjects[&gameObject].assign( nodes, &nodes[numBricks] - 1 );

	renderBackend->UnmapBuffer( tempNodeBuffer, 0 );

	renderBackend->CopyResource( m_TreeBuffer, tempNodeBuffer );

	tempNodeBuffer->Release();
#endif
}

VoxelGrid * Voxelizer::Voxelize( const std::vector<std::pair<const Geometry*, Matrix>>& elements ) {
	if( Game::GetConfig().GetBool( L"LoadTree", false ) ) {
		std::wstring path = Game::GetConfig().GetString( L"TreeLoadPath", L"TestTree.vx" );
		if( LoadTree( path ) )
			return nullptr;
	}

	bool loadVoxelization = Game::GetConfig().GetBool( L"LoadVoxelization", false );

	std::vector<std::vector<Node>> voxelizationParts;

	bool storeVoxelization = false;
	if( loadVoxelization ) {
		std::wstring path = Game::GetConfig().GetString( L"VoxelLoadPath", L"TestVoxel.vx" );
		Game::GetFileLoader().LoadVoxelData( path, voxelizationParts, m_Width, m_Position, m_Size );
		m_Height = m_Depth = m_Width;
		m_ResolutionMultiplier = static_cast<uint32_t>( cbrtf( static_cast<float>( voxelizationParts.size() ) ) );
		UpdateGridData();
	}
	else {
		voxelizationParts.resize( m_ResolutionMultiplier * m_ResolutionMultiplier * m_ResolutionMultiplier );
		storeVoxelization = Game::GetConfig().GetBool( L"StoreVoxelization", false );
	}

	RenderBackend* renderBackend = &Game::GetRenderBackend();

	std::vector<std::pair<std::vector<Node>, std::vector<uint32_t>>> nodeList;
	nodeList.resize( voxelizationParts.size() );

	float3 voxelPartSize = m_Size / static_cast<float>( m_ResolutionMultiplier );

	ID3D11DeviceContext *context = &Game::GetContext();
	ID3D11Device* device = &Game::GetDevice();

	BufferDesc desc;
	desc.ByteWidth = sizeof( Node ) * m_NumTreeNodes;
	desc.CPUAccessFlags = CPUAccessFlag::Read | CPUAccessFlag::Write;
	desc.Usage = Usage::Staging;

	Buffer* tempNodeBuffer, *tempPointerBuffer, *tempApproxBuffer;
	HRESULT hr = device->CreateBuffer( reinterpret_cast<D3D11_BUFFER_DESC*>( &desc ), nullptr, &tempNodeBuffer );
	if( FAILED( hr ) )
		Game::GetLogger().FatalError( L"Buffer creation failed" );

	desc.ByteWidth = sizeof( uint32_t ) * m_NumTreeNodes;

	hr = device->CreateBuffer( reinterpret_cast<D3D11_BUFFER_DESC*>( &desc ), nullptr, &tempPointerBuffer );
	if( FAILED( hr ) )
		Game::GetLogger().FatalError( L"Buffer creation failed" );

	hr = device->CreateBuffer( reinterpret_cast<D3D11_BUFFER_DESC*>( &desc ), nullptr, &tempApproxBuffer );
	if( FAILED( hr ) )
		Game::GetLogger().FatalError( L"Buffer creation failed" );

	DebugData debugData( static_cast<uint32_t>( ceil( log2( m_Width * m_ResolutionMultiplier ) / 2.f ) ) );

	for( uint32_t voxelPart = 0; voxelPart < voxelizationParts.size(); voxelPart++ ) {
#ifndef PREVOXELIZE
		uint32_t numBricks;

		if( !loadVoxelization ) {
			uint32_t clearVal[4] = { 0,0,0,0 };
			renderBackend->ClearUAV( m_TreeUAV, clearVal );
			renderBackend->ClearUAV( m_CountUAV, clearVal );

			Profiler::GlobalProfiler.StartProfile( L"Voxelize", false );

			for( size_t i = 0; i < elements.size(); i++ ) {
				const Geometry& geometry = *elements[i].first;
				const Matrix& transform = elements[i].second;
				m_VoxelShader->SetShader();

				UpdateVoxelizeData( voxelPart, voxelPartSize );

				m_VoxelizeDataBuffer.Bind( ShaderFlag::ComputeShader, 0 );

				VoxelObjectData objData;
				objData.numTriangles = geometry.GetTriangleCount();
				objData.worldMat = transform;
				m_ObjectDataBuffer.Update( objData );
				m_ObjectDataBuffer.Bind( ShaderFlag::ComputeShader, 1 );

				renderBackend->SetUAVCS( 1, { m_TreeUAV, m_CountUAV }, { 0 } );
				renderBackend->SetSRVsCS( 0, { geometry.GetPositionSRV(), geometry.GetIndexSRV() } );

				uint32_t numTriangles = geometry.GetTriangleCount();
				uint32_t groupSize = 128;
				uint32_t numGroups = numTriangles / groupSize;
				if( numTriangles & ( groupSize - 1 ) )
					numGroups++;

				renderBackend->Dispatch( numGroups, 1, 1 );
				renderBackend->SetUAVCS( 1, { nullptr, nullptr }, { 0 } );
			}

			float time = Profiler::GlobalProfiler.EndProfile( L"Voxelize" );

			Game::GetLogger().Log( L"Voxelizer", L"Voxelization Time: " + std::to_wstring( time ) + L" ms" );

			renderBackend->ReadBuffer( m_CountBuffer, sizeof( uint32_t ), &numBricks, sizeof( uint32_t ), 0 );

			Game::GetLogger().Log( L"Voxelizer", L"Number of Bricks voxelized: " + std::to_wstring( numBricks ) );
			if( numBricks == 0 )
				continue;
		}
		else {
			numBricks = static_cast<uint32_t>( voxelizationParts[voxelPart].size() );
			if( numBricks == 0 )
				continue;
		}

#endif
		Node* nodes = nullptr;
		uint32_t* pointer = nullptr;
#ifdef ANISOTROPIC
		uint32_t* approx = nullptr;
#else
		float* approx = nullptr;
#endif	

#ifndef PREVOXELIZE
		if( !loadVoxelization ) {
			renderBackend->CopyResource( tempNodeBuffer, m_TreeBuffer );
		}
#endif // !PREVOXELIZE
		renderBackend->MapBuffer( tempNodeBuffer, reinterpret_cast<void**>( &nodes ), 0, MapType::ReadWrite );
		renderBackend->MapBuffer( tempPointerBuffer, reinterpret_cast<void**>( &pointer ), 0, MapType::Write );
		renderBackend->MapBuffer( tempApproxBuffer, reinterpret_cast<void**>( &approx ), 0, MapType::Write );

		if( storeVoxelization ) {
			SortAndOptimize( nodes, numBricks );
			voxelizationParts[voxelPart].assign( nodes, nodes + numBricks );
			if( Game::GetConfig().GetBool( L"JustStoreVoxelization", false ) ) {
				renderBackend->UnmapBuffer( tempNodeBuffer, 0 );
				renderBackend->UnmapBuffer( tempPointerBuffer, 0 );
				renderBackend->UnmapBuffer( tempApproxBuffer, 0 );
				continue;
			}
		}

		if( loadVoxelization ) {
			memcpy( nodes, voxelizationParts[voxelPart].data(), numBricks * sizeof( Node ) );
		}

#ifdef PREVOXELIZE
		uint32_t idx = 0;
		for( auto& object : m_VoxelizedObjects ) {
			if( object.first->IsVisible() ) {
				for( auto& node : object.second ) {
					nodes[idx++] = node;
				}
			}
		}
		uint32_t numBricks = idx;
#endif

		uint32_t maxLevel = static_cast<uint32_t>( ceil( log2( m_Width ) / 2.f ) ) - 1;
		uint32_t nodesSize, pointersSize;
		BuildTree( nodes, pointer, numBricks, maxLevel, nodesSize, pointersSize, debugData );
		ComputeApproximation( nodes, nodesSize, pointer, approx );

		if( m_ResolutionMultiplier > 1 ) {
			nodeList[voxelPart].first.assign( nodes, nodes + nodesSize );
			nodeList[voxelPart].second.assign( pointer, pointer + pointersSize );
			renderBackend->UnmapBuffer( tempNodeBuffer, 0 );
			renderBackend->UnmapBuffer( tempPointerBuffer, 0 );
			renderBackend->UnmapBuffer( tempApproxBuffer, 0 );
		}
		else {
			if( Game::GetConfig().GetBool( L"StoreTree", false ) ) {
				std::wstring path = Game::GetConfig().GetString( L"TreeStorePath", L"TestTree.tr" );
				Game::GetFileLoader().StoreTreeData( m_Width * m_ResolutionMultiplier, m_Position, m_Size, nodes, nodesSize, pointer, pointersSize, path );
			}
			renderBackend->UnmapBuffer( tempNodeBuffer, 0 );
			renderBackend->UnmapBuffer( tempPointerBuffer, 0 );
			renderBackend->UnmapBuffer( tempApproxBuffer, 0 );
			renderBackend->CopyResource( m_TreeBuffer, tempNodeBuffer );
			renderBackend->CopyResource( m_PointerBuffer, tempPointerBuffer );
			renderBackend->CopyResource( m_ApproxBuffer, tempApproxBuffer );
		}
	}

	tempNodeBuffer->Release();
	tempPointerBuffer->Release();
	tempApproxBuffer->Release();

	if( storeVoxelization ) {
		std::wstring path = Game::GetConfig().GetString( L"VoxelStorePath", L"TestVoxel.vx" );
		Game::GetFileLoader().StoreVoxelData( m_Width, m_Position, m_Size, voxelizationParts, path );
	}
	
	if( m_ResolutionMultiplier > 1 ) {
		StitchTree( nodeList );

		size_t maxLevel = debugData.TotalNodes.size();
		for( size_t i = 1; i < maxLevel; i++ ) {
			size_t idx = maxLevel - i;
			std::swap( debugData.IndvNodes[idx], debugData.IndvNodes[idx - 1] );
			std::swap( debugData.TotalNodes[idx], debugData.TotalNodes[idx - 1] );
		}
		for( size_t i = 2; i < maxLevel; i++ ) {
			size_t idx = maxLevel - i;
			std::swap( debugData.NumPointers[idx], debugData.NumPointers[idx - 1] );
		}

		debugData.IndvNodes[0] = 1;
		debugData.TotalNodes[0] = 1;
		debugData.NumPointers[0] = debugData.TotalNodes[1];

		UpdateGridData();
	}

	StoreDebugData( debugData );
	
	return nullptr;
}

void Voxelizer::TestRender( Camera & camera ) {
	RenderBackend* renderBackend = &Game::GetRenderBackend();

	VoxelTestData data;

	data.vCamPos = make_float3a( camera.GetTransform().GetWorldPosition() );
	data.vLookDir = make_float3a( camera.GetFwdVector() );
	data.vLookUp = make_float3a( camera.GetUpVector() );
	data.vLookRight = make_float3a( camera.GetRightVector() );
	float fov = camera.GetFoV();
	data.fMaxScreenVal.y = tan( fov / 2.0f );
	data.fMaxScreenVal.x = data.fMaxScreenVal.y * camera.GetAspectRatio();
	data.vCamSize = float2( 1.5f, 1.5f );

	m_TestDataBuffer.Update( data );
	m_TestDataBuffer.Bind( ShaderFlag::PixelShader, 0 );
	m_GridDataBuffer.Bind( ShaderFlag::PixelShader, 4 );

	m_TestPass->Apply( RenderFlag::None );
	renderBackend->SetTopology( PrimitiveTopology::Trianglelist );

	ImGui::SliderFloat( "Max Iteration", &m_MaxItValue, 20.f, 200.f, "%.0f" );
	UpdateGridData();

	BindSRVs();

	renderBackend->Draw( 3, 0 );

	renderBackend->SetSRVsPS( 0, { nullptr, nullptr, nullptr } );
}

void Voxelizer::BindSRVs() {
	m_GridDataBuffer.Bind( ShaderFlag::PixelShader, 4 );

#ifndef TEMPORAL
	ImGui::SliderInt( "NumSamples", &m_NumLightSamples, 1, 10 );
#endif
	ImGui::SliderFloat( "Light Size Angle", &m_LightAngleSize, 0.f, 20.f, "%.1f" );
	ImGui::SliderAngle( "Vertical Light Angle", &m_VerticalLightAngle, 0.f, 90.f );
	ImGui::SliderAngle( "Horizontal Light Angle", &m_HorizontalLightAngle, 0.f, 360.f );

	Game::GetRenderer().SetShadowParams( static_cast<uint32_t>( m_NumLightSamples ), m_LightAngleSize );

	Game::GetRenderer().SetLight( GetLightDir(), { 1.f,1.f,1.f,1.f } );

	Game::GetRenderBackend().SetSRVsPS( 2, { m_TreeSRV, m_PointerSRV, m_ApproxSRV } );

	m_UseShadowTexOne = !m_UseShadowTexOne;

	if( m_UseShadowTexOne ) {
		Game::GetRenderBackend().SetUAVPS( 1, { nullptr }, { 0 } );
		Game::GetRenderBackend().SetSRVsPS( 5, { m_ShadowTexture2->GetSRV() } );
		Game::GetRenderBackend().SetUAVPS( 1, { m_ShadowTexture1->GetUAV() }, { 0 } );
	}
	else {
		Game::GetRenderBackend().SetUAVPS( 1, { nullptr }, { 0 } );
		Game::GetRenderBackend().SetSRVsPS( 5, { m_ShadowTexture1->GetSRV() } );
		Game::GetRenderBackend().SetUAVPS( 1, { m_ShadowTexture2->GetUAV() }, { 0 } );
	}

	static Matrix lastFrameMat = Game::GetRenderer().GetActiveCamera()->GetViewProjMat();

	FilterData filterData;
	filterData.LastFrameMatrix = lastFrameMat;
	filterData.Seed = std::rand();
	filterData.JitterRad = tan( Deg2Rad( m_LightAngleSize ) );

	static float radiusMod = 2.f;
	static float depthThreshold = 0.00003f;
	static float jitterSep = 0.4f;

	ImGui::SliderFloat( "Radius Modifier", &radiusMod, 0.1f, 2.f, "%.1f" );
	ImGui::SliderFloat( "Depth Threshold", &depthThreshold, 0.00001f, 0.1f, "%.5f", 10.f );
	ImGui::SliderFloat( "Jitter Seperator", &jitterSep, 0.f, 1.f, "%.2f" );

	filterData.RadiusMod = radiusMod;
	filterData.DepthThresh = depthThreshold;
	filterData.JitterSep = jitterSep;

	static bool renderShadow = true;

	if( renderShadow && ImGui::Button( "Deactivate Shadows" ) )
		renderShadow = false;
	else if( !renderShadow && ImGui::Button( "Activate Shadows" ) )
		renderShadow = true;

	static bool useNormal = true;

	if( useNormal && ImGui::Button( "Use Approximation" ) )
		useNormal = false;
	else if( !useNormal && ImGui::Button( "Use Normal" ) )
		useNormal = true;

	static bool useReprojection = true;

	if( useReprojection && ImGui::Button( "Deactivate Reprojection" ) )
		useReprojection = false;
	else if( !useReprojection && ImGui::Button( "Activate Reprojection" ) )
		useReprojection = true;

	static bool useSmooth = true;

	if( useSmooth && ImGui::Button( "Deactivate Smoothing" ) )
		useSmooth = false;
	else if( !useSmooth && ImGui::Button( "Activate Smoothing" ) )
		useSmooth = true;

	if( ImGui::Button( "Embree Trace" ) ) {
		Game::GetRenderBackend().TraceWithEmdree();
	}

	filterData.UseNormal = useNormal ? UINT32_MAX : 0;
	filterData.UseReprojection = useReprojection ? UINT32_MAX : 0;
	filterData.UseSmooth = useSmooth ? UINT32_MAX : 0;
	filterData.RenderShadow = renderShadow ? UINT32_MAX : 0;

	m_FilterDataBuffer.Update( filterData );
	m_FilterDataBuffer.Bind( ShaderFlag::PixelShader, 1 );

	lastFrameMat = Game::GetRenderer().GetActiveCamera()->GetViewProjMat();
}

void Voxelizer::SetLightSizeAngle( float angle ) {
	m_LightAngleSize = angle;
}

void Voxelizer::SetLightSamples( uint32_t samples ) {
	m_NumLightSamples = samples;
}

void Voxelizer::SetLightDir( float horizontal, float vertical ) {
	m_HorizontalLightAngle = horizontal;
	m_VerticalLightAngle = vertical;
}

float3 Voxelizer::GetLightDir() {
	float angle1 = m_VerticalLightAngle;
	float angle2 = m_HorizontalLightAngle;
	float xyMul = cosf( angle1 );

	float3 lightDir;
	lightDir.x = xyMul * sinf( angle2 );
	lightDir.y = xyMul * cosf( angle2 );
	lightDir.z = sinf( angle1 );

	return lightDir;
}

float Voxelizer::GetLightAngleSize() {
	return m_LightAngleSize;
}

void Voxelizer::SetSliderVisibility( bool visible ) {
	//m_AngleSlider->SetVisible( visible );
	//m_SampleSlider->SetVisible( visible );
	//m_VerticalLightSlider->SetVisible( visible );
	//m_HorizontalLightSlider->SetVisible( visible );
}

Texture * Voxelizer::GetCurrentShadowTex() {
	if( m_UseShadowTexOne )
		return m_ShadowTexture1;
	else
		return m_ShadowTexture2;
}

bool Voxelizer::CreateBuffers() {
	RenderBackend* renderBackend = &Game::GetRenderBackend();

	// Buffer for tree nodes
	BufferDesc bDesc;
	bDesc.BindFlags = BindFlag::ShaderResource | BindFlag::UnorderedAccess;
	bDesc.MiscFlags = ResourceMiscFlag::BufferStructured;
	bDesc.ByteWidth = m_NumTreeNodes * 3 * sizeof( uint32_t );
	bDesc.StructureByteStride = 3 * sizeof( uint32_t );

	m_TreeBuffer = renderBackend->CreateBuffer( nullptr, bDesc );

	if( !m_TreeBuffer )
		return false;

	UAVDesc uavDesc;
	uavDesc.Format = Format::Unknown;
	uavDesc.ViewDimension = UAVDimension::Buffer;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = m_NumTreeNodes;
	uavDesc.Buffer.Flags = UAVBufferFlag::Counter;

	m_TreeUAV = renderBackend->CreateUAV( m_TreeBuffer, &uavDesc );

	if( !m_TreeUAV )
		return false;

	SRVDesc srvDesc;
	srvDesc.Format = Format::Unknown;
	srvDesc.ViewDimension = SRVDimension::Buffer;
	srvDesc.Buffer.ElementOffset = 0;
	srvDesc.Buffer.NumElements = m_NumTreeNodes;

	m_TreeSRV = renderBackend->CreateSRV( m_TreeBuffer, &srvDesc );

	if( !m_TreeSRV )
		return false;

	// counter buffer for next available pointer
	bDesc.ByteWidth = sizeof( uint32_t );
	bDesc.BindFlags = BindFlag::UnorderedAccess;
	bDesc.MiscFlags = ResourceMiscFlag::None;

	m_CountBuffer = renderBackend->CreateBuffer( nullptr, bDesc );

	if( !m_CountBuffer )
		return false;

	uavDesc.Format = Format::R32_UInt;
	uavDesc.Buffer.NumElements = 1;
	uavDesc.Buffer.Flags = UAVBufferFlag::None;

	m_CountUAV = renderBackend->CreateUAV( m_CountBuffer, &uavDesc );

	if( !m_CountUAV )
		return false;

	bDesc.ByteWidth = sizeof( uint32_t ) * m_NumTreeNodes;
	bDesc.BindFlags = BindFlag::ShaderResource | BindFlag::UnorderedAccess;

	m_PointerBuffer = renderBackend->CreateBuffer( nullptr, bDesc );

	if( !m_PointerBuffer )
		return false;

	uavDesc.Buffer.NumElements = m_NumTreeNodes;

	m_PointerUAV = renderBackend->CreateUAV( m_PointerBuffer, &uavDesc );

	if( !m_PointerUAV )
		return false;

	srvDesc.Format = Format::R32_UInt;
	srvDesc.Buffer.NumElements = m_NumTreeNodes;

	m_PointerSRV = renderBackend->CreateSRV( m_PointerBuffer, &srvDesc );

	if( !m_PointerSRV )
		return false;

	bDesc.ByteWidth = sizeof( float ) * m_NumTreeNodes;
	bDesc.BindFlags = BindFlag::ShaderResource;

	m_ApproxBuffer = renderBackend->CreateBuffer( nullptr, bDesc );
	if( !m_ApproxBuffer )
		return false;

#ifdef ANISOTROPIC
	srvDesc.Format = Format::R10G10B10A2_Unorm;
#else
	srvDesc.Format = Format::R32_Float;
#endif

	m_ApproxSRV = renderBackend->CreateSRV( m_ApproxBuffer, &srvDesc );
	if( !m_ApproxSRV )
		return false;

	TextureDesc tDesc;
	tDesc.UniqueName = L"ShadowTexture1";
	tDesc.TextureType = TextureType::Texture2D;

	Texture2DDesc tex2dDesc;
	tex2dDesc.BindFlags = BindFlag::UnorderedAccess | BindFlag::ShaderResource;
	tex2dDesc.Format = Format::R32_Float;
	tex2dDesc.Width = Game::GetWindow().GetWidth();
	tex2dDesc.Height = Game::GetWindow().GetHeight();

	tDesc.Texture2DDesc = &tex2dDesc;

	m_ShadowTexture1 = Texture::Create( tDesc );
	tDesc.UniqueName = L"ShadowTexture2";
	m_ShadowTexture2 = Texture::Create( tDesc );

	Game::GetRenderBackend().RegisterResizeCallBack( this, [&]( uint2 size ) {
		m_ShadowTexture1->ResizeResources( size.x, size.y );
		m_ShadowTexture2->ResizeResources( size.x, size.y );
	} );

	return true;
}

bool Voxelizer::CreateRenderPass() {

	m_VoxelShader = Shader::Get( L"csVoxel" );
	m_SparselizeShader = Shader::Get( L"csSparselize" );
	m_MipMapShader = Shader::Get( L"csMipMap" );

	RenderPassInit rInit;
	rInit.Name = L"VoxelLinkedFragmentPass";
	rInit.RasterizerDesc.CullMode = CullMode::None;
	rInit.DepthStencilDesc.DepthEnable = false;
	rInit.DepthStencilDesc.StencilEnable = false;
	rInit.VertexShader = Shader::Get( L"vsVoxel" );
	rInit.PixelShader = Shader::Get( L"psVoxel" );
	rInit.GeometryShader = Shader::Get( L"gsVoxel" );
	rInit.RenderPassType = RenderPassType::Manual;

	m_VoxelizePass = RenderPass::Create( rInit );
	if( !m_VoxelizePass ) {
		Game::GetLogger().Log( L"Voxelizer", L"Renderpass creation failed" );
		return false;
	}

	rInit.Name = L"VoxelTestPass";
	rInit.VertexShader = Shader::Get( L"vsFullScreenQuad" );
	rInit.GeometryShader = nullptr;
	rInit.PixelShader = Shader::Get( L"psVoxelTest" );

	m_TestPass = RenderPass::Create( rInit );

	return true;
}

bool Voxelizer::CreateCamera() {
	CameraDesc cDesc;
	cDesc.IsPerspective = false;
	cDesc.Ortographic.Width = 2.0f;
	cDesc.Ortographic.Height = 2.0f;
	cDesc.Viewport.Width = static_cast<float>( m_Width );
	cDesc.Viewport.Height = static_cast<float>( m_Height );
	cDesc.UseBackBufferRTV = true;
	cDesc.UseWindowSizeforVP = false;
	cDesc.NearPlane = 0.1f;
	cDesc.FarPlane = 2.1f;
	cDesc.Position = { -1.1f,0.f,0.f };

	m_Camera = Game::GetGOManager().Instantiate<Camera>( cDesc );
	if( !m_Camera ) {
		Game::GetLogger().Log( L"Voxelizer", L"Creating Camera failed" );
		return false;
	}

	return true;
}

void Voxelizer::StitchTree( const std::vector<std::pair<std::vector<Node>, std::vector<uint32_t>>>& treeParts ) {
	// Construct root node
	Node rootNode;
	rootNode.Data.x = 0;
	rootNode.Data.y = 0;
	rootNode.Pointer = 1;

	uint32_t pointerIdx = 1;
	uint32_t nodeIdx = 1;

	for( size_t i = 0; i < treeParts.size(); ++i ) {
		if( !treeParts[i].first.empty() ) {
			if( i < 32 )
				rootNode.Data.x |= 1 << i;
			else
				rootNode.Data.y |= 1 << ( i - 32 );
			++pointerIdx;
		}
	}

	ID3D11DeviceContext *context = &Game::GetContext();
	ID3D11Device* device = &Game::GetDevice();

	BufferDesc desc;
	desc.ByteWidth = sizeof( Node ) * m_NumTreeNodes;
	desc.CPUAccessFlags = CPUAccessFlag::Read | CPUAccessFlag::Write;
	desc.Usage = Usage::Staging;

	Buffer* tempNodeBuffer, *tempPointerBuffer;
	HRESULT hr = device->CreateBuffer( reinterpret_cast<D3D11_BUFFER_DESC*>( &desc ), nullptr, &tempNodeBuffer );
	if( FAILED( hr ) )
		Game::GetLogger().FatalError( L"Buffer creation failed" );

	desc.ByteWidth = sizeof( uint32_t ) * m_NumTreeNodes;

	std::vector<uint32_t> data;
	data.resize( m_NumTreeNodes );

	SubresourceData subData;
	subData.pSysMem = data.data();
	subData.SysMemPitch = sizeof( uint32_t ) * m_NumTreeNodes;

	device->CreateBuffer( reinterpret_cast<D3D11_BUFFER_DESC*>( &desc ), &subData, &tempPointerBuffer );

	Node* nodes = nullptr;
	uint32_t* pointer = nullptr;

	RenderBackend* renderBackend = &Game::GetRenderBackend();
	renderBackend->MapBuffer( tempNodeBuffer, reinterpret_cast<void**>( &nodes ), 0, MapType::ReadWrite );
	renderBackend->MapBuffer( tempPointerBuffer, reinterpret_cast<void**>( &pointer ), 0, MapType::ReadWrite );

	nodes[0] = rootNode;
	pointer[0] = 0;
	uint32_t rootIdx = 1;
	for( size_t i = 0; i < treeParts.size(); ++i ) {
		if( !treeParts[i].first.empty() ) {
			pointer[rootIdx++] = nodeIdx;
			uint32_t oldNodeIdx = nodeIdx;
			for( const Node& node : treeParts[i].first ) {
				nodes[nodeIdx] = node;
				nodes[nodeIdx].Pointer += pointerIdx;
				++nodeIdx;
			}
			for( uint32_t ptr : treeParts[i].second ) {
				pointer[pointerIdx++] = ptr + oldNodeIdx;
			}
		}
	}

	if( Game::GetConfig().GetBool( L"StoreTree", false ) ) {
		std::wstring path = Game::GetConfig().GetString( L"TreeStorePath", L"TestTree.tr" );
		Game::GetFileLoader().StoreTreeData( m_Width * m_ResolutionMultiplier, m_Position, m_Size, nodes, nodeIdx, pointer, pointerIdx, path );
	}

	renderBackend->UnmapBuffer( tempNodeBuffer, 0 );
	renderBackend->UnmapBuffer( tempPointerBuffer, 0 );

	renderBackend->CopyResource( m_TreeBuffer, tempNodeBuffer );
	renderBackend->CopyResource( m_PointerBuffer, tempPointerBuffer );

	tempNodeBuffer->Release();
	tempPointerBuffer->Release();
}

void Voxelizer::UpdateVoxelizeData( uint32_t voxelPart, const float3& partSize ) {
	VoxelizeData data;

	data.gridSize = { m_Width, m_Height, m_Depth };
	data.numBits = { 4, 4, 4 };
	data.numTexels = { m_Width / data.numBits.x, m_Height / data.numBits.y, m_Depth / data.numBits.z };
	data.deltaGrid = { partSize.x / static_cast<float>( m_Width ), partSize.y / static_cast<float>( m_Height ), partSize.z / static_cast<float>( m_Depth ) };
	data.invDeltaGrid = { 1.0f / data.deltaGrid.x, 1.0f / data.deltaGrid.y, 1.0f / data.deltaGrid.z };
	data.minBoxPos = make_float3a( make_float3( MortonDecode( voxelPart ) ) * partSize + m_Position - 0.5f * m_Size );
	data.boxSize = make_float3a( partSize );

	m_VoxelizeDataBuffer.Update( data );
}

bool Voxelizer::LoadTree( const std::wstring & fileName ) {
	Node* nodePtr = nullptr;
	uint32_t* pointerPtr = nullptr;
#ifdef ANISOTROPIC
	uint32_t* approx = nullptr;
#else
	float* approx = nullptr;
#endif // ANISOTROPIC

	ID3D11DeviceContext *context = &Game::GetContext();
	ID3D11Device* device = &Game::GetDevice();

	BufferDesc desc;
	desc.ByteWidth = sizeof( Node ) * m_NumTreeNodes;
	desc.CPUAccessFlags = CPUAccessFlag::Write;
	desc.Usage = Usage::Staging;

	Buffer* tempNodeBuffer, *tempPointerBuffer, *tempApproxBuffer;
	HRESULT hr = device->CreateBuffer( reinterpret_cast<D3D11_BUFFER_DESC*>( &desc ), nullptr, &tempNodeBuffer );
	if( FAILED( hr ) )
		Game::GetLogger().FatalError( L"Buffer creation failed" );

	desc.ByteWidth = sizeof( uint32_t ) * m_NumTreeNodes;

	hr = device->CreateBuffer( reinterpret_cast<D3D11_BUFFER_DESC*>( &desc ), nullptr, &tempPointerBuffer );
	if( FAILED( hr ) )
		Game::GetLogger().FatalError( L"Buffer creation failed" );

	hr = device->CreateBuffer( reinterpret_cast<D3D11_BUFFER_DESC*>( &desc ), nullptr, &tempApproxBuffer );
	if( FAILED( hr ) )
		Game::GetLogger().FatalError( L"Buffer creation failed" );

	RenderBackend* renderBackend = &Game::GetRenderBackend();
	renderBackend->MapBuffer( tempNodeBuffer, reinterpret_cast<void**>( &nodePtr ), 0, MapType::Write );
	renderBackend->MapBuffer( tempPointerBuffer, reinterpret_cast<void**>( &pointerPtr ), 0, MapType::Write );
	renderBackend->MapBuffer( tempApproxBuffer, reinterpret_cast<void**>( &approx ), 0, MapType::Write );

	uint32_t numNodes = 0, numPointers = 0;
	if( !Game::GetFileLoader().LoadTreeData( fileName, nodePtr, numNodes, pointerPtr, numPointers, m_Width, m_Position, m_Size ) )
		return false;
#ifdef SOFTSHADOW
	ComputeApproximation( nodePtr, numNodes, pointerPtr, approx );
#endif // SOFTSHADOW

	m_Height = m_Depth = m_Width;

	m_ResolutionMultiplier = 1;

	renderBackend->UnmapBuffer( tempNodeBuffer, 0 );
	renderBackend->UnmapBuffer( tempPointerBuffer, 0 );
	renderBackend->UnmapBuffer( tempApproxBuffer, 0 );

	renderBackend->CopyResource( m_TreeBuffer, tempNodeBuffer );
	renderBackend->CopyResource( m_PointerBuffer, tempPointerBuffer );

	tempNodeBuffer->Release();
	tempPointerBuffer->Release();
	tempApproxBuffer->Release();

	Game::GetLogger().Log( L"Voxelizer", L"Voxel file loaded with " + std::to_wstring( numNodes ) + L" nodes and " + std::to_wstring( numPointers ) + L" pointers loaded." );

	UpdateGridData();
	
	return true;
}

void Voxelizer::StoreDebugData( const DebugData & debugData ) {
	nlohmann::json j;

	j["TotalNodes"] = debugData.TotalNodes;
	j["IndividualNodes"] = debugData.IndvNodes;
	j["Pointers"] = debugData.NumPointers;

	size_t memoryConsumption = 0;
	for( size_t i = 0; i < debugData.IndvNodes.size() - 1; ++i ) {
		memoryConsumption += debugData.IndvNodes[i] * sizeof( Node );
	}
	memoryConsumption += debugData.CompressedLeaves * sizeof( Node );
	for( uint32_t num : debugData.NumPointers ) {
		memoryConsumption += num * sizeof( uint32_t );
	}
	j["MemoryConsumption"] = memoryConsumption;

	size_t totalMemory = 0;
	for( uint32_t num : debugData.TotalNodes ) {
		totalMemory = num * ( sizeof( Node ) + sizeof( uint32_t ) );
	}
	j["TotalMemoryCompression"] = 1 - ( float( memoryConsumption ) / float( totalMemory ) );

	j["CompressedLeaves"] = debugData.CompressedLeaves;
	j["SortingTime"] = debugData.SortingTime;
	j["TreeBuildTime"] = debugData.TreeBuildTime;
	j["ClusteringTime"] = debugData.ClusteringTime;
	j["LeaveAddingTime"] = debugData.LeaveAddingTime;
	j["DoubleNodeRemovelTime"] = debugData.DoubleNodeRemovelTime;

	j["VoxelResolution"] = m_Width * m_ResolutionMultiplier;
	std::string comparison;
	switch( Game::GetConfig().GetInt( L"SimilarityTest" ) ) {
		case 0:
			comparison = "PureSimilarity";
			break;
		case 1:
			comparison = "NeigboorhoodComparison";
			break;
		case 2:
			comparison = "EMD";
		case 3:
			comparison = "DistanceField";
		default:
			break;
	}

	j["Comparison"] = comparison;

	Game::GetLogger().Log( L"JSON", s2ws( j.dump( 4 ) ) );
}

void Voxelizer::UpdateGridData() {
	GridData gridData;
	gridData.GridResolution = { m_Width / 4 * m_ResolutionMultiplier, m_Height / 4 * m_ResolutionMultiplier, m_Depth / 4 * m_ResolutionMultiplier };
	gridData.TreeSize = static_cast<uint32_t>( ceil( log2( m_Width * m_ResolutionMultiplier ) / 2.f ) - 1 );
	gridData.MinGridPosition = make_float3a( m_Position - 0.5f * m_Size );
	gridData.GridSize = make_float3a( m_Size );
	gridData.MaxItValue = m_MaxItValue;
	m_GridDataBuffer.Update( gridData );
}
