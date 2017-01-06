#pragma once
#include "RenderBackend.h"

#include <d3d11_1.h>
#include <d3d11.h>
#include <vector>
#include <memory>
#include <array>

#include "D3DWrapper.h"

class Window;
class Shader;
class Scene;
class Voxelizer;
class Embree;

union TexDesc;

enum class TextureType;

class D3DRenderBackend :
	public RenderBackend {
	friend class Game;
public:
	static D3DRenderBackend& Init( Window& window );

	virtual void Start() override;
	virtual void Render() override;
	virtual void Exit() override;

	void OnWindowChange( bool fullscreen, uint2 size ) override;

	void CreateSwapChain( IDXGIFactory* factory, const Window& window, IDXGISwapChain* swapChain );
	void CreateDefaultRTV_DSV();
	void ResizeRTV_DSV( uint32_t width, uint32_t height );

	RasterizerState* GetRasterizeState( RasterizerDesc& desc ) override;
	DepthStencilState* GetDepthStencilState( DepthStencilDesc& desc ) override;
	BlendState* GetBlendState( BlendDesc& desc ) override;
	SamplerState* GetSamplerState( SamplerDesc& desc ) override;

	void SetRenderTarget( const std::vector<RenderTargetView*>& rtvs, DepthStencilView* dsv ) override;
	void SetDeferredRTVs() override;
	void SetViewports( const std::vector<Viewport>& viewports ) override;
	void SetRasterizerState( RasterizerState* rasterizerState ) override;
	void SetDepthStencilState( DepthStencilState* depthStencilState ) override;
	void SetBlendState( BlendState* blendState, const float* blendFactor, UINT sampleMask ) override;
	void SetTopology( PrimitiveTopology topology ) override;
	void SetVertexBuffer( UINT startIdx, std::vector<Buffer*> buffers, std::vector<UINT> strides, std::vector<UINT> offsets ) override;
	void SetIndexBuffer( Buffer* buffer, Format format, uint32_t offset ) override;
	void SetSamplerVS( uint16_t startSlot, const std::vector<SamplerState*>& samplers ) override;
	void SetSamplerPS( uint16_t startSlot, const std::vector<SamplerState*>& samplers ) override;
	void SetSamplerGS( uint16_t startSlot, const std::vector<SamplerState*>& samplers ) override;
	void SetSamplerHS( uint16_t startSlot, const std::vector<SamplerState*>& samplers ) override;
	void SetSamplerDS( uint16_t startSlot, const std::vector<SamplerState*>& samplers ) override;
	void SetSamplerCS( uint16_t startSlot, const std::vector<SamplerState*>& samplers ) override;
	void SetSRVsVS( uint16_t startSlot, const std::vector<ShaderResourceView*>& samplers ) override;
	void SetSRVsPS( uint16_t startSlot, const std::vector<ShaderResourceView*>& samplers ) override;
	void SetSRVsGS( uint16_t startSlot, const std::vector<ShaderResourceView*>& samplers ) override;
	void SetSRVsHS( uint16_t startSlot, const std::vector<ShaderResourceView*>& samplers ) override;
	void SetSRVsDS( uint16_t startSlot, const std::vector<ShaderResourceView*>& samplers ) override;
	void SetSRVsCS( uint16_t startSlot, const std::vector<ShaderResourceView*>& samplers ) override;
	void SetUAVPS( uint16_t startSlot, const std::vector<UnorderedAccessView*>& uavs, const std::vector<uint32_t>& initialCounts ) override;
	void SetUAVCS( uint16_t startSlot, const std::vector<UnorderedAccessView*>& uavs, const std::vector<uint32_t>& initialCounts ) override;
	void SetRTV_UAV( const std::vector<RenderTargetView*>& rtvs, DepthStencilView* dsv, uint16_t uavStartSlot,
					 const std::vector<UnorderedAccessView*>& uavs, const std::vector<uint32_t>& initialCounts ) override;

	void ClearRenderTarget( RenderTargetView* rtv, const Color& clearColor ) override;
	void ClearDeferredRTV( const Color& clearColor ) override;
	void ClearDepthStencil( DepthStencilView* dsv, bool clearDepth, bool clearStencil, float depthVal = 1.0f, uint8_t stencilVal = 0 ) override;
	void ClearUAV( UnorderedAccessView* uav, const float clearVal[4] ) override;
	void ClearUAV( UnorderedAccessView* uav, const uint32_t clearVal[4] ) override;
	void GetTextureDesc( TextureResource* tex, void* desc ) override;
	uint3 GetTextureDim( TextureResource* tex ) override;
	void GetSRVDesc( ShaderResourceView* srv, SRVDesc& desc ) override;
	void GetRTVDesc( RenderTargetView* rtv, RTVDesc& desc ) override;
	void GetDSVDesc( DepthStencilView* dsv, DSVDesc& desc ) override;
	void GetUAVDesc( UnorderedAccessView* uav, UAVDesc& desc ) override;

	bool CreateTexture( const TextureDesc& texDesc, TextureResource** tex, ShaderResourceView** srv, RenderTargetView** rtv, DepthStencilView** dsv, UnorderedAccessView** uav ) override;
	ShaderResourceView* CreateSRV( Resource* resource, const SRVDesc* desc ) override;
	RenderTargetView* CreateRTV( Resource* resource, const RTVDesc* desc ) override;
	DepthStencilView* CreateDSV( Resource* resource, const DSVDesc* desc ) override;
	UnorderedAccessView* CreateUAV( Resource* resource, const UAVDesc* desc ) override;
	Buffer* CreateBuffer( const void* data, const BufferDesc& desc ) override;

	void CopyResource( Resource* dst, Resource* src ) override;
	void UpdateBuffer( Buffer* buffer, void* data, UINT bufferSize, UINT subresource, MapType mapType, bool doNotWait = false ) override;
	void ReadBuffer( Buffer * buffer, UINT bufferSize, void* out, UINT cpySize, UINT subresource ) override;
	void MapBuffer( Buffer * buffer, void ** out, UINT subresource, MapType mapType ) override;
	void UnmapBuffer( Buffer* buffer, UINT subresource ) override;
	void UpdateSubresource( Resource* resource, UINT subresource, Box * box, void * data, UINT srcRowPitch, UINT srcDepthPitch ) override;
	void BindConstantBuffer( Buffer* buffer, ShaderFlag bindToShaders, UINT position ) override;

	void Draw( UINT vertexCount, UINT startLocation ) override;
	void DrawIndexed( UINT indexCount, UINT startIndexLocation, int baseVertexLocation ) override;
	void Dispatch( UINT groupCountX, UINT groupCountY, UINT groupCountZ ) override;

	Texture* GetDefaultRenderTarget() override {
		return m_DefaultRenderTarget;
	}
	Texture* GetDefaultDepthStencil() override {
		return m_DefaultDepthStencil;
	}

	void AddToVoxelization( GameObject& gameObject ) override;
	void NotifyChangeInVisibility() override;

	void SaveTexture( TextureResource* res, const std::wstring& fileName ) override;
	void TakeScreenShot( const std::wstring& fileName = L"ScreenShot" ) override;

	void SetLightSizeAngle( float angle ) override;
	void SetLightSamples( uint32_t samples ) override;
	void SetLightDir( float horizontal, float vertical ) override;
	void SetGUIVisibility( bool visibility ) override;

	void TraceWithEmdree() override;
	void CopyRTV_DSV() override;

	void RegisterResizeCallBack( void* Owner, std::function<void( uint2 )> callBack ) override;
	void UnRegisterCallBacks( void* Owner ) override;
private:
	D3DRenderBackend( Window& window );
	virtual ~D3DRenderBackend();

	virtual void Initialize();

	virtual IDXGIAdapter* GetAdapter( IDXGIFactory* factory );
	bool CreateDeferredRTVs();

	std::wstring m_TextureLocation = L"Assets\\Textures\\";

	ID3D11Device* m_Device = nullptr;
	IDXGISwapChain* m_SwapChain = nullptr;
	ID3D11DeviceContext* m_Context = nullptr;
	D3D11_VIEWPORT m_MainViewPort;

	Texture* m_DefaultRenderTarget;
	Texture* m_DefaultDepthStencil;
	Texture* m_TempRTV;
	Texture* m_TempDSV;
	Texture* m_ShadowTexture;
	std::array<Texture*,2> m_DeferredRTVs;

	RenderPass* m_SmoothPass = nullptr;

	std::vector<RasterizerState*> m_RasterizerStates;
	std::vector<DepthStencilState*> m_DepthStencilStates;
	std::vector<BlendState*> m_BlendStates;
	std::vector<SamplerState*> m_SamplerStates;
	std::vector<Buffer*> m_Buffers;

	Window* m_MainWindow;
	Scene* m_MainScene;

	Voxelizer* m_Voxelizer;
	std::vector<std::pair<const Geometry*, Matrix>> m_VoxelElements;
	bool m_RenderVoxel = false;
	bool m_RebuildTree = false;
	
	std::wstring m_ScreenShotName = L"ScreenShot";
	bool m_SaveScreenShot = false;

	Embree* m_Embree = nullptr;
	std::vector<GameObject*> m_EmbreeObjects;

	std::vector<std::pair<void*, std::function<void( uint2 )>>> m_ResizeCallbacks;
};

