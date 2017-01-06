#include "D3DRenderBackend.h"

#include <vector>

#include "Shader\VoxelDefines.hlsli"

#include "Logger.h"
#include "Window.h"
#include "Makros.h"
#include "Shader.h"
#include "Math.h"
#include "GameObjectManager.h"
#include "Camera.h"
#include "Renderer.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"
#include "DirectXTex.h"
#include "Texture.h"
#include "Scene.h"
#include "GUI.h"
#include "InputManager.h"
#include "Voxelizer.h"
#include "Time.h"
#include "ConfigManager.h"
#include "Profiler.h"
#include "RenderPass.h"
#include "Embree.h"

#include "imgui.h"
#include "ImGUI_Impl.h"

#include <Wincodec.h>


void D3DRenderBackend::AddToVoxelization( GameObject& gameObject ) {
	const Geometry& geometry = *gameObject.GetRenderable()->GetGeometry();
	const Matrix& transform = gameObject.GetTransform().GetWorldTransMat();

	m_VoxelElements.push_back( { &geometry, transform } );
	m_EmbreeObjects.push_back( &gameObject );
	m_Voxelizer->VoxelizeObject( gameObject );
}

void D3DRenderBackend::NotifyChangeInVisibility() {
	m_RebuildTree = true;
}

void D3DRenderBackend::SaveTexture( TextureResource* res, const std::wstring & fileName ) {
	using namespace DirectX;
	ScratchImage image;
	HRESULT hr = CaptureTexture( m_Device, m_Context, res, image );

	if( FAILED( hr ) ) {
		Game::GetLogger().Log( L"RenderBackend", L"Failed taking screenshot" );
		return;
	}

	hr = SaveToWICFile( *image.GetImage( 0, 0, 0 ), WIC_FLAGS_IGNORE_SRGB, GetWICCodec( WIC_CODEC_PNG ), ( fileName + L".png" ).c_str(), &GUID_WICPixelFormat24bppBGR );

	if( FAILED( hr ) )
		Game::GetLogger().Log( L"RenderBackend", L"Failed saving screenshot in " + fileName );
}

void D3DRenderBackend::TakeScreenShot( const std::wstring & fileName ) {
	m_ScreenShotName = fileName;
	m_SaveScreenShot = true;
}

void D3DRenderBackend::SetLightSizeAngle( float angle ) {
	m_Voxelizer->SetLightSizeAngle( angle );
}

void D3DRenderBackend::SetLightSamples( uint32_t samples ) {
	m_Voxelizer->SetLightSamples( samples );
}

void D3DRenderBackend::SetLightDir( float horizontal, float vertical ) {
	m_Voxelizer->SetLightDir( horizontal, vertical );
}

void D3DRenderBackend::SetGUIVisibility( bool visibility ) {
	m_Voxelizer->SetSliderVisibility( visibility );
}

void D3DRenderBackend::TraceWithEmdree() {
	if( m_Embree )
		m_Embree->Render( m_DefaultRenderTarget, *Game::GetRenderer().GetActiveCamera(), m_Voxelizer->GetLightDir(), m_Voxelizer->GetLightAngleSize() );
}

void D3DRenderBackend::CopyRTV_DSV() {
	CopyResource( m_TempRTV->GetTextureResource(), m_DefaultRenderTarget->GetTextureResource() );
	CopyResource( m_TempDSV->GetTextureResource(), m_DefaultDepthStencil->GetTextureResource() );
}

void D3DRenderBackend::RegisterResizeCallBack( void * owner, std::function<void( uint2 )> callBack ) {
	m_ResizeCallbacks.emplace_back( owner, callBack );
}

void D3DRenderBackend::UnRegisterCallBacks( void * owner ) {
	while( !m_ResizeCallbacks.empty() && m_ResizeCallbacks.back().first == owner ) {
		m_ResizeCallbacks.pop_back();
	}

	for( size_t i = 0; i < m_ResizeCallbacks.size(); i++ ) {
		if( m_ResizeCallbacks[i].first == owner ) {
			std::swap( m_ResizeCallbacks[i], m_ResizeCallbacks.back() );
			m_ResizeCallbacks.pop_back();
		}
	}
}

D3DRenderBackend::D3DRenderBackend( Window& mainWindow )
	: m_MainWindow( &mainWindow ) {
	Game::SetRenderBackend( *this );
	Initialize();
}


D3DRenderBackend::~D3DRenderBackend() {
	ImGui_ImplDX11_Shutdown();

	SRelease( m_Context );
	SRelease( m_SwapChain );
	SRelease( m_Device );

	SReleaseVec( m_RasterizerStates );
	SReleaseVec( m_BlendStates );
	SReleaseVec( m_DepthStencilStates );
	SReleaseVec( m_Buffers );
	SReleaseVec( m_SamplerStates );

	delete m_Voxelizer;
	delete m_Embree;
}

D3DRenderBackend & D3DRenderBackend::Init( Window & window ) {
	static D3DRenderBackend renderBackend( window );
	return renderBackend;
}

void D3DRenderBackend::Initialize() {
	HRESULT hr;

	IDXGIFactory* factory;
	hr = CreateDXGIFactory( __uuidof( IDXGIFactory ), reinterpret_cast<void**>( &factory ) );
	if( FAILED( hr ) ) {
		Game::GetLogger().FatalError( L"Factory creation failed" );
		return;
	}

	UINT flags = 0;// D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[2] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_11_1
	};

	IDXGIAdapter* adapter = GetAdapter( factory );
	if( !adapter )
		return;

	D3D_FEATURE_LEVEL usedFeatureLevel;
	hr = D3D11CreateDevice( adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, flags, featureLevels,
							ARRAYSIZE( featureLevels ), D3D11_SDK_VERSION, &m_Device, &usedFeatureLevel, nullptr );
	if( FAILED( hr ) ) {
		Game::GetLogger().FatalError( L"Device creation failed" );
		return;
	}

	m_Device->GetImmediateContext( &m_Context );

	CreateSwapChain( factory, *m_MainWindow, m_SwapChain );
	CreateDefaultRTV_DSV();
	if( !CreateDeferredRTVs() )
		return;

	// just leave it here in case no targets are set, the game won't crash instantly
	SetRenderTarget( { m_DefaultRenderTarget->GetRTV() }, m_DefaultDepthStencil->GetDSV() );

	m_MainViewPort.Width = static_cast<float>( m_MainWindow->GetWidth() );
	m_MainViewPort.Height = static_cast<float>( m_MainWindow->GetHeight() );
	m_MainViewPort.MinDepth = 0.0f;
	m_MainViewPort.MaxDepth = 1.0f;
	m_MainViewPort.TopLeftX = 0.0f;
	m_MainViewPort.TopLeftY = 0.0f;

	m_Context->RSSetViewports( 1, &m_MainViewPort );

	SRelease( factory );

	Game::SetDevice( *m_Device );
	Game::SetContext( *m_Context );

	&ShaderManager::Init( m_Device, { L"Shader\\" } );

	m_MainWindow->SetGraphicsCallback( [&]( bool fullScreen, uint2 size ) {
		OnWindowChange( fullScreen, size );
	} );

	Profiler::GlobalProfiler.Initialize( m_Device, m_Context );
	ImGui_ImplDX11_Init( Game::GetWindow().GetHandle(), m_Device, m_Context );

	ImGui_ImplDX11_NewFrame();
}

void D3DRenderBackend::Start() {

	ConfigManager* config = &Game::GetConfig();

	float3 voxelPos = config->GetFloat3( L"VoxelPos", { 0.f,0.f,0.f } );
	float3 voxelSize = config->GetFloat3( L"VoxelSize", { 3.f, 3.f, 3.f } );
	int voxelRes = config->GetInt( L"VoxelRes", 256 );

	m_Voxelizer = new Voxelizer( voxelPos, voxelSize, voxelRes );
	m_Embree = new Embree();
	if( !m_Embree->Init() )
		Game::GetLogger().FatalError( L"Failed creating embree" );

	m_MainScene = new Scene( *m_MainWindow );
	m_MainScene->LoadScene();

	m_Embree->SetGeometry( m_EmbreeObjects );

	Game::GetRenderer().Start();

	Game::GetInput().RegisterKeyUpCallback( Key::F1, [&]() {
		m_RenderVoxel = !m_RenderVoxel;
	} );

	if( Game::GetConfig().GetBool( L"Voxelize", true ) )
		m_Voxelizer->Voxelize( m_VoxelElements );
	m_VoxelElements.clear();


	RenderPassInit rInit;
	rInit.Name = L"Shadow";
	rInit.VertexShader = Shader::Get( L"vsFullScreenQuad" );
	rInit.PixelShader = Shader::Get( L"psShadow" );
	rInit.PreFunction = [&]() {
		SetRenderTarget( { nullptr }, nullptr );
		m_Voxelizer->BindSRVs();
		SetSRVsPS( 0, { m_TempDSV->GetSRV(),  m_DeferredRTVs[1]->GetSRV() } );
	};
	rInit.RenderPassType = RenderPassType::Post;
	rInit.ShowTiming = true;
	rInit.TimingName = L"Shadow Calculation";
	RenderPass::Create( rInit );

	rInit.Name = L"SmoothComposing";
	rInit.VertexShader = Shader::Get( L"vsFullScreenQuad" );
	rInit.PixelShader = Shader::Get( L"psPostSmooth" );
	rInit.PreFunction = [&]() {
		SetRTV_UAV( { m_DefaultRenderTarget->GetRTV() }, m_DefaultDepthStencil->GetDSV(), 1, { m_Voxelizer->GetCurrentShadowTex()->GetUAV() }, { 0 } );
		SetSRVsPS( 0, { m_TempDSV->GetSRV(), m_DeferredRTVs[0]->GetSRV(), m_DeferredRTVs[1]->GetSRV() } );
	};
	rInit.RenderPassType = RenderPassType::Post;
	rInit.RenderPassOrderIndex = 1;
	rInit.ShowTiming = true;
	rInit.TimingName = L"Gaussian Smoothing";
	RenderPass::Create( rInit );
}

void D3DRenderBackend::Render() {
	assert( m_Context );

	ImGui_ImplDX11_NewFrame();

	ImGui::Text( "FPS: %f", Game::GetTime().GetFPS() );

	Game::GetRenderer().Update();

	Profiler::GlobalProfiler.StartProfile( L"Render Time", false );

	if( !m_RenderVoxel ) {
		Game::GetRenderer().Render();
	}
	else {
		m_Voxelizer->TestRender( *Game::GetRenderer().GetActiveCamera() );
	}

	ImGui::Text( "RenderTime: %fms", Profiler::GlobalProfiler.EndProfile( L"Render Time" ) );
	Profiler::GlobalProfiler.EndFrame();

	if( m_SaveScreenShot ) {
		SaveTexture( m_DefaultRenderTarget->GetTextureResource(), m_ScreenShotName );
		m_SaveScreenShot = false;
	}

	// unset geometry shader, so that imgui works
	m_Context->GSSetShader( nullptr, nullptr, 0 );

	static bool renderImGui = true;

	if( Game::GetInput().KeyDown( Key::U ) ) {
		ImGui::SetWindowCollapsed( renderImGui );
		renderImGui = !renderImGui;
	}

	if( renderImGui )
		ImGui::Render();

	Game::GetGUI().Render();

	HRESULT hr = m_SwapChain->Present( 0, 0 );
	if( FAILED( hr ) ) {
		Game::GetLogger().FatalError( L"Rendering Failed" );
	}
}

void D3DRenderBackend::Exit() {
	delete m_MainScene;
}

void D3DRenderBackend::OnWindowChange( bool fullscreen, uint2 size ) {
	ResizeRTV_DSV( size.x, size.y );
	HRESULT hr;
	if( fullscreen ) {
		hr = m_SwapChain->SetFullscreenState( true, nullptr );
	}
	else {
		hr = m_SwapChain->SetFullscreenState( false, nullptr );
	}
	if( FAILED( hr ) )
		Game::GetLogger().FatalError( L"Resizing swapchaing failed" );

	Game::GetGOManager().OnResize( size.x, size.y );
	Game::GetRenderer().OnResize();
	Game::GetGUI().OnResize( size.x, size.y );

	for( auto& callback : m_ResizeCallbacks ) {
		callback.second( size );
	}
}

void D3DRenderBackend::CreateSwapChain( IDXGIFactory* factory, const Window & window, IDXGISwapChain * swapChain ) {
	assert( m_Device );
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = m_MainWindow->GetWidth();
	swapChainDesc.BufferDesc.Height = m_MainWindow->GetHeight();
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	swapChainDesc.OutputWindow = m_MainWindow->GetHandle();
	swapChainDesc.Windowed = true;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	HRESULT hr = factory->CreateSwapChain( m_Device, &swapChainDesc, &m_SwapChain );
	if( FAILED( hr ) ) {
		Game::GetLogger().FatalError( L"SwapChain creation failed" );
		return;
	}
}

void D3DRenderBackend::CreateDefaultRTV_DSV() {
	HRESULT hr;
	ID3D11Texture2D* backBufferTexture;
	hr = m_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&backBufferTexture );
	if( FAILED( hr ) ) {
		Game::GetLogger().FatalError( L"Getting BackBuffer failed" );
		return;
	}

	RenderTargetView* rtv;
	hr = m_Device->CreateRenderTargetView( backBufferTexture, nullptr, &rtv );
	if( FAILED( hr ) ) {
		Game::GetLogger().FatalError( L"Creating Default RTV failed" );
		return;
	}
	ShaderResourceView* srv;
	hr = m_Device->CreateShaderResourceView( backBufferTexture, nullptr, &srv );
	if( FAILED( hr ) ) {
		Game::GetLogger().FatalError( L"Creating SRV for Default RenderTarget failed" );
		return;
	}

	m_DefaultRenderTarget = Game::GetTextureManager().CreateTexture( L"DefaultRenderTarget", backBufferTexture, TextureType::Texture2D, srv, rtv );

	D3D11_TEXTURE2D_DESC temp2dDesc;
	backBufferTexture->GetDesc( &temp2dDesc );
	temp2dDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	TextureDesc tempTexDesc;
	tempTexDesc.UniqueName = L"TempRTV";
	tempTexDesc.TextureType = TextureType::Texture2D;
	tempTexDesc.Texture2DDesc = reinterpret_cast<Texture2DDesc*>( &temp2dDesc );

	m_TempRTV = Texture::Create( tempTexDesc );

	Texture2DDesc desc2d;
	desc2d.BindFlags = BindFlag::DepthStencil | BindFlag::ShaderResource;
	desc2d.Format = Format::R24G8_Typeless;
	desc2d.Usage = Usage::Default;
	desc2d.ArraySize = 1;
	desc2d.MipLevels = 1;
	desc2d.SampleDesc = { 1,0 };
	desc2d.CPUAccessFlags = CPUAccessFlag::None;
	desc2d.MiscFlags = ResourceMiscFlag::None;
	desc2d.Height = Game::GetWindow().GetHeight();
	desc2d.Width = Game::GetWindow().GetWidth();

	TextureDesc texDesc;
	texDesc.Texture2DDesc = &desc2d;
	texDesc.UniqueName = L"DefaultDepthStencil";
	texDesc.TextureType = TextureType::Texture2D;

	DSVDesc dsvDesc;
	dsvDesc.Format = Format::D24_Unorm_S8_UInt;
	dsvDesc.ViewDimension = DSVDimension::Texture2D;
	dsvDesc.Flags = DSVReadOnly::None;
	dsvDesc.Texture2D.MipSlice = 0;

	SRVDesc srvDesc;
	srvDesc.Format = Format::R24_Unorm_X8_Typeless;
	srvDesc.ViewDimension = SRVDimension::Texture2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	texDesc.DSVDesc = &dsvDesc;
	texDesc.SRVDesc = &srvDesc;
	texDesc.Texture2DDesc = &desc2d;

	m_DefaultDepthStencil = Game::GetTextureManager().CreateTexture( texDesc );

	if( !m_DefaultDepthStencil )
		Game::GetLogger().FatalError( L"Default DepthStencil creation failed" );


	desc2d.BindFlags = BindFlag::ShaderResource;
	texDesc.UniqueName = L"TempDSV";

	m_TempDSV = Texture::Create( texDesc );

	desc2d.BindFlags = BindFlag::RenderTarget | BindFlag::ShaderResource;
	desc2d.Format = Format::R32_Float;
	texDesc.UniqueName = L"ShadowTexture";
	texDesc.DSVDesc = nullptr;
	texDesc.SRVDesc = nullptr;

	m_ShadowTexture = Texture::Create( texDesc );
}

void D3DRenderBackend::ResizeRTV_DSV( uint32_t width, uint32_t height ) {
	HRESULT hr;

	m_DefaultRenderTarget->ReleaseResources();

	hr = m_SwapChain->ResizeBuffers( 1, width, height, DXGI_FORMAT_UNKNOWN, 0 );
	if( FAILED( hr ) ) {
		Game::GetLogger().Log( L"RenderBackend", L"Resizing failed" );
		return;
	}

	ID3D11Texture2D* backBufferTexture;
	hr = m_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&backBufferTexture );
	if( FAILED( hr ) ) {
		Game::GetLogger().FatalError( L"Getting BackBuffer failed" );
		return;
	}

	RenderTargetView* rtv;
	hr = m_Device->CreateRenderTargetView( backBufferTexture, nullptr, &rtv );
	if( FAILED( hr ) ) {
		Game::GetLogger().FatalError( L"Creating Default RTV failed" );
		return;
	}
	ShaderResourceView* srv;
	hr = m_Device->CreateShaderResourceView( backBufferTexture, nullptr, &srv );
	if( FAILED( hr ) ) {
		Game::GetLogger().FatalError( L"Creating SRV for Default RenderTarget failed" );
		return;
	}

	m_DefaultRenderTarget->UpdateResources( reinterpret_cast<TextureResource**>( &backBufferTexture ), &srv, &rtv, nullptr, { width, height, 0 } );

	m_DefaultDepthStencil->ResizeResources( width, height );
	m_TempRTV->ResizeResources( width, height );
	m_TempDSV->ResizeResources( width, height );
	m_ShadowTexture->ResizeResources( width, height );

	SetRenderTarget( { m_DefaultRenderTarget->GetRTV() }, m_DefaultDepthStencil->GetDSV() );
}

IDXGIAdapter * D3DRenderBackend::GetAdapter( IDXGIFactory* factory ) {
	std::vector<IDXGIAdapter*> adapters;
	IDXGIAdapter* adapter;
	while( SUCCEEDED( factory->EnumAdapters( static_cast<UINT>( adapters.size() ), &adapter ) ) ) {
		adapters.push_back( adapter );
	}
	if( adapters.size() == 0 ) {
		Game::GetLogger().FatalError( L"Getting Adapter failed" );
		return nullptr;
	}
	size_t maxVRAM = 0;
	size_t bestAdapter = 0;
	for( size_t i = 0; i < adapters.size(); ++i ) {
		DXGI_ADAPTER_DESC desc;
		adapters[i]->GetDesc( &desc );
		if( desc.DedicatedVideoMemory > maxVRAM ) {
			maxVRAM = desc.DedicatedVideoMemory;
			bestAdapter = i;
		}
	}
	adapter = adapters[bestAdapter];
	DXGI_ADAPTER_DESC desc;
	adapter->GetDesc( &desc );
	std::wstring devName = desc.Description;
	size_t size = desc.DedicatedVideoMemory / 1024 / 1024;
	Game::GetLogger().Log( L"RenderBackend", L"\nDevice name: " + devName + L"\nMemory size: " + std::to_wstring( size ) + L"MB" );
	for( auto adapt : adapters ) {
		if( adapt != adapter )
			adapt->Release();
	}

	return adapter;
}

bool D3DRenderBackend::CreateDeferredRTVs() {
	Texture2DDesc desc2d;
	desc2d.BindFlags = BindFlag::RenderTarget | BindFlag::ShaderResource;
	desc2d.Format = Format::R32G32B32A32_Float;
	desc2d.Usage = Usage::Default;
	desc2d.ArraySize = 1;
	desc2d.MipLevels = 1;
	desc2d.SampleDesc = { 1,0 };
	desc2d.CPUAccessFlags = CPUAccessFlag::None;
	desc2d.MiscFlags = ResourceMiscFlag::None;
	desc2d.Height = Game::GetWindow().GetHeight();
	desc2d.Width = Game::GetWindow().GetWidth();

	TextureDesc texDesc;
	texDesc.Texture2DDesc = &desc2d;
	texDesc.TextureType = TextureType::Texture2D;

	RTVDesc rtvDesc;
	rtvDesc.Format = Format::R32G32B32A32_Float;
	rtvDesc.ViewDimension = RTVDimension::Texture2D;
	rtvDesc.Texture2D.MipSlice = 0;

	SRVDesc srvDesc;
	srvDesc.Format = Format::R32G32B32A32_Float;
	srvDesc.ViewDimension = SRVDimension::Texture2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	texDesc.RTVDesc = &rtvDesc;
	texDesc.SRVDesc = &srvDesc;
	texDesc.Texture2DDesc = &desc2d;

	for( size_t i = 0; i < m_DeferredRTVs.size(); i++ ) {
		texDesc.UniqueName = L"DeferredRTV" + std::to_wstring( i );
		if( !( m_DeferredRTVs[i] = Texture::Create( texDesc ) ) )
			return false;
	}

	RegisterResizeCallBack( this, [&]( uint2 size ) {
		for( size_t i = 0; i < m_DeferredRTVs.size(); i++ ) {
			m_DeferredRTVs[i]->ResizeResources( size.x, size.y );
		}
	} );

	return true;
}

RasterizerState* D3DRenderBackend::GetRasterizeState( RasterizerDesc& desc ) {
	D3D11_RASTERIZER_DESC d3dDesc = desc;
	HRESULT hr;
	RasterizerState* rState;
	hr = m_Device->CreateRasterizerState( &d3dDesc, &rState );
	if( FAILED( hr ) ) {
		Game::GetLogger().FatalError( L"Couldn't create Rasterizer State" );
		return nullptr;
	}
	m_RasterizerStates.push_back( rState );

	return m_RasterizerStates.back();
}

DepthStencilState* D3DRenderBackend::GetDepthStencilState( DepthStencilDesc & desc ) {
	D3D11_DEPTH_STENCIL_DESC d3dDesc = desc;
	HRESULT hr;
	DepthStencilState* dState;
	hr = m_Device->CreateDepthStencilState( &d3dDesc, &dState );
	if( FAILED( hr ) ) {
		Game::GetLogger().FatalError( L"Couldn't create Depth Stencil State" );
		return nullptr;
	}
	m_DepthStencilStates.push_back( dState );

	return m_DepthStencilStates.back();
}

BlendState* D3DRenderBackend::GetBlendState( BlendDesc & desc ) {
	D3D11_BLEND_DESC d3dDesc = desc;
	HRESULT hr;
	BlendState* bState;
	hr = m_Device->CreateBlendState( &d3dDesc, &bState );
	if( FAILED( hr ) ) {
		Game::GetLogger().FatalError( L"Couldn't create Blend State" );
		return nullptr;
	}
	m_BlendStates.push_back( bState );

	return m_BlendStates.back();
}

SamplerState* D3DRenderBackend::GetSamplerState( SamplerDesc & desc ) {
	D3D11_SAMPLER_DESC d3dDesc = desc;
	HRESULT hr;
	SamplerState* sState;
	hr = m_Device->CreateSamplerState( &d3dDesc, &sState );
	if( FAILED( hr ) ) {
		Game::GetLogger().FatalError( L"Couldn't create ShaderSampler State" );
		return nullptr;
	}
	m_SamplerStates.push_back( sState );

	return m_SamplerStates.back();
}

void D3DRenderBackend::SetRenderTarget( const std::vector<RenderTargetView*>& rtvs, DepthStencilView* dsv ) {
	m_Context->OMSetRenderTargets( static_cast<uint32_t>( rtvs.size() ), rtvs.data(), dsv );
}

void D3DRenderBackend::SetDeferredRTVs() {
	std::vector<RenderTargetView*> rtvs = { m_DeferredRTVs[0]->GetRTV(), m_DeferredRTVs[1]->GetRTV() };
	m_Context->OMSetRenderTargets( static_cast<UINT>( rtvs.size() ), rtvs.data(), m_DefaultDepthStencil->GetDSV() );
}

void D3DRenderBackend::SetViewports( const std::vector<Viewport>& viewports ) {
	m_Context->RSSetViewports( static_cast<uint32_t>( viewports.size() ), reinterpret_cast<const D3D11_VIEWPORT*>( viewports.data() ) );
}

void D3DRenderBackend::SetRasterizerState( RasterizerState* rasterizerState ) {
	m_Context->RSSetState( rasterizerState );
}

void D3DRenderBackend::SetDepthStencilState( DepthStencilState* depthStencilState ) {
	m_Context->OMSetDepthStencilState( depthStencilState, 0 );
}

void D3DRenderBackend::SetBlendState( BlendState* blendState, const float* blendFactor, UINT sampleMask ) {
	m_Context->OMSetBlendState( blendState, blendFactor, sampleMask );
}

void D3DRenderBackend::SetTopology( PrimitiveTopology topology ) {
	m_Context->IASetPrimitiveTopology( static_cast<D3D11_PRIMITIVE_TOPOLOGY>( topology ) );
}

void D3DRenderBackend::SetVertexBuffer( UINT startIdx, std::vector<Buffer*> buffers, std::vector<UINT> strides, std::vector<UINT> offsets ) {
	m_Context->IASetVertexBuffers( startIdx, static_cast<UINT>( buffers.size() ), buffers.data(), strides.data(), offsets.data() );
}

void D3DRenderBackend::SetIndexBuffer( Buffer* buffer, Format format, uint32_t offset ) {
	m_Context->IASetIndexBuffer( buffer, static_cast<DXGI_FORMAT>( format ), offset );
}

void D3DRenderBackend::SetSamplerVS( uint16_t startSlot, const std::vector<SamplerState*>& samplers ) {
	m_Context->VSSetSamplers( startSlot, static_cast<uint32_t>( samplers.size() ), samplers.data() );
}

void D3DRenderBackend::SetSamplerPS( uint16_t startSlot, const std::vector<SamplerState*>& samplers ) {
	m_Context->PSSetSamplers( startSlot, static_cast<uint32_t>( samplers.size() ), samplers.data() );
}

void D3DRenderBackend::SetSamplerGS( uint16_t startSlot, const std::vector<SamplerState*>& samplers ) {
	m_Context->GSSetSamplers( startSlot, static_cast<uint32_t>( samplers.size() ), samplers.data() );
}

void D3DRenderBackend::SetSamplerHS( uint16_t startSlot, const std::vector<SamplerState*>& samplers ) {
	m_Context->HSSetSamplers( startSlot, static_cast<uint32_t>( samplers.size() ), samplers.data() );
}

void D3DRenderBackend::SetSamplerDS( uint16_t startSlot, const std::vector<SamplerState*>& samplers ) {
	m_Context->DSSetSamplers( startSlot, static_cast<uint32_t>( samplers.size() ), samplers.data() );
}

void D3DRenderBackend::SetSamplerCS( uint16_t startSlot, const std::vector<SamplerState*>& samplers ) {
	m_Context->CSSetSamplers( startSlot, static_cast<uint32_t>( samplers.size() ), samplers.data() );
}

void D3DRenderBackend::ClearRenderTarget( RenderTargetView* rtv, const Color & clearColor ) {
	const float* fClearColor = reinterpret_cast<const float*>( &clearColor );
	m_Context->ClearRenderTargetView( rtv, fClearColor );
}

void D3DRenderBackend::ClearDeferredRTV( const Color & clearColor ) {
	m_Context->ClearRenderTargetView( m_DeferredRTVs[0]->GetRTV(), reinterpret_cast<const float*>( &clearColor ) );
	float normcolor[4] = { 1.f,1.f,1.f,1.f };
	m_Context->ClearRenderTargetView( m_DeferredRTVs[1]->GetRTV(), normcolor );
}

void D3DRenderBackend::ClearDepthStencil( DepthStencilView* dsv, bool clearDepth, bool clearStencil, float depthVal, uint8_t stencilVal ) {
	UINT flags = 0;
	if( clearDepth )
		flags |= D3D11_CLEAR_DEPTH;
	if( clearStencil )
		flags |= D3D11_CLEAR_STENCIL;
	m_Context->ClearDepthStencilView( dsv, flags, depthVal, stencilVal );
}

void D3DRenderBackend::ClearUAV( UnorderedAccessView* uav, const float clearVal[4] ) {
	m_Context->ClearUnorderedAccessViewFloat( uav, clearVal );
}

void D3DRenderBackend::ClearUAV( UnorderedAccessView* uav, const uint32_t clearVal[4] ) {
	m_Context->ClearUnorderedAccessViewUint( uav, clearVal );
}

void D3DRenderBackend::GetTextureDesc( TextureResource* tex, void* desc ) {
	if( !desc )
		nullptr;
	D3D11_RESOURCE_DIMENSION type;
	tex->GetType( &type );
	switch( type ) {
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			Texture1D* tex1d = reinterpret_cast<Texture1D*>( tex );
			tex1d->GetDesc( reinterpret_cast<D3D11_TEXTURE1D_DESC*>( desc ) );
			break;
		}
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			Texture2D* tex2d = reinterpret_cast<Texture2D*>( tex );
			tex2d->GetDesc( reinterpret_cast<D3D11_TEXTURE2D_DESC*>( desc ) );
			break;
		}
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			Texture3D* tex3d = reinterpret_cast<Texture3D*>( tex );
			tex3d->GetDesc( reinterpret_cast<D3D11_TEXTURE3D_DESC*>( desc ) );
			break;
		}
		default:
			break;
	}
}

uint3 D3DRenderBackend::GetTextureDim( TextureResource* tex ) {
	D3D11_RESOURCE_DIMENSION type;
	tex->GetType( &type );
	uint3 dim = { 1,1,1 };
	switch( type ) {
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		{
			Texture1D* tex1d = reinterpret_cast<Texture1D*>( tex );
			D3D11_TEXTURE1D_DESC desc1d;
			tex1d->GetDesc( &desc1d );
			dim.x = desc1d.Width;
			break;
		}
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			Texture2D* tex2d = reinterpret_cast<Texture2D*>( tex );
			D3D11_TEXTURE2D_DESC desc2d;
			tex2d->GetDesc( &desc2d );
			dim.x = desc2d.Width;
			dim.y = desc2d.Height;
			break;
		}
		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		{
			Texture3D* tex3d = reinterpret_cast<Texture3D*>( tex );
			D3D11_TEXTURE3D_DESC desc3d;
			tex3d->GetDesc( &desc3d );
			dim.x = desc3d.Width;
			dim.y = desc3d.Height;
			dim.z = desc3d.Depth;
			break;
		}
		default:
			break;
	}
	return dim;
}

void D3DRenderBackend::GetSRVDesc( ShaderResourceView* srv, SRVDesc & desc ) {
	srv->GetDesc( reinterpret_cast<D3D11_SHADER_RESOURCE_VIEW_DESC*>( &desc ) );
}

void D3DRenderBackend::GetRTVDesc( RenderTargetView* rtv, RTVDesc & desc ) {
	rtv->GetDesc( reinterpret_cast<D3D11_RENDER_TARGET_VIEW_DESC*>( &desc ) );
}

void D3DRenderBackend::GetDSVDesc( DepthStencilView* dsv, DSVDesc & desc ) {
	dsv->GetDesc( reinterpret_cast<D3D11_DEPTH_STENCIL_VIEW_DESC*>( &desc ) );
}

void D3DRenderBackend::GetUAVDesc( UnorderedAccessView * uav, UAVDesc & desc ) {
	uav->GetDesc( reinterpret_cast<D3D11_UNORDERED_ACCESS_VIEW_DESC*>( &desc ) );
}

bool D3DRenderBackend::CreateTexture( const TextureDesc & texDesc, TextureResource** tex, ShaderResourceView** srv, RenderTargetView** rtv, DepthStencilView** dsv, UnorderedAccessView** uav ) {
	assert( tex );
	*tex = nullptr;
	HRESULT hr;
	BindFlag flags = BindFlag::None;
	switch( texDesc.TextureType ) {
		case TextureType::Texture1D:
		case TextureType::Texture1DArray:
		{
			if( !texDesc.Texture1DDesc )
				return false;
			D3D11_TEXTURE1D_DESC d3dTexDesc = *texDesc.Texture1DDesc;
			flags = texDesc.Texture1DDesc->BindFlags;
			Texture1D* tex1DResource;
			hr = m_Device->CreateTexture1D( &d3dTexDesc, texDesc.InitialData, &tex1DResource );
			*tex = tex1DResource;
			break;
		}
		case TextureType::Texture2D:
		case TextureType::Texture2DArray:
		case TextureType::TextureCube:
		{
			if( !texDesc.Texture2DDesc )
				return false;
			D3D11_TEXTURE2D_DESC d3dTexDesc = *texDesc.Texture2DDesc;
			flags = texDesc.Texture2DDesc->BindFlags;
			Texture2D* tex2DResource;
			hr = m_Device->CreateTexture2D( &d3dTexDesc, texDesc.InitialData, &tex2DResource );
			*tex = tex2DResource;
			break;
		}
		case TextureType::Texture3D:
		{
			if( !texDesc.Texture3DDesc )
				return false;
			D3D11_TEXTURE3D_DESC d3dTexDesc = *texDesc.Texture3DDesc;
			flags = texDesc.Texture3DDesc->BindFlags;
			Texture3D* tex3DResource;
			hr = m_Device->CreateTexture3D( &d3dTexDesc, texDesc.InitialData, &tex3DResource );
			*tex = tex3DResource;
			break;
		}
		default:
			hr = E_INVALIDARG;
			break;
	}

	if( FAILED( hr ) ) {
		Game::GetLogger().Log( L"Texture", L"Creation of Texture with unique name \"" + texDesc.UniqueName + L"\" failed." );
		SRelease( *tex );
		return false;
	}


	if( srv ) {
		if( CheckFlag( flags, BindFlag::ShaderResource ) ) {
			if( texDesc.SRVDesc ) {
				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = *texDesc.SRVDesc;
				hr = m_Device->CreateShaderResourceView( *tex, &srvDesc, srv );
			}
			else
				hr = m_Device->CreateShaderResourceView( *tex, nullptr, srv );
			if( FAILED( hr ) ) {
				Game::GetLogger().Log( L"Texture", L"Creation of SRV for Texture with unique name \"" + texDesc.UniqueName + L"\" failed." );
				SRelease( *tex );
				SRelease( *srv );
				return false;
			}
		}
		else
			*srv = nullptr;
	}
	if( rtv ) {
		if( CheckFlag( flags, BindFlag::RenderTarget ) ) {
			if( texDesc.RTVDesc ) {
				D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = *texDesc.RTVDesc;
				hr = m_Device->CreateRenderTargetView( *tex, &rtvDesc, rtv );
			}
			else
				hr = m_Device->CreateRenderTargetView( *tex, nullptr, rtv );
			if( FAILED( hr ) ) {
				Game::GetLogger().Log( L"Texture", L"Creation of RTV for Texture with unique name \"" + texDesc.UniqueName + L"\" failed." );
				SRelease( *tex );
				if( srv )
					SRelease( *srv );
				SRelease( *rtv );
				return false;
			}
		}
		else
			*rtv = nullptr;
	}
	if( dsv ) {
		if( CheckFlag( flags, BindFlag::DepthStencil ) ) {
			if( texDesc.DSVDesc ) {
				D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = *texDesc.DSVDesc;
				hr = m_Device->CreateDepthStencilView( *tex, &dsvDesc, dsv );
			}
			else
				hr = m_Device->CreateDepthStencilView( *tex, nullptr, dsv );
			if( FAILED( hr ) ) {
				Game::GetLogger().Log( L"Texture", L"Creation of DSV for Texture with unique name \"" + texDesc.UniqueName + L"\" failed." );
				SRelease( *tex );
				if( srv )
					SRelease( *srv );
				if( rtv )
					SRelease( *rtv );
				SRelease( *dsv );
				return false;
			}
		}
		else
			*dsv = nullptr;
	}
	if( uav ) {
		if( CheckFlag( flags, BindFlag::UnorderedAccess ) ) {
			hr = m_Device->CreateUnorderedAccessView( *tex, reinterpret_cast<D3D11_UNORDERED_ACCESS_VIEW_DESC*>( texDesc.UAVDesc ), uav );
			if( FAILED( hr ) ) {
				Game::GetLogger().Log( L"Texture", L"Creation of UAV for Texture with unique name \"" + texDesc.UniqueName + L"\" failed." );
				SRelease( *tex );
				if( srv )
					SRelease( *srv );
				if( rtv )
					SRelease( *rtv );
				if( dsv )
					SRelease( *dsv );
				SRelease( *uav );
				return false;
			}
		}
		else
			*uav = nullptr;
	}

	return true;
}

UnorderedAccessView* D3DRenderBackend::CreateUAV( Resource* resource, const UAVDesc* desc ) {
	UnorderedAccessView* uav;
	HRESULT hr = m_Device->CreateUnorderedAccessView( resource, reinterpret_cast<const D3D11_UNORDERED_ACCESS_VIEW_DESC*>( desc ), &uav );
	if( FAILED( hr ) ) {
		Game::GetLogger().Log( L"RenderBackend", L"Failed Creating UAV" );
		return nullptr;
	}
	return uav;
}

ShaderResourceView* D3DRenderBackend::CreateSRV( Resource* resource, const SRVDesc * desc ) {
	ShaderResourceView *srv;
	HRESULT hr = m_Device->CreateShaderResourceView( resource, reinterpret_cast<const D3D11_SHADER_RESOURCE_VIEW_DESC*>( desc ), &srv );
	if( FAILED( hr ) ) {
		Game::GetLogger().Log( L"RenderBackend", L"Failed Creating SRV" );
		return nullptr;
	}
	return srv;
}

RenderTargetView * D3DRenderBackend::CreateRTV( Resource * resource, const RTVDesc * desc ) {
	RenderTargetView *rtv;
	HRESULT hr = m_Device->CreateRenderTargetView( resource, reinterpret_cast<const D3D11_RENDER_TARGET_VIEW_DESC*>( desc ), &rtv );
	if( FAILED( hr ) ) {
		Game::GetLogger().Log( L"RenderBackend", L"Failed Creating SRV" );
		return nullptr;
	}
	return rtv;
}

DepthStencilView * D3DRenderBackend::CreateDSV( Resource * resource, const DSVDesc * desc ) {
	DepthStencilView *dsv;
	HRESULT hr = m_Device->CreateDepthStencilView( resource, reinterpret_cast<const D3D11_DEPTH_STENCIL_VIEW_DESC*>( desc ), &dsv );
	if( FAILED( hr ) ) {
		Game::GetLogger().Log( L"RenderBackend", L"Failed Creating SRV" );
		return nullptr;
	}
	return dsv;
}

Buffer* D3DRenderBackend::CreateBuffer( const void * data, const BufferDesc& desc ) {
	HRESULT hr;
	D3D11_BUFFER_DESC d3dDesc = desc;

	Buffer* buffer;
	if( data ) {
		D3D11_SUBRESOURCE_DATA resourceData;
		ZeroMemory( &resourceData, sizeof( resourceData ) );
		resourceData.pSysMem = data;
		hr = m_Device->CreateBuffer( &d3dDesc, &resourceData, &buffer );
	}
	else
		hr = m_Device->CreateBuffer( &d3dDesc, nullptr, &buffer );
	if( FAILED( hr ) ) {
		Game::GetLogger().FatalError( L"Failed creating Buffer" );
		return nullptr;
	}

	m_Buffers.push_back( buffer );
	return m_Buffers.back();
}

void D3DRenderBackend::CopyResource( Resource * dst, Resource * src ) {
	m_Context->CopyResource( dst, src );
}

void D3DRenderBackend::UpdateBuffer( Buffer* buffer, void * data, UINT bufferSize, UINT subresource, MapType mapType, bool doNotWait ) {
	UINT mapflags = doNotWait ? D3D11_MAP_FLAG_DO_NOT_WAIT : 0;

	D3D11_MAPPED_SUBRESOURCE resource;
	m_Context->Map( buffer, subresource, static_cast<D3D11_MAP>( mapType ), mapflags, &resource );

	memcpy( resource.pData, data, bufferSize );

	m_Context->Unmap( buffer, subresource );
}

void D3DRenderBackend::ReadBuffer( Buffer * buffer, UINT bufferSize, void* out, UINT cpySize, UINT subresource ) {
	BufferDesc bDesc;
	bDesc.ByteWidth = bufferSize;
	bDesc.Usage = Usage::Staging;
	bDesc.CPUAccessFlags = CPUAccessFlag::Read;

	Buffer* tempBuffer;
	m_Device->CreateBuffer( reinterpret_cast<D3D11_BUFFER_DESC*>( &bDesc ), nullptr, &tempBuffer );

	if( !tempBuffer )
		return;

	m_Context->CopyResource( tempBuffer, buffer );

	D3D11_MAPPED_SUBRESOURCE resource;
	HRESULT hr = m_Context->Map( tempBuffer, subresource, D3D11_MAP_READ, 0, &resource );

	memcpy( out, resource.pData, cpySize );

	m_Context->Unmap( tempBuffer, subresource );

	tempBuffer->Release();
}

void D3DRenderBackend::MapBuffer( Buffer * buffer, void ** out, UINT subresource, MapType mapType ) {
	D3D11_MAPPED_SUBRESOURCE resource;
	if( FAILED( m_Context->Map( buffer, subresource, static_cast<D3D11_MAP>( mapType ), 0, &resource ) ) ) {
		out = nullptr;
		return;
	}
	*out = resource.pData;
}

void D3DRenderBackend::UnmapBuffer( Buffer * buffer, UINT subresource ) {
	m_Context->Unmap( buffer, subresource );
}

void D3DRenderBackend::UpdateSubresource( Resource* resource, UINT subresource, Box * box, void * data, UINT srcRowPitch, UINT srcDepthPitch ) {
	m_Context->UpdateSubresource( resource, subresource, box, data, srcRowPitch, srcDepthPitch );
}

void D3DRenderBackend::BindConstantBuffer( Buffer* buffer, ShaderFlag bindToShaders, UINT position ) {
	if( CheckFlag( bindToShaders, ShaderFlag::VertexShader ) )
		m_Context->VSSetConstantBuffers( position, 1, &buffer );
	if( CheckFlag( bindToShaders, ShaderFlag::PixelShader ) )
		m_Context->PSSetConstantBuffers( position, 1, &buffer );
	if( CheckFlag( bindToShaders, ShaderFlag::ComputeShader ) )
		m_Context->CSSetConstantBuffers( position, 1, &buffer );
	if( CheckFlag( bindToShaders, ShaderFlag::GeometryShader ) )
		m_Context->GSSetConstantBuffers( position, 1, &buffer );
	if( CheckFlag( bindToShaders, ShaderFlag::HullShader ) )
		m_Context->HSSetConstantBuffers( position, 1, &buffer );
	if( CheckFlag( bindToShaders, ShaderFlag::DomainShader ) )
		m_Context->DSSetConstantBuffers( position, 1, &buffer );
}

void D3DRenderBackend::Draw( UINT vertexCount, UINT startLocation ) {
	m_Context->Draw( vertexCount, startLocation );
}

void D3DRenderBackend::DrawIndexed( UINT indexCount, UINT startIndexLocation, int baseVertexLocation ) {
	m_Context->DrawIndexed( indexCount, startIndexLocation, baseVertexLocation );
}

void D3DRenderBackend::Dispatch( UINT groupCountX, UINT groupCountY, UINT groupCountZ ) {
	m_Context->Dispatch( groupCountX, groupCountY, groupCountZ );
}

void D3DRenderBackend::SetSRVsVS( uint16_t startSlot, const std::vector<ShaderResourceView*>& srvs ) {
	m_Context->VSSetShaderResources( startSlot, static_cast<uint32_t>( srvs.size() ), srvs.data() );
}

void D3DRenderBackend::SetSRVsPS( uint16_t startSlot, const std::vector<ShaderResourceView*>& srvs ) {
	m_Context->PSSetShaderResources( startSlot, static_cast<uint32_t>( srvs.size() ), srvs.data() );
}

void D3DRenderBackend::SetSRVsGS( uint16_t startSlot, const std::vector<ShaderResourceView*>& srvs ) {
	m_Context->GSSetShaderResources( startSlot, static_cast<uint32_t>( srvs.size() ), srvs.data() );
}

void D3DRenderBackend::SetSRVsHS( uint16_t startSlot, const std::vector<ShaderResourceView*>& srvs ) {
	m_Context->HSSetShaderResources( startSlot, static_cast<uint32_t>( srvs.size() ), srvs.data() );
}

void D3DRenderBackend::SetSRVsDS( uint16_t startSlot, const std::vector<ShaderResourceView*>& srvs ) {
	m_Context->DSSetShaderResources( startSlot, static_cast<uint32_t>( srvs.size() ), srvs.data() );
}

void D3DRenderBackend::SetSRVsCS( uint16_t startSlot, const std::vector<ShaderResourceView*>& srvs ) {
	m_Context->CSSetShaderResources( startSlot, static_cast<uint32_t>( srvs.size() ), srvs.data() );
}

void D3DRenderBackend::SetUAVPS( uint16_t startSlot, const std::vector<UnorderedAccessView*>& uavs, const std::vector<uint32_t>& initialCounts ) {
	m_Context->OMSetRenderTargetsAndUnorderedAccessViews( D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, startSlot, static_cast<uint32_t>( uavs.size() ), uavs.data(), initialCounts.data() );
}

void D3DRenderBackend::SetUAVCS( uint16_t startSlot, const std::vector<UnorderedAccessView*>& uavs, const std::vector<uint32_t>& initialCounts ) {
	m_Context->CSSetUnorderedAccessViews( startSlot, static_cast<uint32_t>( uavs.size() ), uavs.data(), initialCounts.data() );
}

void D3DRenderBackend::SetRTV_UAV( const std::vector<RenderTargetView*>& rtvs, DepthStencilView* dsv, uint16_t uavStartSlot,
								   const std::vector<UnorderedAccessView*>& uavs, const std::vector<uint32_t>& initialCounts ) {
	m_Context->OMSetRenderTargetsAndUnorderedAccessViews( static_cast<uint32_t>( rtvs.size() ), rtvs.data(), dsv, uavStartSlot, static_cast<uint32_t>( uavs.size() ), uavs.data(), initialCounts.data() );
}
