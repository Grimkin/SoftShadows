#include "RenderPass.h"

#include <cassert>

#include "Shader.h"
#include "RenderBackend.h"
#include "Logger.h"
#include "Renderable.h"
#include "Geometry.h"
#include "Renderer.h"
#include "Makros.h"

RenderPassManager* RenderPass::s_RenderPassManager = nullptr;

RenderPass * RenderPass::Create( const RenderPassInit & init ) {
	return s_RenderPassManager->CreatePass( init );
}

RenderPass * RenderPass::Get( const std::wstring & name ) {
	return s_RenderPassManager->GetPass( name );
}

RenderPass::RenderPass( RenderPassInit& init )
	: m_Name( init.Name )
	, m_PreFunction( init.PreFunction )
	, m_ShowTiming( init.ShowTiming )
	, m_TimingName( init.TimingName.empty() ? init.Name : init.TimingName ){
	assert( !m_Name.empty() );

	m_HashValue = std::hash<RenderPassInit>()( init );

	if( init.VertexShader ) {
		assert( init.VertexShader->GetShaderType() == ShaderType::VertexShader );
		m_Shaders.push_back( init.VertexShader );
	}
	if( init.PixelShader ) {
		assert( init.PixelShader->GetShaderType() == ShaderType::PixelShader );
		m_Shaders.push_back( init.PixelShader );
	}
	if( init.GeometryShader ) {
		assert( init.GeometryShader->GetShaderType() == ShaderType::GeometryShader );
		m_Shaders.push_back( init.GeometryShader );
	}
	if( init.HullShader ) {
		assert( init.HullShader->GetShaderType() == ShaderType::HullShader );
		m_Shaders.push_back( init.HullShader );
	}
	if( init.DomainShader ) {
		assert( init.DomainShader->GetShaderType() == ShaderType::DomainShader );
		m_Shaders.push_back( init.DomainShader );
	}

	bool success;
	success = InitPipeline( init );
	if( !success )
		Game::GetLogger().Log( L"Renderpass", L"Error while creating Renderpass" + m_Name );

	Game::GetRenderer().AddRenderPass( *this, init.RenderPassType, init.RenderPassOrderIndex );
}


RenderPass::~RenderPass() {
}

void RenderPass::Execute( RenderFlag flags ) {
	Apply( flags );

	for( Renderable* renderable : m_AssignedRenderables ) {
		if( renderable->IsVisible() )
			RenderObject( *renderable );
	}
}

void RenderPass::ExecuteDraw( RenderFlag flags, uint32_t numVertices ) {
	Apply( flags );

	Game::GetRenderBackend().Draw( numVertices, 0 );
}

void RenderPass::Apply( RenderFlag flags ) {
	Shader::ResetShaders();
	for( Shader* shader : m_Shaders ) {
		shader->SetShader();
	}

	RenderBackend& renderBackend = Game::GetRenderBackend();
	renderBackend.SetRasterizerState( m_RasterizerState );
	renderBackend.SetDepthStencilState( m_DepthStencilState );
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	UINT sampleMask = 0xffffffff;
	renderBackend.SetBlendState( m_BlendState, blendFactor, sampleMask );

	if( m_PreFunction )
		m_PreFunction();
}

void RenderPass::Assign( Renderable* renderable ) {
	if( !renderable )
		return;
	m_AssignedRenderables.push_back( renderable );
	renderable->m_AssignedRenderPasses.push_back( this );
	m_RenderablePosition[renderable] = static_cast<uint32_t>( m_AssignedRenderables.size() - 1 );
}

void RenderPass::Deassign( const Renderable & renderable ) {
	auto position = m_RenderablePosition.find( &renderable );
	if( position != m_RenderablePosition.end() ) {
		assert( &renderable == m_AssignedRenderables[position->second] );
		if( m_AssignedRenderables[position->second] == m_AssignedRenderables.back() ) {
			m_AssignedRenderables.pop_back();
			m_RenderablePosition.erase( &renderable );
		}
		else {
			std::swap( m_AssignedRenderables[position->second], m_AssignedRenderables.back() );
			m_RenderablePosition[m_AssignedRenderables[position->second]] = position->second;
			m_RenderablePosition.erase( &renderable );
			m_AssignedRenderables.pop_back();
		}
	}
}

bool RenderPass::InitPipeline( RenderPassInit & init ) {
	RenderBackend& renderBackend = Game::GetRenderBackend();
	m_RasterizerState = renderBackend.GetRasterizeState( init.RasterizerDesc );
	if( !m_RasterizerState )
		return false;
	m_DepthStencilState = renderBackend.GetDepthStencilState( init.DepthStencilDesc );
	if( !m_DepthStencilState )
		return false;
	m_BlendState = renderBackend.GetBlendState( init.BlendDesc );
	if( !m_BlendState )
		return false;
	return true;
}

void RenderPass::RenderObject( Renderable & renderable ) {
	renderable.PrepareForRender();
	renderable.Render();
	renderable.PostRender();
}

RenderPassManager & RenderPassManager::Init() {
	static RenderPassManager renderPassManager;
	return renderPassManager;
}

RenderPass * RenderPassManager::GetPass( std::wstring uniqueName ) {
	auto pass = m_RenderPasses.find( uniqueName );
	if( pass == m_RenderPasses.end() ) {
		Game::GetLogger().Log( L"Renderpass", L"Attempt to get Renderpass " + uniqueName + L" failed" );
		return nullptr;
	}
	return pass->second;
}

RenderPass* RenderPassManager::CreatePass( RenderPassInit renderPassInit ) {
	// check for double unique name existance
	if( m_RenderPasses.find( renderPassInit.Name ) != m_RenderPasses.end() ) {
		Game::GetLogger().Log( L"Renderpass", L"RenderPass with name \"" + renderPassInit.Name + L"\" already exists. Choose different name or use GetPass() to use the pass" );
		return m_RenderPasses[renderPassInit.Name];
	}

	// check for same renderpass type existance
	size_t hash = std::hash<RenderPassInit>()( renderPassInit );
	auto  pass = m_UniqueRenderPasses.find( hash );
	if( pass != m_UniqueRenderPasses.end() ) {
		Game::GetLogger().Log( L"Renderpass", L"Attempt to create renderPass with same type already existing. Existing name: \"" + pass->second->m_Name + L"\". Consider using this." );
		m_RenderPasses[renderPassInit.Name] = pass->second;
		return pass->second;
	}

	// create new Renderpass
	RenderPass* newRenderPass = new RenderPass( renderPassInit );
	m_RenderPasses[renderPassInit.Name] = newRenderPass;
	m_UniqueRenderPasses[hash] = newRenderPass;
	return newRenderPass;
}

RenderPassManager::RenderPassManager() {
	Game::SetRenderPassManager( *this );
	RenderPass::s_RenderPassManager = this;
}

RenderPassManager::~RenderPassManager() {
	SDeleteMap<size_t,RenderPass>( m_UniqueRenderPasses, []( RenderPass* pass ) { delete pass; } );
}
