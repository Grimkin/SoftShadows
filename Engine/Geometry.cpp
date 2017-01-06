#include "Geometry.h"

#include <cassert>

#include "D3DRenderBackend.h"
#include "Logger.h"
#include "Game.h"
#include "Makros.h"
#include "Texture.h"
#include "FileLoader.h"
#include "Math.h"

GeometryManager* Geometry::s_GeometryManager = nullptr;

Geometry::~Geometry() {
	SRelease( m_PositionsSRV );
	SRelease( m_IndexSRV );
}

Geometry * Geometry::Create( const std::wstring & fileName ) {
	return s_GeometryManager->LoadFromFile( fileName );
}

Geometry * Geometry::Create( const std::wstring & name, const std::vector<float3> positions, const std::vector<float3>& normals, const std::vector<float2>& texCoords, const std::vector<uint32_t>& indices, PrimitiveTopology topology ) {
	return new Geometry( name, positions.data(), normals.data(), texCoords.data(), static_cast<uint32_t>( positions.size() ), indices.data(), static_cast<uint32_t>( indices.size() ), topology );
}

Geometry * Geometry::Create( const std::wstring & name, const float3 * positions, const float3 * normals, const float2 * texCoords, uint32_t numVertices, const uint32_t * indices, uint32_t numIndices, PrimitiveTopology topology ) {
	return new Geometry( name, positions, normals, texCoords, numVertices, indices, numIndices, topology );
}

Geometry * Geometry::Get( const std::wstring & name ) {
	return s_GeometryManager->GetGeometry( name );
}

Geometry * Geometry::Get( uint32_t id ) {
	return s_GeometryManager->GetGeometry( id );
}

const Buffer* Geometry::GetVertexBuffer() const {
	return m_VertexBuffer;
}

void Geometry::PrepareForRender() const {
	RenderBackend* renderBackend = &Game::GetRenderBackend();

	if( m_DrawIndexed ) {
		renderBackend->SetVertexBuffer( 0, { m_PositionsBuffer, m_NormalsBuffer, m_TexCoordsBuffer }, { sizeof( float3 ), sizeof( float3 ), sizeof( float2 ) }, { 0, 0, 0 } );
		renderBackend->SetIndexBuffer( m_IndexBuffer, Format::R32_UInt, 0 );
	}
	else
		renderBackend->SetVertexBuffer( 0, { m_VertexBuffer }, { m_VertexStride }, { 0 } );
	renderBackend->SetTopology( m_Topology );

}

uint32_t Geometry::GetTriangleCount() const {
	if( m_DrawIndexed )
		return m_IndexCount / 3;
	return m_VertexCount / 3;
}

Geometry& Geometry::GetTriangleGeometry() {
	Geometry* triangleGeometry = s_GeometryManager->GetGeometry( L"Triangle" );
	if( triangleGeometry )
		return *triangleGeometry;

	std::vector<float3> positions = {
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f }
	};

	std::vector<float3> normals = {
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f }
	};

	std::vector<float2> texCoords = {
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 0.0f, 1.0f }
	};

	std::vector<uint32_t> indices = {
		0, 1, 2
	};

	triangleGeometry = new Geometry( L"Triangle", positions.data(), normals.data(), texCoords.data(), static_cast<uint32_t>( positions.size() ), indices.data(), static_cast<uint32_t>( indices.size() ), PrimitiveTopology::Trianglelist );
	return *triangleGeometry;
}

Geometry & Geometry::GetPlaneGeometry() {
	Geometry* planeGeometry = s_GeometryManager->GetGeometry( L"Plane" );
	if( planeGeometry )
		return *planeGeometry;

	std::vector<float3> positions = {
		{ -.5f, -.5f, 0.0f },
		{ .5f, -.5f, 0.0f },
		{ -.5f, .5f, 0.0f },
		{ .5f, .5f, 0.0f }
	};

	std::vector<float3> normals = {
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f }
	};

	std::vector<float2> texCoords = {
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 0.0f, 1.0f },
		{ 1.0f, 1.0f }
	};

	std::vector<uint32_t> indices = {
		0, 1, 2,
		1, 3, 2
	};

	planeGeometry = new Geometry( L"Plane", positions.data(), normals.data(), texCoords.data(), static_cast<uint32_t>( positions.size() ), indices.data(), static_cast<uint32_t>( indices.size() ), PrimitiveTopology::Trianglelist );
	return *planeGeometry;
}

Geometry & Geometry::GetCubeGeometry() {
	Geometry* cubeGeometry = s_GeometryManager->GetGeometry( L"Cube" );
	if( cubeGeometry )
		return *cubeGeometry;

	std::vector<float3> positions = {
		{ -.5f, 0.5f, -.5f },
		{ 0.5f, 0.5f, -.5f },
		{ -.5f, 0.5f, 0.5f },
		{ 0.5f, 0.5f, 0.5f },
		// Back Face
		{ -.5f, -.5f, -.5f },
		{ 0.5f, -.5f, -.5f },
		{ -.5f, -.5f, 0.5f },
		{ 0.5f, -.5f, 0.5f },
		// Right Face
		{ 0.5f, -.5f, -.5f },
		{ 0.5f, 0.5f, -.5f },
		{ 0.5f, -.5f, 0.5f },
		{ 0.5f, 0.5f, 0.5f },
		// Left Face
		{ -.5f, -.5f, -.5f },
		{ -.5f, 0.5f, -.5f },
		{ -.5f, -.5f, 0.5f },
		{ -.5f, 0.5f, 0.5f },
		// Top Face
		{ -.5f, -.5f, 0.5f },
		{ -.5f, 0.5f, 0.5f },
		{ 0.5f, -.5f, 0.5f },
		{ 0.5f, 0.5f, 0.5f },
		// Bottom Face
		{ -.5f, -.5f, -.5f },
		{ -.5f, 0.5f, -.5f },
		{ 0.5f, -.5f, -.5f },
		{ 0.5f, 0.5f, -.5f }
	};

	std::vector<float3> normals = {
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },

		{ 0.0f, -1.0f, 0.0f },
		{ 0.0f, -1.0f, 0.0f },
		{ 0.0f, -1.0f, 0.0f },
		{ 0.0f, -1.0f, 0.0f },

		{ 1.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },

		{ -1.0f, 0.0f, 0.0f },
		{ -1.0f, 0.0f, 0.0f },
		{ -1.0f, 0.0f, 0.0f },
		{ -1.0f, 0.0f, 0.0f },

		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f },

		{ 0.0f, 0.0f, -1.0f },
		{ 0.0f, 0.0f, -1.0f },
		{ 0.0f, 0.0f, -1.0f },
		{ 0.0f, 0.0f, -1.0f },
	};

	std::vector<float2> texCoords = {
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 0.0f, 1.0f },
		{ 1.0f, 1.0f },

		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 0.0f, 1.0f },
		{ 1.0f, 1.0f },

		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 0.0f, 1.0f },
		{ 1.0f, 1.0f },

		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 0.0f, 1.0f },
		{ 1.0f, 1.0f },

		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 0.0f, 1.0f },
		{ 1.0f, 1.0f },

		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 0.0f, 1.0f },
		{ 1.0f, 1.0f }
	};

	std::vector<uint32_t> indices = {
		// Front Face
		0, 2, 1,
		1, 2, 3,
		// Back Face
		4, 5, 6,
		5, 7, 6,
		// Right Face
		8, 9, 10,
		9, 11, 10,
		// Left Face
		12, 14, 13,
		13, 14, 15,
		// Top Face
		16, 18, 17,
		17, 18, 19,
		// Bottom Face
		20, 21, 22,
		21, 23, 22
	};

	cubeGeometry = new Geometry( L"Cube", positions.data(), normals.data(), texCoords.data(), static_cast<uint32_t>( positions.size() ), indices.data(), static_cast<uint32_t>( indices.size() ), PrimitiveTopology::Trianglelist );
	return *cubeGeometry;
}

void Geometry::Draw() const {
	if( m_DrawIndexed )
		Game::GetRenderBackend().DrawIndexed( m_IndexCount, 0, 0 );
	else
		Game::GetRenderBackend().Draw( m_VertexCount, 0 );
}

Geometry::Geometry( const std::wstring & name, Buffer* vertexBuffer, uint32_t vertices, uint32_t vertexStride, PrimitiveTopology topology )
	: m_Name( name )
	, m_VertexBuffer( vertexBuffer )
	, m_VertexCount( vertices )
	, m_VertexStride( vertexStride )
	, m_Topology( topology ) {
	s_GeometryManager->RegisterGeometry( *this, m_ID );
}

GeometryManager& GeometryManager::Init() {
	static GeometryManager geometryManager;
	return geometryManager;
}

Geometry* GeometryManager::LoadFromFile( const std::wstring & fileName ) {
	if( m_NameToID.count( fileName ) > 0 )
		return m_Geometries[m_NameToID[fileName]];

	std::wstring loadPath = fileName;
	size_t pos = loadPath.rfind( '.' );
	std::wstring type;
	type = loadPath.substr( pos + 1 );

	std::vector<Geometry*> geometries;
	if( type == L"obj" )
		Game::GetFileLoader().LoadObjFile( fileName, m_GeometryPath, &geometries );
	else if( type == L"mesh" )
		Game::GetFileLoader().LoadMeshFile( m_GeometryPath + fileName, &geometries );

	if( !geometries.empty() ) {
		m_NameToID[fileName] = geometries[0]->GetID();
		return geometries[0];
	}

	return nullptr;
}

bool GeometryManager::RegisterGeometry( Geometry & geometry, uint32_t& id ) {
	bool added = true;
	id = static_cast<uint32_t>( m_Geometries.size() );

	if( m_NameToID.count( geometry.GetName() ) || geometry.GetName() == L"" )
		added = false;
	else
		m_NameToID.emplace( geometry.GetName(), id );

	m_Geometries.push_back( &geometry );
	return added;
}

Geometry * GeometryManager::GetGeometry( const std::wstring & name ) {
	if( m_NameToID.count( name ) == 0 ) {
		return nullptr;
	}

	return m_Geometries[m_NameToID[name]];
}

Geometry * GeometryManager::GetGeometry( uint32_t id ) {
	if( m_Geometries.size() >= id )
		return nullptr;

	return m_Geometries[id];
}

GeometryManager::GeometryManager() {
	Game::SetGeometryManager( *this );
	Geometry::s_GeometryManager = this;
}

GeometryManager::~GeometryManager() {
	SDeleteVec<Geometry>( m_Geometries, []( Geometry* geometry ) { delete geometry; } );
}

DynamicGeometry * DynamicGeometry::Create( const std::wstring& name, uint32_t maxElements, uint32_t elementSize, PrimitiveTopology topology ) {
	BufferDesc bDesc;
	bDesc.BindFlags = BindFlag::VertexBuffer;
	bDesc.Usage = Usage::Dynamic;
	bDesc.ByteWidth = maxElements * elementSize;
	bDesc.CPUAccessFlags = CPUAccessFlag::Write;
	Buffer* vertexBuffer = Game::GetRenderBackend().CreateBuffer( nullptr, bDesc );

	DynamicGeometry* newGeometry = new DynamicGeometry( name, vertexBuffer, 0, elementSize, topology );
	return newGeometry;
}

void DynamicGeometry::UpdateBuffer( void * data, uint32_t numElements ) {
	m_VertexCount = numElements;
	if( numElements > 0 )
		Game::GetRenderBackend().UpdateBuffer( m_VertexBuffer, data, numElements*m_VertexStride, 0, MapType::WriteDiscard );
}

DynamicGeometry::DynamicGeometry( const std::wstring& name, Buffer* vertexBuffer, uint32_t vertices, uint32_t vertexStride, PrimitiveTopology topology )
	: Geometry( name, vertexBuffer, vertices, vertexStride, topology ) {
}


Geometry::Geometry( const std::wstring& name, const float3* positions, const float3* normals, const float2* texCoords, uint32_t numVertices,
					const uint32_t* indices, uint32_t numIndices, PrimitiveTopology topology )
	: m_Name( name )
	, m_VertexCount( numVertices )
	, m_IndexCount( numIndices )
	, m_VertexStride( 0 )
	, m_DrawIndexed( true )
	, m_Topology( topology ) {

	m_AlignedPositions.reserve( m_VertexCount );
	for( uint32_t i = 0; i < m_VertexCount; ++i ) {
		m_AlignedPositions.push_back( make_float3a( positions[i] ) );
	}
	m_TriangleIndices.reserve( m_IndexCount / 3 );
	for( uint32_t i = 0; i < m_IndexCount / 3; ++i ) {
		uint3 triangleIndex;
		triangleIndex.x = indices[3 * i];
		triangleIndex.y = indices[3 * i + 1];
		triangleIndex.z = indices[3 * i + 2];
		m_TriangleIndices.push_back( triangleIndex );
	}

	RenderBackend* renderBackend = &Game::GetRenderBackend();

	BufferDesc desc;
	desc.BindFlags = BindFlag::VertexBuffer | BindFlag::ShaderResource;
	desc.ByteWidth = sizeof( float3 ) *	numVertices;
	desc.Usage = Usage::Immutable;
	desc.CPUAccessFlags = CPUAccessFlag::None;
	desc.MiscFlags = ResourceMiscFlag::BufferAllowRawViews;
	m_PositionsBuffer = renderBackend->CreateBuffer( positions, desc );

	SRVDesc srvDesc;
	srvDesc.Format = Format::R32_Typeless;
	srvDesc.ViewDimension = SRVDimension::BufferEx;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.BufferEx.Flags = 1;
	srvDesc.BufferEx.NumElements = desc.ByteWidth / 4;

	m_PositionsSRV = renderBackend->CreateSRV( m_PositionsBuffer, &srvDesc );

	if( normals ) {
		desc.ByteWidth = sizeof( float3 ) * numVertices;
		m_NormalsBuffer = renderBackend->CreateBuffer( normals, desc );
	}
	if( texCoords ) {
		desc.ByteWidth = sizeof( float2 ) * numVertices;
		m_TexCoordsBuffer = renderBackend->CreateBuffer( texCoords, desc );
	}

	desc.BindFlags = BindFlag::IndexBuffer | BindFlag::ShaderResource;
	desc.ByteWidth = sizeof( uint32_t ) * numIndices;
	m_IndexBuffer = renderBackend->CreateBuffer( indices, desc );

	srvDesc.Format = Format::R32_UInt;
	srvDesc.ViewDimension = SRVDimension::Buffer;
	srvDesc.Buffer.NumElements = numIndices;
	m_IndexSRV = renderBackend->CreateSRV( m_IndexBuffer, &srvDesc );


	if( !s_GeometryManager->RegisterGeometry( *this, m_ID ) )
		if( name.size() > 0 )
			Game::GetLogger().Log( L"Geometry", L"Name \"" + name + L"\" already taken. New Geometry not accessible through name" );
}