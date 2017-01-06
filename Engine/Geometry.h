#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "D3DWrapper.h"
#include "RenderBackend.h"
#include "Logger.h"
#include "Types.h"

class D3DRenderBackend;

struct Vertex {
	float3 Position;
	float3 Normal;
	float2 TextureCoord;
};

struct ShapeData {
	std::wstring ShapeName;
	std::vector<Vertex> Vertices;
	std::vector<uint32_t> Indices;
	std::wstring AmbientTexture;
	std::wstring DiffuseTexture;
	std::wstring SpecularTexture;
	std::wstring NormalTexture;
};

class Geometry {
	friend class GeometryManager;
public:
	static Geometry* Create( const std::wstring& fileName );
	template<typename T>
	static Geometry* Create( const std::wstring& name, const std::vector<T>& vertices, PrimitiveTopology topology, bool isDynamic = false );
	template<typename T>
	static Geometry* Create( const std::wstring& name, const std::vector<T>& vertices, const std::vector<uint32_t> indices, PrimitiveTopology topology, bool isDynamic = false );
	static Geometry* Create( const std::wstring& name, const std::vector<float3> positions, const std::vector<float3>& normals, 
							 const std::vector<float2>& texCoords, const std::vector<uint32_t>& indices, PrimitiveTopology topology );
	static Geometry* Create( const std::wstring& name, const float3* positions, const float3* normals, const float2* texCoords, uint32_t numVertices,
							 const uint32_t* indices, uint32_t numIndices, PrimitiveTopology topology );

	static Geometry* Get( const std::wstring& name );
	static Geometry* Get( uint32_t id );

	virtual const std::wstring& GetName() const {
		return m_Name;
	}

	virtual const Buffer* GetVertexBuffer() const;
	virtual void PrepareForRender() const;
	virtual uint32_t GetTriangleCount() const;

	uint32_t GetID() const {
		return m_ID;
	}
	ShaderResourceView* GetPositionSRV() const {
		return m_PositionsSRV;
	}
	ShaderResourceView* GetIndexSRV() const {
		return m_IndexSRV;
	}
	const std::vector<float3a>& GetAlignedPositions() {
		return m_AlignedPositions;
	}
	const std::vector<uint3>& GetTriangleIndices(){
		return m_TriangleIndices;
	}

	static Geometry& GetTriangleGeometry();
	static Geometry& GetPlaneGeometry();
	static Geometry& GetCubeGeometry();

	virtual void Draw() const;
protected:
	Geometry( const std::wstring& name, Buffer* vertexBuffer, uint32_t vertices, uint32_t vertexStride, PrimitiveTopology topology );
	template<typename T>
	Geometry( const std::wstring& name, const std::vector<T>& vertices, PrimitiveTopology topology, bool isDynamic = false );
	template<typename T>
	Geometry( const std::wstring& name, const std::vector<T>& vertices, const std::vector<uint32_t>& indices, PrimitiveTopology topology, bool isDynamic = false );
	Geometry( const std::wstring& name, const float3* positions, const float3* normals, const float2* texCoords, uint32_t numVertices,
			  const uint32_t* indices, uint32_t numIndices, PrimitiveTopology topology );
	virtual ~Geometry();

	std::wstring m_Name;
	Buffer* m_VertexBuffer;
	Buffer* m_IndexBuffer;
	Buffer* m_PositionsBuffer;
	Buffer* m_NormalsBuffer;
	Buffer* m_TexCoordsBuffer;
	ShaderResourceView* m_PositionsSRV;
	ShaderResourceView* m_IndexSRV;
	uint32_t m_VertexCount;
	uint32_t m_IndexCount;
	uint32_t m_VertexStride;
	uint32_t m_ID;

	std::vector<float3a> m_AlignedPositions;
	std::vector<uint3> m_TriangleIndices;

	bool m_SeperateBuffers = false;

	PrimitiveTopology m_Topology;

	static GeometryManager* s_GeometryManager;

	bool m_DrawIndexed;
};

class DynamicGeometry : public Geometry {
	friend class GeometryManager;
public:
	static DynamicGeometry* Create( const std::wstring& name, uint32_t maxElements, uint32_t elementSize, PrimitiveTopology topology );

	void UpdateBuffer( void* data, uint32_t numElements );
private:
	DynamicGeometry( const std::wstring& name, Buffer* vertexBuffer, uint32_t vertices, uint32_t vertexStride, PrimitiveTopology topology );
};

class GeometryManager {

public:
	static GeometryManager& Init();

	Geometry* LoadFromFile( const std::wstring& filename );
	bool RegisterGeometry( Geometry& geometry, uint32_t& id );
	Geometry* GetGeometry( const std::wstring& name );
	Geometry* GetGeometry( uint32_t id );
private:
	GeometryManager();
	virtual ~GeometryManager();

	std::vector<Geometry*> m_Geometries;
	std::unordered_map<std::wstring, uint32_t> m_NameToID;

	const std::wstring m_GeometryPath = L"Assets\\Geometries\\";
};

template<typename T>
inline Geometry * Geometry::Create( const std::wstring& name, const std::vector<T>& vertices, PrimitiveTopology topology, bool isDynamic ) {
	return new Geometry( name, vertices, topology, isDynamic );
}

template<typename T>
inline Geometry * Geometry::Create( const std::wstring& name, const std::vector<T>& vertices, const std::vector<uint32_t> indices, PrimitiveTopology topology, bool isDynamic ) {
	return new Geometry( name, vertices, indices, topology, isDynamic );
}

template<typename T>
inline Geometry::Geometry( const std::wstring& name, const std::vector<T>& vertices, PrimitiveTopology topology, bool isDynamic )
	: m_Name( name )
	, m_VertexCount( vertices.size() )
	, m_VertexStride( sizeof( T ) )
	, m_DrawIndexed( false )
	, m_Topology( topology ) {

	BufferDesc desc;
	desc.BindFlags = BindFlag::VertexBuffer;
	desc.ByteWidth = sizeof( T ) * vertices.size();
	if( isDynamic ) {
		desc.Usage = Usage::Dynamic;
		desc.CPUAccessFlags = CPUAccessFlag::Write;
	}
	else {
		desc.Usage = Usage::Default;
		desc.CPUAccessFlags = CPUAccessFlag::None;
	}

	m_VertexBuffer = Game::GetRenderBackend().CreateBuffer( vertices.data(), desc );

	if( !s_GeometryManager->RegisterGeometry( *this, m_ID ) )
		if( name.size() > 0 )
			Game::GetLogger().Log( L"Geometry", L"Name \"" + name + L"\" already taken. New Geometry not accessible through name" );
		else
			Game::GetLogger().Log( L"Geometry", L"No name specified. Geometry only accessible via ID or pointer" );
}

template<typename T>
inline Geometry::Geometry( const std::wstring& name, const std::vector<T>& vertices, const std::vector<uint32_t>& indices, PrimitiveTopology topology, bool isDynamic )
	: m_Name( name )
	, m_VertexCount( static_cast<uint32_t>( vertices.size() ) )
	, m_IndexCount( static_cast<uint32_t>( indices.size() ) )
	, m_VertexStride( sizeof( T ) )
	, m_DrawIndexed( true )
	, m_Topology( topology ) {

	RenderBackend* renderBackend = &Game::GetRenderBackend();

	BufferDesc desc;
	desc.BindFlags = BindFlag::VertexBuffer;
	desc.ByteWidth = sizeof( T ) * m_VertexCount;
	if( isDynamic ) {
		desc.Usage = Usage::Dynamic;
		desc.CPUAccessFlags = CPUAccessFlag::Write;
	}
	else {
		desc.Usage = Usage::Default;
		desc.CPUAccessFlags = CPUAccessFlag::None;
	}
	m_VertexBuffer = renderBackend->CreateBuffer( vertices.data(), desc );
	
	desc.BindFlags = BindFlag::IndexBuffer;
	desc.ByteWidth = sizeof( float3 ) * m_IndexCount;
	m_IndexBuffer = renderBackend->CreateBuffer( indices.data(), desc );

	if( !s_GeometryManager->RegisterGeometry( *this, m_ID ) )
		if( name.size() > 0 )
			Game::GetLogger().Log( L"Geometry", L"Name \"" + name + L"\" already taken. New Geometry not accessible through name" );
		else
			Game::GetLogger().Log( L"Geometry", L"No name specified. Geometry only accessible via ID or pointer" );
}
