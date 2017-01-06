#define TINYOBJLOADER_IMPLEMENTATION
#include "FileLoader.h"

#include <fstream>

#include "Makros.h"
#include "Game.h"
#include "Logger.h"
#include "Geometry.h"
#include "Texture.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"
#include "Mesh_generated.h"
#include "Voxel_generated.h"
#include "Voxelization_generated.h"
#include "Voxelizer.h"

FileLoader & FileLoader::Init( ID3D11Device * device, ID3D11DeviceContext * context ) {
	static FileLoader fileLoader( device, context );
	return fileLoader;
}

ObjFileData FileLoader::GetObjFileData( const std::wstring & fileName, const std::wstring& path ) {
	std::string inputfile = ws2s( path + fileName );
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;

	ObjFileData data;
	bool ret = tinyobj::LoadObj( shapes, materials, err, inputfile.c_str(), ws2s( path ).c_str() );

	if( !err.empty() ) {
		Game::GetLogger().Log( L"Geometry", L"TinyObjLoader warning for file \"" + fileName + L"\": " + s2ws( err ) );
	}
	if( !ret ) {
		Game::GetLogger().Log( L"Geometry", L"Loading of file \"" + fileName + L" failed" );
		return data;
	}

	data.Shapes = shapes;
	data.Materials = materials;
	return data;
}

bool FileLoader::LoadObjFile( const std::wstring & fileName, const std::wstring& path, std::vector<Geometry*>* geometries, std::vector<Texture*>* textures ) {
	if( m_LoadedData.count( fileName ) > 0 )
		return true;

	ObjFileData data = GetObjFileData( fileName, path );
	if( data.Shapes.empty() )
		return false;

	m_LoadedData[fileName] = LoadedObjData();
	LoadedObjData& loadedData = m_LoadedData[fileName];

	// Create the Geometry
	for( tinyobj::shape_t& shape : data.Shapes ) {
		std::vector<float3>* positions = reinterpret_cast<std::vector<float3>*>( &shape.mesh.positions );
		std::vector<float3>* normals = reinterpret_cast<std::vector<float3>*>( &shape.mesh.normals );
		std::vector<float2>* texCoords = reinterpret_cast<std::vector<float2>*>( &shape.mesh.texcoords );

			

		Geometry* newGeometry = Geometry::Create( s2ws( shape.name ), *positions, *normals, *texCoords, shape.mesh.indices, PrimitiveTopology::Trianglelist );
		if( newGeometry )
			loadedData.Geometries.push_back( newGeometry );
	}

	// Create the textures
	for( tinyobj::material_t& material : data.Materials ) {
		if( !material.alpha_texname.empty() ) {
			Texture* newTex( Texture::CreateFromFile( s2ws( material.alpha_texname ) ) );
			if( newTex )
				loadedData.Textures.push_back( newTex );
		}
		if( !material.ambient_texname.empty() ) {
			Texture* newTex( Texture::CreateFromFile( s2ws( material.ambient_texname ) ) );
			if( newTex )
				loadedData.Textures.push_back( newTex );
		}
		if( !material.bump_texname.empty() ) {
			Texture* newTex( Texture::CreateFromFile( s2ws( material.bump_texname ) ) );
			if( newTex )
				loadedData.Textures.push_back( newTex );
		}
		if( !material.diffuse_texname.empty() ) {
			Texture* newTex( Texture::CreateFromFile( s2ws( material.diffuse_texname ) ) );
			if( newTex )
				loadedData.Textures.push_back( newTex );
		}
		if( !material.displacement_texname.empty() ) {
			Texture* newTex( Texture::CreateFromFile( s2ws( material.displacement_texname ) ) );
			if( newTex )
				loadedData.Textures.push_back( newTex );
		}
		if( !material.specular_highlight_texname.empty() ) {
			Texture* newTex( Texture::CreateFromFile( s2ws( material.specular_highlight_texname ) ) );
			if( newTex )
				loadedData.Textures.push_back( newTex );
		}
		if( !material.specular_texname.empty() ) {
			Texture* newTex( Texture::CreateFromFile( s2ws( material.specular_texname ) ) );
			if( newTex )
				loadedData.Textures.push_back( newTex );
		}
	}

	if( geometries )
		*geometries = loadedData.Geometries;
	if( textures )
		*textures = loadedData.Textures;

	return true;
}

bool FileLoader::LoadTexFile( const std::wstring & fileName, TextureResource*& texture, ShaderResourceView*& srv ) {
	std::wstring loadPath = fileName;
	size_t pos = loadPath.rfind( '.' );
	std::wstring type;
	if( pos == std::wstring::npos ) {
		Game::GetLogger().Log( L"Texture", loadPath + L" has no file ending. DDS assumed." );
		loadPath += L".dds";
		type = L"dds";
	}
	else
		type = loadPath.substr( pos + 1 );

	HRESULT hr;
	if( type == L"dds" ) {
		hr = DirectX::CreateDDSTextureFromFile( m_Device, m_Context, ( loadPath ).c_str(), &texture, &srv );
	}
	else {
		hr = DirectX::CreateWICTextureFromFile( m_Device, m_Context, ( loadPath ).c_str(), &texture, &srv );
	}

	if( FAILED( hr ) ) {
		Game::GetLogger().FatalError( L"Could not open file " + loadPath );
		return false;
	}
	return true;
}

bool FileLoader::LoadMeshFile( const std::wstring & fileName, std::vector<Geometry*>* geometries ) {
	Game::GetLogger().Log( L"Loader", L"Loading mesh " + fileName + L"." );
	using namespace Loader::Mesh;
	std::ifstream file( fileName, std::ios::in | std::ios::binary );
	if( !file.is_open() ) {
		Game::GetLogger().Log( L"Loader", L"Loading failed. Couldn't open file \"" + fileName + L"\"." );
		return false;
	}
	std::string data = std::string( std::istreambuf_iterator<char>( file ), std::istreambuf_iterator<char>() );

	flatbuffers::FlatBufferBuilder builder;
	builder.PushFlatBuffer( reinterpret_cast<const uint8_t*>( data.c_str() ), data.size() );

	auto meshes = GetMeshes( builder.GetBufferPointer() )->Objects();
	for( uint32_t i = 0; i < static_cast<uint32_t>( meshes->size() ); i++ ) {
		auto mesh = meshes->Get( i );
		const float3* positions = reinterpret_cast<const float3*>( mesh->Positions()->Get( 0 ) );
		const float3* normals = reinterpret_cast<const float3*>( mesh->Normals()->Get( 0 ) );
		const uint32_t* indices = mesh->Indices()->data();
		uint32_t numVertices = mesh->Positions()->size();
		uint32_t numIndices = mesh->Indices()->size();

		if( geometries )
			geometries->push_back( Geometry::Create( L"", positions, normals, nullptr, numVertices, indices, numIndices, PrimitiveTopology::Trianglelist ) );
		Game::GetLogger().Log( L"Loader", L"Loaded mesh with " + std::to_wstring( numVertices ) + L" vertices and " + std::to_wstring( numIndices / 3 ) + L" triangles." );
	}

	return true;
}

bool FileLoader::StoreTreeData( uint32_t resolution, float3 position, float3 size, const Node * nodes, uint32_t numNodes, const uint32_t * pointers, uint32_t numPointers, const std::wstring & fileName ) {
	using namespace Loader::Voxel;
	flatbuffers::FlatBufferBuilder builder;

	const Loader::Voxel::Node* voxNodes = reinterpret_cast<const Loader::Voxel::Node*>( nodes );

	auto nodeVec = builder.CreateVectorOfStructs( voxNodes, numNodes );
	auto ptrVec = builder.CreateVector( pointers, numPointers );

	Loader::Voxel::float3* fbPos = reinterpret_cast<Loader::Voxel::float3*>( &position );
	Loader::Voxel::float3* fbSize = reinterpret_cast<Loader::Voxel::float3*>( &size );
	
	auto obj = CreateVoxelObject( builder, resolution, fbPos, fbSize, nodeVec, ptrVec );

	FinishVoxelObjectBuffer( builder, obj );

	std::ofstream file( fileName, std::ios::out | std::ios::trunc | std::ios::binary );
	if( !file.is_open() ) {
		Game::GetLogger().Log( L"Loader", L"Creating file \"" + fileName + L"\" failed for storing voxel data." );
		return false;
	}

	file.write( reinterpret_cast<char*>( builder.GetBufferPointer() ), builder.GetSize() );
	file.close();

	if( file.bad() ) {
		Game::GetLogger().Log( L"Loader", L"Storing voxel data into file \"" + fileName + L"\" failed." );
		return false;
	}
	return true;
}

bool FileLoader::LoadTreeData( const std::wstring & fileName, std::vector<Node>& nodes, std::vector<uint32_t>& pointer, uint32_t& resolution, float3& position, float3& size ) {
	using namespace Loader::Voxel;
	std::ifstream file( fileName, std::ios::in | std::ios::binary );
	if( !file.is_open() ) {
		Game::GetLogger().Log( L"Loader", L"Opening voxel file \"" + fileName + L"\" failed. Revoxalizing." );
		return false;
	}

	std::string data = std::string( std::istreambuf_iterator<char>( file ), std::istreambuf_iterator<char>() );

	flatbuffers::FlatBufferBuilder builder;
	builder.PushFlatBuffer( reinterpret_cast<const uint8_t*>( data.c_str() ), data.size() );

	auto obj = GetVoxelObject( builder.GetBufferPointer() );
	
	const ::Node* nodePtr = reinterpret_cast<const ::Node*>( obj->Nodes()->Get( 0 ) );
	uint32_t numNodes = obj->Nodes()->size();

	nodes.assign( nodePtr, nodePtr + numNodes );
	pointer.assign( obj->Pointers()->begin(), obj->Pointers()->end() );

	resolution = obj->Resolution();
	position = *reinterpret_cast<const ::float3*>( obj->Position() );
	size = *reinterpret_cast<const ::float3*>( obj->Size() );

	return true;
}

bool FileLoader::LoadTreeData( const std::wstring & fileName, Node * nodes, uint32_t& numNodes, uint32_t * pointer, uint32_t& numPointers, uint32_t& resolution, float3& position, float3& size ) {
	using namespace Loader::Voxel;
	std::ifstream file( fileName, std::ios::in | std::ios::binary );
	if( !file.is_open() ) {
		Game::GetLogger().Log( L"Loader", L"Opening voxel file \"" + fileName + L"\" failed. Revoxalizing." );
		return false;
	}

	std::string data = std::string( std::istreambuf_iterator<char>( file ), std::istreambuf_iterator<char>() );

	flatbuffers::FlatBufferBuilder builder;
	builder.PushFlatBuffer( reinterpret_cast<const uint8_t*>( data.c_str() ), data.size() );

	auto obj = GetVoxelObject( builder.GetBufferPointer() );

	numNodes = obj->Nodes()->size();
	memcpy( nodes, obj->Nodes()->Get( 0 ), numNodes * sizeof( ::Node ) );

	numPointers = obj->Pointers()->size();
	memcpy( pointer, obj->Pointers()->data(), numPointers * sizeof( uint32_t ) );

	resolution = obj->Resolution();
	position = *reinterpret_cast<const ::float3*>( obj->Position() );
	size = *reinterpret_cast<const ::float3*>( obj->Size() );

	return true;
}

bool FileLoader::StoreVoxelData( uint32_t resolution, float3 position, float3 size, const std::vector<std::vector<Node>>& voxelParts, const std::wstring & fileName ) {
	using namespace Loader::Voxelization;
	flatbuffers::FlatBufferBuilder builder;

	std::vector<flatbuffers::Offset<VoxelPart>> fbVoxelParts;

	for( auto& voxelPart : voxelParts ) {
		const std::vector<Loader::Voxelization::Brick>* fbPartVec = reinterpret_cast<const std::vector<Loader::Voxelization::Brick>*>( &voxelPart );
		auto fbBricks = builder.CreateVectorOfStructs( *fbPartVec );
		auto fbPart = CreateVoxelPart( builder, fbBricks );
		fbVoxelParts.push_back( fbPart );
	}

	Loader::Voxelization::float3* fbPos = reinterpret_cast<Loader::Voxelization::float3*>( &position );
	Loader::Voxelization::float3* fbSize = reinterpret_cast<Loader::Voxelization::float3*>( &size );

	auto fbVoxelPartVec = builder.CreateVector( fbVoxelParts );

	auto obj = CreateVoxelization( builder, resolution, fbPos, fbSize, fbVoxelPartVec );

	FinishVoxelizationBuffer( builder, obj );

	std::ofstream file( fileName, std::ios::out | std::ios::trunc | std::ios::binary );
	if( !file.is_open() ) {
		Game::GetLogger().Log( L"Loader", L"Creating file \"" + fileName + L"\" failed for storing voxel data." );
		return false;
	}

	file.write( reinterpret_cast<char*>( builder.GetBufferPointer() ), builder.GetSize() );
	file.close();

	if( file.bad() ) {
		Game::GetLogger().Log( L"Loader", L"Storing voxel data into file \"" + fileName + L"\" failed." );
		return false;
	}

	return true;
}

bool FileLoader::LoadVoxelData( const std::wstring & fileName, std::vector<std::vector<Node>>& voxelParts, uint32_t & resolution, float3 & position, float3 & size ) {
	using namespace Loader::Voxelization;
	std::ifstream file( fileName, std::ios::in | std::ios::binary );
	if( !file.is_open() )
		return false;

	std::string data = std::string( std::istreambuf_iterator<char>( file ), std::istreambuf_iterator<char>() );

	flatbuffers::FlatBufferBuilder builder;
	builder.PushFlatBuffer( reinterpret_cast<const uint8_t*>( data.c_str() ), data.size() );

	auto fbVoxelization = GetVoxelization( builder.GetBufferPointer() );

	resolution = fbVoxelization->Resolution();
	position = *reinterpret_cast<const ::float3*>( fbVoxelization->Position() );
	size = *reinterpret_cast<const ::float3*>( fbVoxelization->Size() );

	auto fbVoxelParts = fbVoxelization->VoxelParts();
	voxelParts.resize( fbVoxelParts->size() );
	for( uint32_t i = 0; i < static_cast<uint32_t>( fbVoxelParts->size() ); i++ ) {
		auto voxelPart = fbVoxelParts->Get( i )->Bricks();
		if( voxelPart->size() > 0 ) {
			const ::Node* brickPtr = reinterpret_cast<const ::Node*>( voxelPart->Get( 0 ) );
			uint32_t numBricks = voxelPart->size();

			voxelParts[i].assign( brickPtr, brickPtr + numBricks );
		}
	}

	return true;
}

FileLoader::FileLoader( ID3D11Device * device, ID3D11DeviceContext * context )
	: m_Device( device )
	, m_Context( context ) {
	Game::SetFileLoader( *this );
}


FileLoader::~FileLoader() {
}
