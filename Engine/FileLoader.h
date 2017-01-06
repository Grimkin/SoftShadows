#pragma once
#include <unordered_map>
#include <d3d11.h>

#include "Types.h"
#include "tiny_obj_loader.h"

class Geometry;
class Texture;
struct Node;

struct ObjFileData {
	std::vector<tinyobj::shape_t> Shapes;
	std::vector<tinyobj::material_t> Materials;
};

struct LoadedObjData {
	std::vector<Geometry*> Geometries;
	std::vector<Texture*> Textures;
};

class FileLoader {
public:
	static FileLoader& Init( ID3D11Device* device, ID3D11DeviceContext* context );

	ObjFileData GetObjFileData( const std::wstring& fileName, const std::wstring& path );

	bool LoadObjFile( const std::wstring& fileName, const std::wstring& path, std::vector<Geometry*>* geometries = nullptr, std::vector<Texture*>* textures = nullptr );
	bool LoadTexFile( const std::wstring& fileName, TextureResource*& texture, ShaderResourceView*& srv );
	bool LoadMeshFile( const std::wstring& fileName, std::vector<Geometry*>* geometries );

	bool StoreTreeData( uint32_t resolution, float3 position, float3 size, const Node* nodes, uint32_t numNodes, const uint32_t* pointers, uint32_t numPointers, const std::wstring& fileName );
	bool LoadTreeData( const std::wstring& fileName, std::vector<Node>& nodes, std::vector<uint32_t>& pointer, uint32_t& resolution, float3& position, float3& size );
	bool LoadTreeData( const std::wstring& fileName, Node* nodes, uint32_t& numNodes, uint32_t* pointer, uint32_t& numPointers, uint32_t& resolution, float3& position, float3& size );
	bool StoreVoxelData( uint32_t resolution, float3 position, float3 size, const std::vector<std::vector<Node>>& voxelParts, const std::wstring& fileName );
	bool LoadVoxelData( const std::wstring& fileName, std::vector<std::vector<Node>>& voxelParts, uint32_t& resolution, float3& position, float3& size );
private:
	FileLoader( ID3D11Device* device, ID3D11DeviceContext* context );
	virtual ~FileLoader();

	std::unordered_map<std::wstring, LoadedObjData> m_LoadedData;

	ID3D11Device* m_Device;
	ID3D11DeviceContext* m_Context;
};

