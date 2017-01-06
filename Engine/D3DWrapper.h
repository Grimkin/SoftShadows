#pragma once
#include <d3d11.h>
#include <string>
#include "Types.h"

template<typename T>
struct Wrapper {
	Wrapper() {
	}
	Wrapper( T* ref ) : ref( ref ) {
	}
	T* ref = nullptr;
	operator T* ( ) {
		return ref;
	}
	T** operator& () {
		return &ref;
	}
	void Release() {
		if( ref )
			ref->Release();
		ref = nullptr;
	}
};

enum class CullMode {
	None = 1,
	Front = 2,
	Back = 3
};

enum class FillMode {
	Wireframe = 2,
	Solid = 3
};

enum class Format : unsigned int {
	Unknown = 0,
	R32G32B32A32_Typeless = 1,
	R32G32B32A32_Float = 2,
	R32G32B32A32_UInt = 3,
	R32G32B32A32_SInt = 4,
	R32G32B32_Typeless = 5,
	R32G32B32_Float = 6,
	R32G32B32_UInt = 7,
	R32G32B32_SInt = 8,
	R16G16B16A16_Typeless = 9,
	R16G16B16A16_Float = 10,
	R16G16B16A16_Unorm = 11,
	R16G16B16A16_UInt = 12,
	R16G16B16A16_Snorm = 13,
	R16G16B16A16_SInt = 14,
	R32G32_Typeless = 15,
	R32G32_Float = 16,
	R32G32_UInt = 17,
	R32G32_SInt = 18,
	R32G8X24_Typeless = 19,
	D32_Float_S8X24_UInt = 20,
	R32_Float_X8X24_Typeless = 21,
	X32_Typeless_G8X24_UInt = 22,
	R10G10B10A2_Typeless = 23,
	R10G10B10A2_Unorm = 24,
	R10G10B10A2_UInt = 25,
	R11G11B10_Float = 26,
	R8G8B8A8_Typeless = 27,
	R8G8B8A8_Unorm = 28,
	R8G8B8A8_Unorm_srgB = 29,
	R8G8B8A8_UInt = 30,
	R8G8B8A8_Snorm = 31,
	R8G8B8A8_SInt = 32,
	R16G16_Typeless = 33,
	R16G16_Float = 34,
	R16G16_Unorm = 35,
	R16G16_UInt = 36,
	R16G16_Snorm = 37,
	R16G16_SInt = 38,
	R32_Typeless = 39,
	D32_Float = 40,
	R32_Float = 41,
	R32_UInt = 42,
	R32_SInt = 43,
	R24G8_Typeless = 44,
	D24_Unorm_S8_UInt = 45,
	R24_Unorm_X8_Typeless = 46,
	X24_Typeless_G8_UInt = 47,
	R8G8_Typeless = 48,
	R8G8_Unorm = 49,
	R8G8_UInt = 50,
	R8G8_Snorm = 51,
	R8G8_SInt = 52,
	R16_Typeless = 53,
	R16_Float = 54,
	D16_Unorm = 55,
	R16_Unorm = 56,
	R16_UInt = 57,
	R16_Snorm = 58,
	R16_SInt = 59,
	R8_Typeless = 60,
	R8_Unorm = 61,
	R8_UInt = 62,
	R8_Snorm = 63,
	R8_SInt = 64,
	A8_Unorm = 65,
	R1_Unorm = 66,
	R9G9B9E5_SharedExp = 67,
	R8G8_B8G8_Unorm = 68,
	G8R8_G8B8_Unorm = 69,
	BC1_Typeless = 70,
	BC1_Unorm = 71,
	BC1_Unorm_SRGB = 72,
	BC2_Typeless = 73,
	BC2_Unorm = 74,
	BC2_Unorm_SRGB = 75,
	BC3_Typeless = 76,
	BC3_Unorm = 77,
	BC3_UnormSRGB = 78,
	BC4_Typeless = 79,
	BC4_Unorm = 80,
	BC4_Snorm = 81,
	BC5_Typeless = 82,
	BC5_Unorm = 83,
	BC5_Snorm = 84,
	B5G6R5_Unorm = 85,
	B5G5R5A1_Unorm = 86,
	B8G8R8A8_Unorm = 87,
	B8G8R8X8_Unorm = 88,
	R10G10B10_XR_Bias_A2_Unorm = 89,
	B8G8R8A8_Typeless = 90,
	B8G8R8A8_UnormSRGB = 91,
	B8G8R8X8_Typeless = 92,
	B8G8R8X8_UnormSRGB = 93,
	BC6H_Typeless = 94,
	BC6H_UF16 = 95,
	BC6H_SF16 = 96,
	BC7_Typeless = 97,
	BC7_Unorm = 98,
	BC7_Unorm_SRGB = 99,
	AYUV = 100,
	Y410 = 101,
	Y416 = 102,
	NV12 = 103,
	P010 = 104,
	P016 = 105,
	F420_Opaque = 106,
	YUY2 = 107,
	Y210 = 108,
	Y216 = 109,
	NV11 = 110,
	AI44 = 111,
	IA44 = 112,
	P8 = 113,
	A8P8 = 114,
	B4G4R4A4_Unorm = 115,
	ForceUInt = 0xffffffff
};

enum class InputSpecification {
	PerVertexData = 0,
	PerInstanceData = 1
};

struct RasterizerDesc {
	FillMode FillMode = FillMode::Solid;
	CullMode CullMode = CullMode::Back;
	BOOL FrontCounterClockwise = false;
	int DepthBias = 0;
	float DepthBiasClamp = 0.0f;
	float SlopeScaledDepthBias = 0.0f;
	BOOL DepthClipEnable = false;
	BOOL ScissorEnable = false;
	BOOL MultisampleEnable = false;
	BOOL AntialiasedLineEnable = false;
	operator D3D11_RASTERIZER_DESC () const {
		return *reinterpret_cast<const D3D11_RASTERIZER_DESC*>( this );
	}
};


enum class PrimitiveTopology {
	Undefined = 0,
	Pointlist = 1,
	Linelist = 2,
	Linestrip = 3,
	Trianglelist = 4,
	Trianglestrip = 5,
	Linelist_adj = 10,
	Linestrip_adj = 11,
	Trianglelist_adj = 12,
	Trianglestrip_adj = 13,
	_1_ControlPointPatchlist = 33,
	_2_ControlPointPatchlist = 34,
	_3_ControlPointPatchlist = 35,
	_4_ControlPointPatchlist = 36,
	_5_ControlPointPatchlist = 37,
	_6_ControlPointPatchlist = 38,
	_7_ControlPointPatchlist = 39,
	_8_ControlPointPatchlist = 40,
	_9_ControlPointPatchlist = 41,
	_10_ControlPointPatchlist = 42,
	_11_ControlPointPatchlist = 43,
	_12_ControlPointPatchlist = 44,
	_13_ControlPointPatchlist = 45,
	_14_ControlPointPatchlist = 46,
	_15_ControlPointPatchlist = 47,
	_16_ControlPointPatchlist = 48,
	_17_ControlPointPatchlist = 49,
	_18_ControlPointPatchlist = 50,
	_19_ControlPointPatchlist = 51,
	_20_ControlPointPatchlist = 52,
	_21_ControlPointPatchlist = 53,
	_22_ControlPointPatchlist = 54,
	_23_ControlPointPatchlist = 55,
	_24_ControlPointPatchlist = 56,
	_25_ControlPointPatchlist = 57,
	_26_ControlPointPatchlist = 58,
	_27_ControlPointPatchlist = 59,
	_28_ControlPointPatchlist = 60,
	_29_ControlPointPatchlist = 61,
	_30_ControlPointPatchlist = 62,
	_31_ControlPointPatchlist = 63,
	_32_ControlPointPatchlist = 64,
};

struct InputDesc {
	const char* SemanticName;
	unsigned int SemanticIndex;
	Format Format;
	unsigned int InputSlot;
	unsigned int AlignedByteOffset;
	InputSpecification InputSlotClass;
	unsigned int InstanceDataStepRate;
	operator D3D11_INPUT_ELEMENT_DESC () const {
		return *reinterpret_cast<const D3D11_INPUT_ELEMENT_DESC*>( this );
	}
};


enum class DepthWriteMask {
	Zero = 0,
	All = 1
};

enum class ComparisonFunc {
	Never = 1,
	Less = 2,
	Equal = 3,
	Less_Equal = 4,
	Greater = 5,
	NotEqual = 6,
	GreaterEqual = 7,
	Always = 8
};

enum class StencilOp {
	Keep = 1,
	Zero = 2,
	Replace = 3,
	IncrSat = 4,
	DecrSat = 5,
	Invert = 6,
	Incr = 7,
	Decr = 8
};

struct DepthStencilOpDesc {
	StencilOp StencilFailOp = StencilOp::Keep;
	StencilOp StencilDepthFailOp = StencilOp::Keep;
	StencilOp StencilPassOp = StencilOp::Keep;
	ComparisonFunc StencilFunc = ComparisonFunc::Never;
	operator D3D11_DEPTH_STENCILOP_DESC () const {
		return *reinterpret_cast<const D3D11_DEPTH_STENCILOP_DESC*>( this );
	}
};


struct DepthStencilDesc {
	BOOL DepthEnable = true;
	DepthWriteMask DepthWriteMask = DepthWriteMask::All;
	ComparisonFunc DepthFunc = ComparisonFunc::Less;
	BOOL StencilEnable = false;
	unsigned char StencilReadMask = 0xff;
	unsigned char StencilWriteMask = 0xff;
	DepthStencilOpDesc FrontFace;
	DepthStencilOpDesc BackFace;
	operator D3D11_DEPTH_STENCIL_DESC () const {
		return *reinterpret_cast<const D3D11_DEPTH_STENCIL_DESC*>( this );
	}
};

enum class Blend {
	Zero = 1,
	One = 2,
	SrcColor = 3,
	InvSrcColor = 4,
	SrcAlpha = 5,
	InvSrcAlpha = 6,
	DestAlpha = 7,
	InvDestAlpha = 8,
	DestColor = 9,
	InvDestColor = 10,
	SrcAlphaSat = 11,
	BlendFactor = 14,
	InvBlendFactor = 15,
	Src1Color = 16,
	InvSrc1Color = 17,
	Src1Alpha = 18,
	InvSrc1Alpha = 19
};

enum class BlendOp {
	Add = 1,
	Subtract = 2,
	RevSubtract = 3,
	Min = 4,
	Max = 5
};

enum class WriteEnable : unsigned char {
	Red = 1,
	Green = 2,
	Blue = 4,
	Alpha = 8,
	All = ( ( ( Red | Green ) | Blue ) | Alpha )
};

inline WriteEnable operator|( WriteEnable flag1, WriteEnable flag2 ) {
	typedef std::underlying_type<WriteEnable>::type enum_type;
	return static_cast<WriteEnable>( static_cast<enum_type>( flag1 ) | static_cast<enum_type>( flag2 ) );
}

struct RTVBlendDesc {
	BOOL BlendEnable = false;
	Blend SrcBlend = Blend::SrcAlpha;
	Blend DestBlend = Blend::InvSrcAlpha;
	BlendOp BlendOpColor = BlendOp::Add;
	Blend SrcBlendAlpha = Blend::SrcAlpha;
	Blend DestBlendAlpha = Blend::InvSrcAlpha;
	BlendOp BlendOpAlpha = BlendOp::Add;
	WriteEnable RenderTargetWriteMask = WriteEnable::All;
	operator D3D11_RENDER_TARGET_BLEND_DESC () const {
		return *reinterpret_cast<const D3D11_RENDER_TARGET_BLEND_DESC*>( this );
	}
};


struct BlendDesc {
	BOOL AlphaToCoverageEnable = false;
	BOOL IndependentBlendEnable = false;
	RTVBlendDesc RenderTarget[8];
	operator D3D11_BLEND_DESC () const {
		return *reinterpret_cast<const D3D11_BLEND_DESC*>( this );
	}
};

enum class Usage {
	Default = 0,
	Immutable = 1,
	Dynamic = 2,
	Staging = 3
};

enum class BindFlag : unsigned int {
	None = 0x0l,
	VertexBuffer = 0x1l,
	IndexBuffer = 0x2l,
	ConstantBuffer = 0x4l,
	ShaderResource = 0x8l,
	StreamOutput = 0x10l,
	RenderTarget = 0x20l,
	DepthStencil = 0x40l,
	UnorderedAccess = 0x80l,
	Decoder = 0x200l,
	VideoEncoder = 0x400l
};

inline BindFlag operator|( BindFlag flag1, BindFlag flag2 ) {
	typedef std::underlying_type<WriteEnable>::type enum_type;
	return static_cast<BindFlag>( static_cast<enum_type>( flag1 ) | static_cast<enum_type>( flag2 ) );
}

inline BindFlag& operator|=( BindFlag& flag1, const BindFlag& flag2 ) {
	typedef std::underlying_type<WriteEnable>::type enum_type;
	return flag1 = flag1 | flag2;
}

enum class CPUAccessFlag : unsigned int {
	None = 0x0l,
	Write = 0x10000l,
	Read = 0x20000l
};

inline CPUAccessFlag operator|( CPUAccessFlag flag1, CPUAccessFlag flag2 ) {
	typedef std::underlying_type<CPUAccessFlag>::type enum_type;
	return static_cast<CPUAccessFlag>( static_cast<enum_type>( flag1 ) | static_cast<enum_type>( flag2 ) );
}

enum class ResourceMiscFlag : unsigned int {
	None = 0x0l,
	GenerateMips = 0x1l,
	Shared = 0x2l,
	TextureCube = 0x4l,
	DrawindirectArgs = 0x10l,
	BufferAllowRawViews = 0x20l,
	BufferStructured = 0x40l,
	ResourceClamp = 0x80l,
	SharedKeyedmutex = 0x100l,
	GDICompatible = 0x200l
};

inline ResourceMiscFlag operator|( ResourceMiscFlag flag1, ResourceMiscFlag flag2 ) {
	typedef std::underlying_type<ResourceMiscFlag>::type enum_type;
	return static_cast<ResourceMiscFlag>( static_cast<enum_type>( flag1 ) | static_cast<enum_type>( flag2 ) );
}

enum class MapType {
	Read = 1,
	Write = 2,
	ReadWrite = 3,
	WriteDiscard = 4,
	WriteNoOverwrite = 5
};

struct BufferDesc {
	unsigned int ByteWidth = 0;
	Usage Usage = Usage::Default;
	BindFlag BindFlags = BindFlag::None;
	CPUAccessFlag CPUAccessFlags = CPUAccessFlag::None;
	ResourceMiscFlag MiscFlags = ResourceMiscFlag::None;
	unsigned int StructureByteStride = 0;
	operator D3D11_BUFFER_DESC() const {
		return *reinterpret_cast<const D3D11_BUFFER_DESC*> ( this );
	}
};

// used to specify type of shader, no |operator allowed
enum class ShaderType {
	VertexShader = 1,
	PixelShader = ( 1 << 1 ),
	GeometryShader = ( 1 << 2 ),
	HullShader = ( 1 << 3 ),
	ComputeShader = ( 1 << 4 ),
	DomainShader = ( 1 << 5 )
};

// used to specify to which shaders something should apply
enum class ShaderFlag {
	None = 0,
	VertexShader = 1,
	PixelShader = ( 1 << 1 ),
	GeometryShader = ( 1 << 2 ),
	HullShader = ( 1 << 3 ),
	ComputeShader = ( 1 << 4 ),
	DomainShader = ( 1 << 5 )
};

inline ShaderFlag operator|( ShaderFlag flag1, ShaderFlag flag2 ) {
	typedef std::underlying_type<ShaderFlag>::type enum_type;
	return static_cast<ShaderFlag>( static_cast<enum_type>( flag1 ) | static_cast<enum_type>( flag2 ) );
}

enum class Filter {
	MinMagMipPoint = 0,
	MinMagPointMipLinear = 0x1,
	MinPointMagLinearMipPoint = 0x4,
	MinPointMagMipLinear = 0x5,
	MinLinearMagMipPoint = 0x10,
	MinLinearMagPointMipLinear = 0x11,
	MinMagLinearMipPoint = 0x14,
	MinMagMipLinear = 0x15,
	Anisotropic = 0x55,
	ComparisonMinMagMipPoint = 0x80,
	ComparisonMinMagPointMipLinear = 0x81,
	ComparisonMinPointMagLinearMipPoint = 0x84,
	ComparisonMinPointMagMipLinear = 0x85,
	ComparisonMinLinearMagMipPoint = 0x90,
	ComparisonMinLinearMagPointMipLinear = 0x91,
	ComparisonMinMagLinearMipPoint = 0x94,
	ComparisonMinMagMipLinear = 0x95,
	ComparisonAnisotropic = 0xd5,
	MinimumMinMagMipPoint = 0x100,
	MinimumMinMagPointMipLinear = 0x101,
	MinimumMinPointMagLinearMipPoint = 0x104,
	MinimumMinPointMagMipLinear = 0x105,
	MinimumMinLinearMagMipPoint = 0x110,
	MinimumMinLinearMagPointMipLinear = 0x111,
	MinimumMinMagLinearMipPoint = 0x114,
	MinimumMinMagMipLinear = 0x115,
	MinimumAnisotropic = 0x155,
	MaximumMinMagMipPoint = 0x180,
	MaximumMinMagPointMipLinear = 0x181,
	MaximumMinPointMagLinearMipPoint = 0x184,
	MaximumMinPointMagMipLinear = 0x185,
	MaximumMinLinearMagMipPoint = 0x190,
	MaximumMinLinearMagPointMipLinear = 0x191,
	MaximumMinMagLinearMipPoint = 0x194,
	MaximumMinMagMipLinear = 0x195,
	MaximumAnisotropic = 0x1d5
};

enum class AddressMode {
	Wrap = 1,
	Mirror = 2,
	Clamp = 3,
	Border = 4,
	MirrorOnce = 5
};
struct SamplerDesc {
	Filter Filter = Filter::MinMagMipLinear;
	AddressMode AddressU = AddressMode::Clamp;
	AddressMode AddressV = AddressMode::Clamp;
	AddressMode AddressW = AddressMode::Clamp;
	float MipLODBias = 0.0f;
	uint32_t MaxAnisotropy = 16;
	ComparisonFunc ComparisonFunc = ComparisonFunc::Never;
	float4 BorderColor = { 0.0f,0.0f,0.0f,0.0f };
	float MinLOD = -3.402823466e+38F;
	float MaxLOD = 3.402823466e+38F;
	operator D3D11_SAMPLER_DESC() const {
		return *reinterpret_cast<const D3D11_SAMPLER_DESC*>( this );
	}
};

enum class SRVDimension {
	Unknown = 0,
	Buffer = 1,
	Texture1D = 2,
	Texture1DArray = 3,
	Texture2D = 4,
	Texture2DArray = 5,
	Texture2DMS = 6,
	Texture2DMSArray = 7,
	Texture3D = 8,
	TextureCube = 9,
	TextureCubeArray = 10,
	BufferEx = 11,
};

struct SRVDesc {
	SRVDesc() {
		memset( this, 0, sizeof( SRVDesc ) );
	}
	Format Format;
	SRVDimension ViewDimension;
	union {
		BufferSRV Buffer;
		Texture1DSRV Texture1D;
		Texture1DArraySRV Texture1DArray;
		Texture2DSRV Texture2D;
		Texture2DArraySRV Texture2DArray;
		Texture2DMSSRV Texture2DMS;
		Texture2DMSArraySRV Texture2DMSArray;
		Texture3DSRV Texture3D;
		TextureCubeSRV TextureCube;
		TextureCubeArraySRV TextureCubeArray;
		BufferExSRV BufferEx;
	};
	operator D3D11_SHADER_RESOURCE_VIEW_DESC() const {
		static_assert( sizeof( SRVDesc ) == sizeof( D3D11_SHADER_RESOURCE_VIEW_DESC ), "size must be the same" );
		return *reinterpret_cast<const D3D11_SHADER_RESOURCE_VIEW_DESC*>( this );
	}
};

enum class RTVDimension {
	Unknown = 0,
	Buffer = 1,
	Texture1D = 2,
	Texture1DArray = 3,
	Texture2D = 4,
	Texture2DArray = 5,
	Texture2DMS = 6,
	Texture2DMSArray = 7,
	Texture3D = 8
};

struct RTVDesc {
	RTVDesc() {
		memset( this, 0, sizeof( RTVDesc ) );
	}
	Format Format;
	RTVDimension ViewDimension;
	union {
		BufferRTV Buffer;
		Texture1DRTV Texture1D;
		Texture1DArrayRTV Texture1DArray;
		Texture2DRTV Texture2D;
		Texture2DArrayRTV Texture2DArray;
		Texture2DMSRTV Texture2DMS;
		Texture2DMSArrayRTV Texture2DMSArray;
		Texture3DRTV Texture3D;
	};
	operator D3D11_RENDER_TARGET_VIEW_DESC() const {
		static_assert( sizeof( RTVDesc ) == sizeof( D3D11_RENDER_TARGET_VIEW_DESC ), "size must be the same" );
		return *reinterpret_cast<const D3D11_RENDER_TARGET_VIEW_DESC*>( this );
	}
};

enum class DSVDimension {
	Unknown = 0,
	Texture1D = 1,
	Texture1DArray = 2,
	Texture2D = 3,
	Texture2DArray = 4,
	Texture2DMS = 5,
	Texture2DMSArray = 6,
};

enum class DSVReadOnly : unsigned int {
	None = 0x0,
	ReadOnlyDepth = 0x1,
	ReadOnlyStencil = 0x2
};

struct DSVDesc {
	DSVDesc() {
		memset( this, 0, sizeof( DSVDesc ) );
	}
	Format Format;
	DSVDimension ViewDimension;
	DSVReadOnly Flags;
	union {
		Texture1DDSV Texture1D;
		Texture1DArrayDSV Texture1DArray;
		Texture2DDSV Texture2D;
		Texture2DArrayDSV Texture2DArray;
		Texture2DMSDSV Texture2DMS;
		Texture2DMSArrayDSV Texture2DMSArray;
	};

	operator D3D11_DEPTH_STENCIL_VIEW_DESC() const {
		static_assert( sizeof( DSVDesc ) == sizeof( D3D11_DEPTH_STENCIL_VIEW_DESC ), "size must be the same" );
		return *reinterpret_cast<const D3D11_DEPTH_STENCIL_VIEW_DESC*>( this );
	}
};

enum class UAVDimension {
	Unknown = 0,
	Buffer = 1,
	Texture1d = 2,
	Texture1darray = 3,
	Texture2d = 4,
	Texture2darray = 5,
	Texture3d = 8
};

enum class UAVBufferFlag : unsigned int {
	None = 0x0,
	Raw = 0x1,
	Append = 0x2,
	Counter = 0x4
};

struct BufferUAV {
	UINT FirstElement;
	UINT NumElements;
	UAVBufferFlag Flags;
	operator D3D11_BUFFER_UAV() const {
		static_assert( sizeof( BufferUAV ) == sizeof( D3D11_BUFFER_UAV ), "size must be the same" );
		return *reinterpret_cast<const D3D11_BUFFER_UAV*>( this );
	}
};

struct UAVDesc {
	Format Format;
	UAVDimension ViewDimension;
	union {
		BufferUAV Buffer;
		Texture1DUAV Texture1D;
		Texture1DArrayUAV Texture1DArray;
		Texture2DUAV Texture2D;
		Texture2DArrayUAV Texture2DArray;
		Texture3DUAV Texture3D;
	};
	operator D3D11_UNORDERED_ACCESS_VIEW_DESC() const {
		static_assert( sizeof( UAVDesc ) == sizeof( D3D11_UNORDERED_ACCESS_VIEW_DESC ), "size must be the same" );
		return *reinterpret_cast<const D3D11_UNORDERED_ACCESS_VIEW_DESC*>( this );
	}
};

struct Texture1DDesc {
	UINT Width = 1;
	UINT MipLevels = 1;
	UINT ArraySize = 1;
	Format Format = Format::Unknown;
	Usage Usage = Usage::Default;
	BindFlag BindFlags = BindFlag::None;
	CPUAccessFlag CPUAccessFlags = CPUAccessFlag::None;
	ResourceMiscFlag MiscFlags = ResourceMiscFlag::None;
	operator D3D11_TEXTURE1D_DESC() const {
		return *reinterpret_cast<const D3D11_TEXTURE1D_DESC*>( this );
	}
};

struct Texture2DDesc {
	UINT Width = 1;
	UINT Height = 1;
	UINT MipLevels = 1;
	UINT ArraySize = 1;
	Format Format = Format::Unknown;
	SampleDesc SampleDesc = { 1, 0 };
	Usage Usage = Usage::Default;
	BindFlag BindFlags = BindFlag::None;
	CPUAccessFlag CPUAccessFlags = CPUAccessFlag::None;
	ResourceMiscFlag MiscFlags = ResourceMiscFlag::None;
	operator D3D11_TEXTURE2D_DESC() const {
		return *reinterpret_cast<const D3D11_TEXTURE2D_DESC*>( this );
	}
};

struct Texture3DDesc {
	UINT Width = 1;
	UINT Height = 1;
	UINT Depth = 1;
	UINT MipLevels = 1;
	Format Format = Format::Unknown;
	Usage Usage = Usage::Default;
	BindFlag BindFlags = BindFlag::None;
	CPUAccessFlag CPUAccessFlags = CPUAccessFlag::None;
	ResourceMiscFlag MiscFlags = ResourceMiscFlag::None;
	operator D3D11_TEXTURE3D_DESC() const {
		return *reinterpret_cast<const D3D11_TEXTURE3D_DESC*>( this );
	}
};

struct Viewport {
	float TopLeftX = 0.0f;
	float TopLeftY = 0.0f;
	float Width = 100.0f;
	float Height = 100.0f;
	float MinDepth = 0.0f;
	float MaxDepth = 1.0f;
	operator D3D11_VIEWPORT() const {
		return *reinterpret_cast<const D3D11_VIEWPORT*>( this );
	}
};

namespace std {
	template<>
	class hash<InputDesc> {
	public:
		size_t operator() ( const InputDesc& inputDesc ) {
			size_t out = std::hash<std::string>()( inputDesc.SemanticName );
			out ^= std::hash<Format>()( inputDesc.Format );
			out ^= std::hash<u_int>()( inputDesc.InputSlot );
			return out;
		}
	};

	template<>
	class hash<RasterizerDesc> {
	public:
		size_t operator()( const RasterizerDesc& rs ) const {
			size_t h = hash<FillMode>()( rs.FillMode ) << 1;
			h ^= hash<CullMode>()( rs.CullMode ) << 2;
			h ^= hash<BOOL>()( rs.FrontCounterClockwise ) << 3;
			h ^= hash<int>()( rs.DepthBias ) << 4;
			h ^= hash<float>()( rs.DepthBiasClamp ) << 5;
			h ^= hash<float>()( rs.SlopeScaledDepthBias ) << 6;
			h ^= hash<BOOL>()( rs.DepthClipEnable ) << 7;
			h ^= hash<BOOL>()( rs.ScissorEnable ) << 8;
			h ^= hash<BOOL>()( rs.MultisampleEnable ) << 9;
			h ^= hash<BOOL>()( rs.AntialiasedLineEnable ) << 10;
			return h;
		}
	};

	template<>
	class hash<DepthStencilOpDesc> {
	public:
		size_t operator()( const DepthStencilOpDesc& dsOPd ) const {
			size_t h = std::hash<StencilOp>()( dsOPd.StencilFailOp );
			h ^= std::hash<StencilOp>()( dsOPd.StencilDepthFailOp ) << 1;
			h ^= std::hash<StencilOp>()( dsOPd.StencilPassOp ) << 2;
			h ^= std::hash<ComparisonFunc>()( dsOPd.StencilFunc ) << 3;
			return h;
		}
	};

	template<>
	class hash<DepthStencilDesc> {
	public:
		size_t operator()( const DepthStencilDesc& dsd ) const {
			size_t h = std::hash<BOOL>()( dsd.DepthEnable );
			h ^= std::hash<DepthWriteMask>()( dsd.DepthWriteMask ) << 1;
			h ^= std::hash<ComparisonFunc>()( dsd.DepthFunc ) << 2;
			h ^= std::hash<BOOL>()( dsd.StencilEnable ) << 3;
			h ^= std::hash<unsigned char>()( dsd.StencilReadMask ) << 4;
			h ^= std::hash<unsigned char>()( dsd.StencilWriteMask ) << 5;
			h ^= std::hash<DepthStencilOpDesc>()( dsd.FrontFace ) << 6;
			h ^= std::hash<DepthStencilOpDesc>()( dsd.BackFace ) << 7;
			return h;
		}
	};

	template<>
	class hash<RTVBlendDesc> {
	public:
		size_t operator()( const RTVBlendDesc& bs ) const {
			size_t h = std::hash<BOOL>()( bs.BlendEnable );
			h ^= std::hash<Blend>()( bs.SrcBlend ) << 1;
			h ^= std::hash<Blend>()( bs.DestBlend ) << 2;
			h ^= std::hash<BlendOp>()( bs.BlendOpColor ) << 3;
			h ^= std::hash<Blend>()( bs.SrcBlendAlpha ) << 4;
			h ^= std::hash<Blend>()( bs.DestBlendAlpha ) << 5;
			h ^= std::hash<BlendOp>()( bs.BlendOpAlpha ) << 6;
			h ^= std::hash<WriteEnable>()( bs.RenderTargetWriteMask ) << 7;
			return h;
		}
	};

	template<>
	class hash<BlendDesc> {
	public:
		size_t operator()( const BlendDesc& bs ) const {
			size_t h = std::hash<BOOL>()( bs.AlphaToCoverageEnable );
			h ^= std::hash<BOOL>()( bs.IndependentBlendEnable ) << 1;
			for( int i = 0; i < 8; ++i ) {
				h ^= std::hash<RTVBlendDesc>() ( bs.RenderTarget[i] ) << ( ( 4 * i ) + 2 );
			}
			return h;
		}
	};
	template<>
	class hash<SamplerDesc> {
	public:
		size_t operator()( const SamplerDesc& desc ) const {
			size_t out = static_cast<size_t>( desc.Filter );
			out ^= static_cast<size_t>( desc.AddressU ) << 4;
			out ^= static_cast<size_t>( desc.AddressV ) << 7;
			out ^= static_cast<size_t>( desc.AddressW ) << 10;
			out ^= *reinterpret_cast<const size_t*>( &desc.BorderColor.x ) << 12;
			out ^= *reinterpret_cast<const size_t*>( &desc.BorderColor.y ) << 14;
			out ^= *reinterpret_cast<const size_t*>( &desc.BorderColor.z ) << 16;
			out ^= *reinterpret_cast<const size_t*>( &desc.BorderColor.w ) << 18;
			out ^= static_cast<size_t>( desc.MaxAnisotropy ) << 20;
			out ^= *reinterpret_cast<const size_t*>( &desc.MaxLOD ) << 22;
			out ^= *reinterpret_cast<const size_t*>( &desc.MinLOD ) << 24;
			out ^= *reinterpret_cast<const size_t*>( &desc.MipLODBias ) << 26;
			out ^= static_cast<size_t>( desc.ComparisonFunc ) << 28;
			return out;
		}
	};
	template<>
	class hash<SRVDesc> {
	public:
		size_t operator()( const SRVDesc& desc ) const {
			size_t out;
			out = static_cast<size_t>( desc.ViewDimension );
			out ^= static_cast<size_t>( desc.Format ) << 4;
			out ^= reinterpret_cast<const size_t>( &desc.Texture2D );
			return out;
		}
	};
	template<>
	class hash<RTVDesc> {
	public:
		size_t operator()( const RTVDesc& desc ) const {
			size_t out;
			out = static_cast<size_t>( desc.ViewDimension );
			out ^= static_cast<size_t>( desc.Format ) << 4;
			out ^= reinterpret_cast<const size_t>( &desc.Texture2D );
			return out;
		}
	};
	template<>
	class hash<DSVDesc> {
	public:
		size_t operator()( const DSVDesc& desc ) const {
			size_t out;
			out = static_cast<size_t>( desc.ViewDimension );
			out ^= static_cast<size_t>( desc.Format ) << 4;
			out ^= reinterpret_cast<const size_t>( &desc.Texture2D );
			return out;
		}
	};
}