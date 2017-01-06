#include "Renderer.h"

#include "RenderPass.h"
#include "Game.h"
#include "Camera.h"
#include "GUI.h"
#include "Time.h"
#include "Profiler.h"
#include "imgui.h"


Renderer& Renderer::Init() {
	static Renderer renderer;
	return renderer;
}

void Renderer::Start() {
	//m_FPSCounter = GUIText::Create( L"", { 0.05f, 0.05f }, 0, 20.0f, Alignment::Right, { 0.0f, 1.0f, 0.0f, 1.0f } );
}

void Renderer::Render() {
	assert( m_ActiveCamera );

	m_ActiveCamera->PrepareRender();

	if( m_ClearRenderTarget ) {
		for( RenderTargetView* rtv : m_ActiveCamera->GetRTVs() ) {
			Game::GetRenderBackend().ClearRenderTarget( rtv, { 0.0f, 0.0f, 1.0f, 1.0f } );
		}
	}
	if( m_ClearDepthStencil ) {
		Game::GetRenderBackend().ClearDepthStencil( m_ActiveCamera->GetDSV(), true, true );
	}

	RenderOpaque();
	RenderTransparent();
	PostRender();
}

void Renderer::Update() {
	//m_FPSCounter->SetText( std::to_wstring( std::min( static_cast<int>( Game::GetTime().GetFPS() ), 999 ) ) );
}

// Called by Create Renderpass so no need to call it manually
void Renderer::AddRenderPass( RenderPass& renderPass, RenderPassType renderPassType, int8_t idx ) {
	switch( renderPassType ) {
		case RenderPassType::Opaque:
			m_OpaqueRenderPasses.insert( { idx, &renderPass } );
			break;
		case RenderPassType::Transparency:
			m_TransparencyRenderPasses.insert( { idx, &renderPass } );
			break;
		case RenderPassType::Post:
			m_PostRenderPasses.insert( { idx, &renderPass } );
			break;
		default:
			break;
	}
}

void Renderer::SetActiveCamera( Camera& camera ) {
	m_ActiveCamera = &camera;

	RenderBackend* renderBackend = &Game::GetRenderBackend();
	renderBackend->SetRenderTarget( m_ActiveCamera->GetRTVs(), m_ActiveCamera->GetDSV() );
	renderBackend->SetViewports( { m_ActiveCamera->GetViewport() } );
}

void Renderer::OnResize() {
	RenderBackend* renderBackend = &Game::GetRenderBackend();
	renderBackend->SetRenderTarget( m_ActiveCamera->GetRTVs(), m_ActiveCamera->GetDSV() );
	renderBackend->SetViewports( { m_ActiveCamera->GetViewport() } );
}

void Renderer::SetLight( float3 invDirection, Color lightColor ) {
	m_LightDir = Normalize( invDirection );
	m_LightColor = lightColor;
	UpdateLightBuffer();
}

void Renderer::SetShadowParams( uint32_t numSamples, float lightAngle ) {
	m_NumLightSamples = numSamples;
	m_LightAngle = lightAngle;
	UpdateLightBuffer();
}

Renderer::Renderer() {
	Game::SetRenderer( *this );
}


Renderer::~Renderer() {
}

void Renderer::RenderOpaque() {
	for( auto pass : m_OpaqueRenderPasses ) {
		if( pass.second->ShowTiming() )
			Profiler::GlobalProfiler.StartProfile( pass.second->GetTimingName(), false );
		pass.second->Execute( RenderFlag::None );
		if( pass.second->ShowTiming() )
			ImGui::Text( "%s: %fms", ws2s( pass.second->GetTimingName() ).c_str(), Profiler::GlobalProfiler.EndProfile( pass.second->GetTimingName() ) );
	}
}

void Renderer::RenderTransparent() {
	for( auto pass : m_TransparencyRenderPasses ) {
		if( pass.second->ShowTiming() )
			Profiler::GlobalProfiler.StartProfile( pass.second->GetTimingName(), false );
		pass.second->Execute( RenderFlag::None );
		if( pass.second->ShowTiming() )
			ImGui::Text( "%s: %fms", ws2s( pass.second->GetTimingName() ).c_str(), Profiler::GlobalProfiler.EndProfile( pass.second->GetTimingName() ) );
	}
}

void Renderer::PostRender() {
	Game::GetRenderBackend().SetTopology( PrimitiveTopology::Trianglelist );
	for( auto pass : m_PostRenderPasses ) {
		Game::GetRenderBackend().CopyRTV_DSV();
		if( pass.second->ShowTiming() )
			Profiler::GlobalProfiler.StartProfile( pass.second->GetTimingName(), false );
		pass.second->ExecuteDraw( RenderFlag::None, 4 );
		if( pass.second->ShowTiming() )
			ImGui::Text( "%s: %fms", ws2s( pass.second->GetTimingName() ).c_str(), Profiler::GlobalProfiler.EndProfile( pass.second->GetTimingName() ) );
	}
}

void Renderer::UpdateLightBuffer() {
	LightData data;
	data.InvLightDir = m_LightDir;
	data.LightColor = m_LightColor;
	data.NumSamples = m_NumLightSamples;
	data.JitterRad = tan( Deg2Rad( m_LightAngle ) );
	data.LightAngle = Deg2Rad( m_LightAngle );
	m_LightBuffer.Update( data );
	m_LightBuffer.Bind( ShaderFlag::PixelShader, 2 );
}
