#pragma once

#include <Windows.h>
#include <vector>
#include <functional>

#include "Types.h"

class RenderPass;
class Texture;
class Geometry;
class Transform;
class GameObject;
struct RenderPassInit;
struct TextureDesc;
union TexDesc;
enum class TextureType;

class RenderBackend {
public:
	RenderBackend() {
	}
	virtual ~RenderBackend() {
	}

	virtual void Start() = 0;
	virtual void Render() = 0;
	virtual void Exit() = 0;

	virtual RasterizerState* GetRasterizeState( RasterizerDesc& desc ) = 0;
	virtual DepthStencilState* GetDepthStencilState( DepthStencilDesc& desc ) = 0;
	virtual BlendState* GetBlendState( BlendDesc& desc ) = 0;
	virtual SamplerState* GetSamplerState( SamplerDesc& desc ) = 0;

	virtual void SetRenderTarget( const std::vector<RenderTargetView*>& rtvs, DepthStencilView* dsv ) = 0;
	virtual void SetDeferredRTVs() = 0;
	virtual void SetViewports( const std::vector<Viewport>& viewports ) = 0;
	virtual void SetRasterizerState( RasterizerState* rasterizerState ) = 0;
	virtual void SetDepthStencilState( DepthStencilState* depthStencilState ) = 0;
	virtual void SetBlendState( BlendState* blendState, const float* blendFactor, UINT sampleMask ) = 0;
	virtual void SetSamplerVS( uint16_t startSlot, const std::vector<SamplerState*>& samplers ) = 0;
	virtual void SetSamplerPS( uint16_t startSlot, const std::vector<SamplerState*>& samplers ) = 0;
	virtual void SetSamplerGS( uint16_t startSlot, const std::vector<SamplerState*>& samplers ) = 0;
	virtual void SetSamplerHS( uint16_t startSlot, const std::vector<SamplerState*>& samplers ) = 0;
	virtual void SetSamplerDS( uint16_t startSlot, const std::vector<SamplerState*>& samplers ) = 0;
	virtual void SetSamplerCS( uint16_t startSlot, const std::vector<SamplerState*>& samplers ) = 0;
	virtual void SetSRVsVS( uint16_t startSlot, const std::vector<ShaderResourceView*>& samplers ) = 0;
	virtual void SetSRVsPS( uint16_t startSlot, const std::vector<ShaderResourceView*>& samplers ) = 0;
	virtual void SetSRVsGS( uint16_t startSlot, const std::vector<ShaderResourceView*>& samplers ) = 0;
	virtual void SetSRVsHS( uint16_t startSlot, const std::vector<ShaderResourceView*>& samplers ) = 0;
	virtual void SetSRVsDS( uint16_t startSlot, const std::vector<ShaderResourceView*>& samplers ) = 0;
	virtual void SetSRVsCS( uint16_t startSlot, const std::vector<ShaderResourceView*>& samplers ) = 0;
	virtual void SetUAVPS( uint16_t startSlot, const std::vector<UnorderedAccessView*>& uavs, const std::vector<uint32_t>& initialCounts ) = 0;
	virtual void SetUAVCS( uint16_t startSlot, const std::vector<UnorderedAccessView*>& uavs, const std::vector<uint32_t>& initialCounts ) = 0;
	virtual void SetRTV_UAV( const std::vector<RenderTargetView*>& rtvs, DepthStencilView* dsv, uint16_t uavStartSlot, 
							 const std::vector<UnorderedAccessView*>& uavs, const std::vector<uint32_t>& initialCounts ) = 0;

	virtual void ClearRenderTarget( RenderTargetView* rtv, const Color& clearColor ) = 0;
	virtual void ClearDeferredRTV( const Color& clearColor ) = 0;
	virtual void ClearDepthStencil( DepthStencilView* dsv, bool clearDepth, bool clearStencil, float depthVal = 1.0f, uint8_t stencilVal = 0 ) = 0;
	virtual void ClearUAV( UnorderedAccessView* uav, const float clearVal[4] ) = 0;
	virtual void ClearUAV( UnorderedAccessView* uav, const uint32_t clearVal[4] ) = 0;

	virtual bool CreateTexture( const TextureDesc& texDesc, TextureResource** tex, ShaderResourceView** srv, RenderTargetView** rtv, DepthStencilView** dsv, UnorderedAccessView** uav ) = 0;
	virtual ShaderResourceView* CreateSRV( Resource* resource, const SRVDesc* desc ) = 0;
	virtual RenderTargetView* CreateRTV( Resource* resource, const RTVDesc* desc ) = 0;
	virtual DepthStencilView* CreateDSV( Resource* resource, const DSVDesc* desc ) = 0;
	virtual UnorderedAccessView* CreateUAV( Resource* resource, const UAVDesc* desc ) = 0;
	virtual void GetTextureDesc( TextureResource* tex, void* desc ) = 0;
	virtual uint3 GetTextureDim( TextureResource* tex ) = 0;
	virtual void GetSRVDesc( ShaderResourceView* srv, SRVDesc& desc ) = 0;
	virtual void GetRTVDesc( RenderTargetView* rtv, RTVDesc& desc ) = 0;
	virtual void GetDSVDesc( DepthStencilView* dsv, DSVDesc& desc ) = 0;
	virtual void GetUAVDesc( UnorderedAccessView* uav, UAVDesc& desc ) = 0;

	virtual void SetTopology( PrimitiveTopology topology ) = 0;
	virtual void SetVertexBuffer( UINT startIdx, std::vector<Buffer*> buffers, std::vector<UINT> strides, std::vector<UINT> offsets ) = 0;
	virtual void SetIndexBuffer( Buffer* buffer, Format format, uint32_t offset ) = 0;

	virtual void OnWindowChange( bool fullScreen, uint2 size ) = 0;

	virtual void Draw( UINT vertexCount, UINT vertexOffset ) = 0;
	virtual void DrawIndexed( UINT indexCount, UINT startIndexLocation, int baseVertexLocation ) = 0;
	virtual void Dispatch( UINT groupCountX, UINT groupCountY, UINT groupCountZ ) = 0;

	template<typename T>
	Buffer* CreateVertexBuffer( std::vector<T> vertices ) {
		BufferDesc desc;
		desc.Usage = Usage::Default;
		desc.BindFlags = BindFlag::VertexBuffer;
		desc.ByteWidth = static_cast<UINT>( sizeof( T ) * vertices.size() );
		return CreateBuffer( vertices.data(), desc );
	}

	template<typename T>
	Buffer* CreateIndexBuffer( std::vector<T> indices ) {
		BufferDesc desc;
		desc.Usage = Usage::Default;
		desc.BindFlags = BindFlag::IndexBuffer;
		desc.ByteWidth = static_cast<UINT>( sizeof( T ) * indices.size() );
		return CreateBuffer( indices.data(), desc );
	}


	virtual Buffer* CreateBuffer( const void* data, const BufferDesc& desc ) = 0;

	virtual void CopyResource( Resource* dst, Resource* src ) = 0;
	virtual void UpdateBuffer( Buffer* buffer, void* data, UINT bufferSize, UINT subresource, MapType mapType, bool doNotWait = false ) = 0;
	virtual void ReadBuffer( Buffer * buffer, UINT bufferSize, void* out, UINT cpySize, UINT subresource ) = 0;
	virtual void MapBuffer( Buffer * buffer, void ** out, UINT subresource, MapType mapType ) = 0;
	virtual void UnmapBuffer( Buffer* buffer, UINT subresource ) = 0;
	virtual void UpdateSubresource( Resource* resource, UINT subresource, Box * box, void * data, UINT srcRowPitch, UINT srcDepthPitch ) = 0;
	virtual void BindConstantBuffer( Buffer* buffer, ShaderFlag bindToShaders, UINT position ) = 0;

	virtual Texture* GetDefaultRenderTarget() = 0;
	virtual Texture* GetDefaultDepthStencil() = 0;

	virtual void AddToVoxelization( GameObject& gameObject ) = 0;
	virtual void NotifyChangeInVisibility() = 0;

	virtual void SaveTexture( TextureResource* res, const std::wstring& fileName ) = 0;
	virtual void TakeScreenShot( const std::wstring& fileName = L"ScreenShot" ) = 0;

	virtual void SetLightSizeAngle( float angle ) = 0;
	virtual void SetLightSamples( uint32_t samples ) = 0;
	virtual void SetLightDir( float horizontal, float vertical ) = 0;
	virtual void SetGUIVisibility( bool visibility ) = 0;

	virtual void TraceWithEmdree() = 0;

	virtual void CopyRTV_DSV() = 0;

	virtual void RegisterResizeCallBack( void* Owner, std::function<void( uint2 )> callBack ) = 0;
	virtual void UnRegisterCallBacks( void* Owner ) = 0;
};

