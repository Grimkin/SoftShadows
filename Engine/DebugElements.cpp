#include "DebugElements.h"

#include "GameObject.h"
#include "Geometry.h"
#include "GameObjectManager.h"
#include "Texture.h"
#include "RenderPass.h"
#include "RenderBackend.h"

Geometry* DebugCube::s_Geometry = nullptr;

DebugElementsManager* DebugCube::s_Manager = nullptr;
DebugElementsManager* DebugLine::s_Manager = nullptr;

DebugElementsManager& DebugElementsManager::Init( RenderBackend& renderBackend) {
	static DebugElementsManager manager( renderBackend );
	return manager;
}

void DebugElementsManager::RegisterCube( DebugCube& cube ) {
	m_DebugCubes.push_back( &cube );
}

void DebugElementsManager::RegisterLine( DebugLine& line ) {
	m_DebugLines.push_back( &line );
	UpdateLineBuffer();
}

void DebugElementsManager::DeleteCube( DebugCube & cube ) {
	for( size_t i = 0; i < m_DebugCubes.size(); ++i ) {
		if( &cube == m_DebugCubes[i] ) {
			m_DebugCubes[i] = m_DebugCubes.back();
			m_DebugCubes.pop_back();
		}
	}
}

void DebugElementsManager::DeleteLine( DebugLine & line ) {
	for( size_t i = 0; i < m_DebugLines.size(); ++i ) {
		if( &line == m_DebugLines[i] ) {
			m_DebugLines[i] = m_DebugLines.back();
			m_DebugLines.pop_back();
		}
	}
}

void DebugElementsManager::UpdateLineBuffer() {
	std::vector<float3> vertices;
	for( DebugLine* line : m_DebugLines ) {
		vertices.emplace_back( line->GetStartPosition() );
		vertices.emplace_back( line->GetEndPosition() );
	}
	m_LineGeometry->UpdateBuffer( vertices.data(), static_cast<uint32_t>( vertices.size() ) );
}

void DebugElementsManager::SetLineColor( const Color & color ) {
	m_LineGameObject->GetRenderable()->SetColor( color );
}

DebugElementsManager::DebugElementsManager( RenderBackend& renderBackend)
	: m_RenderBackend( &renderBackend ){
	DebugCube::s_Manager = this;
	DebugLine::s_Manager = this;

	BufferDesc bDesc;
	bDesc.BindFlags = BindFlag::VertexBuffer;
	bDesc.Usage = Usage::Dynamic;
	bDesc.ByteWidth = sizeof( float3 ) * 2 * m_MaximumLines;
	bDesc.CPUAccessFlags = CPUAccessFlag::Write;

	m_LinePointBuffer = m_RenderBackend->CreateBuffer( nullptr, bDesc );

	RenderPassInit rInit;
	rInit.Name = L"PureColor";
	rInit.VertexShader = Shader::Get( L"vsSimple" );
	rInit.PixelShader = Shader::Get( L"psColor" );

	MaterialDesc mDesc;
	mDesc.RenderPass = RenderPass::Create( rInit );

	m_Material = Material::Create( mDesc );

	m_LineGeometry = DynamicGeometry::Create( L"LineGeometry", m_MaximumLines * 2, sizeof( float3 ), PrimitiveTopology::Linelist );

	GameObjectDesc goDesc;
	goDesc.HasRenderable = true;
	goDesc.RenderableDesc.Geometry = m_LineGeometry;
	goDesc.RenderableDesc.Material = m_Material;

	m_LineGameObject = Game::GetGOManager().Instantiate<GameObject>( goDesc );
}

DebugElementsManager::~DebugElementsManager() {
	SDeleteVec<DebugCube>( m_DebugCubes, []( DebugCube* cube ) { delete cube; } );
	SDeleteVec<DebugLine>( m_DebugLines, []( DebugLine* line ) { delete line; } );
}

DebugCube::DebugCube( const float3 & position, const float3 & size, const Color& color ) {
	if( !s_Geometry )
		CreateGeometry();

	s_Manager->RegisterCube( *this );

	GameObjectDesc desc;
	desc.HasRenderable = true;
	desc.Position = position;
	desc.Scale = size;
	desc.RenderableDesc.Material = s_Manager->GetDebugMaterial();
	desc.RenderableDesc.Geometry = s_Geometry;
	desc.RenderableDesc.Color = &color;

	m_GameObject = Game::GetGOManager().Instantiate<GameObject>( desc );
}

DebugCube * DebugCube::Create( const float3 & position, const float3 & size, const Color& color ) {
	return new DebugCube( position, size, color );
}

void DebugCube::Delete() {
	s_Manager->DeleteCube( *this );
	delete this;
}

DebugCube::~DebugCube() {
	m_GameObject->Delete();
}

void DebugCube::CreateGeometry() {
	if( s_Geometry )
		return;

	std::vector<float3> vertices = {
		{ -.5f, -.5f, -.5f },
		{ -.5f, 0.5f, -.5f },
		{ 0.5f, 0.5f, -.5f },
		{ 0.5f, -.5f, -.5f },
		{ -.5f, -.5f, 0.5f },
		{ -.5f, 0.5f, 0.5f },
		{ 0.5f, 0.5f, 0.5f },
		{ 0.5f, -.5f, 0.5f }
	};

	std::vector<uint32_t> indices = {
		0,1,
		1,2,
		2,3,
		3,0,
		4,5,
		5,6,
		6,7,
		7,4,
		0,4,
		1,5,
		2,6,
		3,7
	};

	s_Geometry = Geometry::Create( L"DebugCube", vertices, indices, PrimitiveTopology::Linelist );
}

DebugLine * DebugLine::Create( const float3 & startPosition, const float3 & endPosition, const Color & color ) {
	s_Manager->SetLineColor( color );
	return new DebugLine( startPosition, endPosition, color );
}

void DebugLine::Delete() {
	s_Manager->DeleteLine( *this );
	delete this;
}

DebugLine::DebugLine( const float3 & startPosition, const float3 & endPosition, const Color & color ) 
	: m_StartPosition( startPosition )
	, m_EndPosition( endPosition ) {
	s_Manager->RegisterLine( *this );
}

DebugLine::~DebugLine() {
	m_GameObject->Delete();
}
