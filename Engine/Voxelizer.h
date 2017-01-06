#pragma once

#include <unordered_map>

#include "Math.h"
#include "ConstantBuffer.h"

class Geometry;
class RenderPass;
class Camera;
class Texture;
class Shader;
class Slider;
class GameObject;
struct DebugData;

struct VoxelGrid {

};

__declspec( align( 16 ) )
struct VoxelizeData {
	uint3 gridSize;
	__declspec( align( 16 ) )
		uint3 numTexels;
	float3a deltaGrid;
	float3a invDeltaGrid;
	__declspec( align( 16 ) )
		uint3 numBits;
	float3a minBoxPos;
	float3a boxSize;
};

__declspec( align( 16 ) )
struct VoxelObjectData {
	Matrix worldMat;
	uint32_t numTriangles;
};

__declspec( align( 16 ) )
struct VoxelTestData {
	float3a vCamPos;
	float3a vLookDir;
	float3a vLookUp;
	float3a vLookRight;
	float2 fMaxScreenVal;
	float2 vCamSize;
};

__declspec( align( 16 ) )
struct GridData {
	uint3 GridResolution;
	uint32_t TreeSize = sizeof( GridData );
	float3 MinGridPosition;
	float MaxItValue;
	float3 GridSize;
};

__declspec( align( 16 ) )
struct FilterData {
	Matrix LastFrameMatrix;
	uint32_t Seed;
	float JitterSep;
	float JitterRad;
	uint32_t UseNormal;
	uint32_t UseReprojection;
	uint32_t UseSmooth;
	uint32_t RenderShadow;
	float RadiusMod;
	float DepthThresh;
};

struct Node {
	uint2 Data;
	uint32_t Pointer;
};

class Voxelizer {
public:
	Voxelizer( const float3& position, const float3& size, const uint32_t resolution );
	virtual ~Voxelizer();

	void VoxelizeObject( GameObject& gameObject );
	VoxelGrid* Voxelize( const std::vector<std::pair<const Geometry*, Matrix>>& elements );
	void TestRender( Camera& camera );
	void BindSRVs();

	void SetLightSizeAngle( float angle );
	void SetLightSamples( uint32_t samples );
	void SetLightDir( float horizontal, float vertical );
	float3 GetLightDir();
	float GetLightAngleSize();

	void SetSliderVisibility( bool visible );
	Texture* GetCurrentShadowTex();
private:
	bool CreateBuffers();
	bool CreateRenderPass();
	bool CreateCamera();
	void StitchTree( const std::vector<std::pair<std::vector<Node>, std::vector<uint32_t>>>& treeParts );
	void UpdateVoxelizeData( uint32_t voxelPart, const float3& numVoxelParts );
	bool LoadTree( const std::wstring& fileName );
	void StoreDebugData( const DebugData& debugData );
	
	void UpdateGridData();

	uint32_t m_ResolutionMultiplier = 1;
	uint32_t m_Width = 256;
	uint32_t m_Height = 256;
	uint32_t m_Depth = 256;

	float3 m_Size;
	float3 m_Position;

	std::unordered_map<Geometry*, VoxelGrid> m_VoxelizedMeshes;

	Texture* m_VoxelRenderTarget;
	Texture* m_GridTexture;
	Buffer* m_TreeBuffer;
	UnorderedAccessView* m_TreeUAV;
	ShaderResourceView* m_TreeSRV;
	Buffer* m_BrickBuffer;
	UnorderedAccessView* m_BrickUAV;
	ShaderResourceView* m_BrickSRV;

	Buffer* m_CountBuffer;
	UnorderedAccessView* m_CountUAV;

	Buffer* m_PointerBuffer;
	UnorderedAccessView* m_PointerUAV;
	ShaderResourceView* m_PointerSRV;

	Buffer* m_ApproxBuffer;
	ShaderResourceView* m_ApproxSRV;

	Shader* m_VoxelShader;
	Shader* m_SparselizeShader;
	Shader* m_MipMapShader;

	RenderPass* m_VoxelizePass;
	RenderPass* m_TestPass;
	Camera* m_Camera;
	ConstantBuffer<VoxelizeData> m_VoxelizeDataBuffer;
	ConstantBuffer<VoxelObjectData> m_ObjectDataBuffer;
	ConstantBuffer<VoxelTestData> m_TestDataBuffer;
	ConstantBuffer<GridData> m_GridDataBuffer;
	ConstantBuffer<FilterData> m_FilterDataBuffer;

	Texture* m_ShadowTexture1;
	Texture* m_ShadowTexture2;

	Slider* m_SampleSlider;
	Slider* m_AngleSlider;
	Slider* m_HorizontalLightSlider;
	Slider* m_VerticalLightSlider;

	std::unordered_map<const GameObject*, std::vector<Node>> m_VoxelizedObjects;

	int m_NumLightSamples = 1;
	float m_LightAngleSize = 0.f;
	float m_VerticalLightAngle = Deg2Rad( 45.f );
	float m_HorizontalLightAngle = Deg2Rad( 315.f );

	float m_MaxItValue = 20.f;

	const uint32_t m_NumTreeNodes = 1024 * 1024 * 1024 / 16;

	uint32_t m_FilterIdx = 0;

	bool m_UseShadowTexOne = true;
};

