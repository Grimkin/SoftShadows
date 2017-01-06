#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

template <typename T>
struct Wrapper;

// D3D types
typedef ID3D11Buffer Buffer;
typedef ID3D11RasterizerState RasterizerState;
typedef ID3D11DepthStencilState DepthStencilState;
typedef ID3D11BlendState BlendState;
typedef ID3D11SamplerState SamplerState;

typedef ID3D11Resource TextureResource;
typedef ID3D11Resource Resource;
typedef ID3D11Texture1D Texture1D;
typedef ID3D11Texture2D Texture2D;
typedef ID3D11Texture3D Texture3D;
typedef ID3D11ShaderResourceView ShaderResourceView;
typedef ID3D11RenderTargetView RenderTargetView;
typedef ID3D11DepthStencilView DepthStencilView;
typedef ID3D11UnorderedAccessView UnorderedAccessView;

typedef D3D11_BUFFER_SRV BufferSRV;
typedef D3D11_TEX1D_SRV Texture1DSRV;
typedef D3D11_TEX1D_ARRAY_SRV Texture1DArraySRV;
typedef D3D11_TEX2D_SRV Texture2DSRV;
typedef D3D11_TEX2D_ARRAY_SRV Texture2DArraySRV;
typedef D3D11_TEX2DMS_SRV Texture2DMSSRV;
typedef D3D11_TEX2DMS_ARRAY_SRV Texture2DMSArraySRV;
typedef D3D11_TEX3D_SRV Texture3DSRV;
typedef D3D11_TEXCUBE_SRV TextureCubeSRV;
typedef D3D11_TEXCUBE_ARRAY_SRV TextureCubeArraySRV;
typedef D3D11_BUFFEREX_SRV BufferExSRV;

typedef D3D11_BUFFER_RTV BufferRTV;
typedef D3D11_TEX1D_RTV Texture1DRTV;
typedef D3D11_TEX1D_ARRAY_RTV Texture1DArrayRTV;
typedef D3D11_TEX2D_RTV Texture2DRTV;
typedef D3D11_TEX2D_ARRAY_RTV Texture2DArrayRTV;
typedef D3D11_TEX2DMS_RTV Texture2DMSRTV;
typedef D3D11_TEX2DMS_ARRAY_RTV Texture2DMSArrayRTV;
typedef D3D11_TEX3D_RTV Texture3DRTV;

typedef D3D11_TEX1D_DSV Texture1DDSV;
typedef D3D11_TEX1D_ARRAY_DSV Texture1DArrayDSV;
typedef D3D11_TEX2D_DSV Texture2DDSV;
typedef D3D11_TEX2D_ARRAY_DSV Texture2DArrayDSV;
typedef D3D11_TEX2DMS_DSV Texture2DMSDSV;
typedef D3D11_TEX2DMS_ARRAY_DSV Texture2DMSArrayDSV;

typedef D3D11_TEX1D_UAV Texture1DUAV;
typedef D3D11_TEX1D_ARRAY_UAV Texture1DArrayUAV;
typedef D3D11_TEX2D_UAV Texture2DUAV;
typedef D3D11_TEX2D_ARRAY_UAV Texture2DArrayUAV;
typedef D3D11_TEX3D_UAV Texture3DUAV;

typedef DXGI_SAMPLE_DESC SampleDesc;
typedef D3D11_SUBRESOURCE_DATA SubresourceData;
typedef D3D11_BOX Box;

struct RasterizerDesc;
struct DepthStencilDesc;
struct BlendDesc;
struct BufferDesc;
struct SamplerDesc;
struct Texture1DDesc;
struct Texture2DDesc;
struct Texture3DDesc;
struct SRVDesc;
struct RTVDesc;
struct DSVDesc;
struct UAVDesc;

struct Viewport;

enum class PrimitiveTopology;
enum class MapType;
enum class ShaderType;
enum class ShaderFlag;
enum class Format : unsigned int;
enum class BindFlag : unsigned int;
enum class CPUAccessFlag : unsigned int;
enum class ResourceMiscFlag : unsigned int;


// VectorTypes
typedef DirectX::XMFLOAT2 float2;
typedef DirectX::XMFLOAT3 float3;
typedef DirectX::XMFLOAT4 float4;
typedef DirectX::XMFLOAT4 Quaternion;
typedef float4 Color;

typedef DirectX::XMFLOAT2A float2a;
typedef DirectX::XMFLOAT3A float3a;
typedef DirectX::XMFLOAT4A float4a;

typedef DirectX::XMINT2 int2;
typedef DirectX::XMINT3 int3;
typedef DirectX::XMINT4 int4;
typedef DirectX::XMUINT2 uint2;
typedef DirectX::XMUINT3 uint3;
typedef DirectX::XMUINT4 uint4;

// Matrices
typedef DirectX::XMFLOAT4X4 Matrix;

// Rotation
extern Quaternion QuaternionIdentity;