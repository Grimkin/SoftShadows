#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include "D3DWrapper.h"


enum class TextureType {
	Texture1D,
	Texture1DArray,
	Texture2D,
	Texture2DArray,
	Texture3D,
	TextureCube
};

struct TextureDesc {
	std::wstring UniqueName;
	TextureType TextureType;
	union {
		Texture1DDesc* Texture1DDesc;
		Texture2DDesc* Texture2DDesc;
		Texture3DDesc* Texture3DDesc;
	};
	SubresourceData* InitialData = nullptr;
	SRVDesc* SRVDesc = nullptr;
	RTVDesc* RTVDesc = nullptr;
	DSVDesc* DSVDesc = nullptr;
	UAVDesc* UAVDesc = nullptr;
};

class Texture {
	friend class TextureManager;
public:
	const std::wstring& GetName() {
		return m_Name;
	}
	size_t GetID() {
		return m_ID;
	}

	bool HasSRV() const {
		return m_ShaderResourceView != nullptr;
	}
	bool HasRTV() const {
		return m_RenderTargetView != nullptr;
	}
	bool HasDSV() const {
		return m_DepthStencilView != nullptr;
	}

	TextureResource* GetTextureResource() const {
		assert( m_TextureResource );
		return m_TextureResource;
	}
	ShaderResourceView* GetSRV() const {
		assert( m_ShaderResourceView );
		return m_ShaderResourceView;
	}
	RenderTargetView* GetRTV() const {
		assert( m_RenderTargetView );
		return m_RenderTargetView;
	}
	DepthStencilView* GetDSV() const {
		assert( m_DepthStencilView );
		return m_DepthStencilView;
	}
	UnorderedAccessView* GetUAV() const {
		assert( m_UnordedAccessView );
		return m_UnordedAccessView;
	}
	uint32_t GetTextureWidth() const {
		return m_Dimension.x;
	}
	uint32_t GetTextureHeight() const {
		return m_Dimension.y;
	}
	uint32_t GetTextureDepth() const {
		return m_Dimension.z;
	}
	void ReleaseResources();
	void ResizeResources( uint32_t width, uint32_t height = 0, uint32_t depth = 0);
	void UpdateResources( TextureResource** texRes, ShaderResourceView** srv, RenderTargetView** rtv, DepthStencilView** dsv, const uint3& newSize );
	void UpdateData( void* data, uint32_t rowPitch, uint32_t depthPitch );

	void BindSRV( uint8_t slot, ShaderType shaderType ) const;

	static Texture* Create( TextureDesc desc );
	static Texture* Create( const std::wstring& uniqueName, TextureResource* texRes, TextureType texType, ShaderResourceView* srv = nullptr,
							RenderTargetView* rtv = nullptr, DepthStencilView* dsv = nullptr );
	static Texture* CreateFromFile( const std::wstring& fileName, const std::wstring& uniqueName = L"" );
	static Texture* Get( const std::wstring& name );
	static Texture* Get( size_t id );

	static const std::wstring& GetTexturePath();
private:
	Texture( const std::wstring& uniqueName, size_t id, TextureType type, uint3 dim, TextureResource* texResource, 
			 ShaderResourceView* srv = nullptr, RenderTargetView* rtv = nullptr, DepthStencilView* dsv = nullptr,
			 UnorderedAccessView* uav = nullptr );
	virtual ~Texture();

	std::wstring m_Name;
	size_t m_ID;
	TextureType m_TextureType;
	bool m_IsLoadedFromFile = false;
	uint3 m_Dimension;

	TextureResource* m_TextureResource = nullptr;
	ShaderResourceView* m_ShaderResourceView = nullptr;
	RenderTargetView* m_RenderTargetView = nullptr;
	DepthStencilView* m_DepthStencilView = nullptr;
	UnorderedAccessView* m_UnordedAccessView = nullptr;

	static TextureManager* s_TextureManager;
};

class TextureManager {
public:
	static TextureManager& Init();

	Texture* GetTexture( std::wstring uniqueName );
	Texture* GetTexture( size_t id );

	Texture* CreateTexture( TextureDesc desc );
	Texture* CreateTexture( const std::wstring& uniqueName, TextureResource* texRes, TextureType texType, ShaderResourceView* srv = nullptr, 
							RenderTargetView* rtv = nullptr, DepthStencilView* dsv = nullptr );
	Texture* CreateTextureFromFile( const std::wstring& fileName, const std::wstring& uniqueName = L"" );

	Texture* GetDummyTexture();
	void CreateDummyTexture();

	const std::wstring& GetTexturePath() {
		return m_TexturePath;
	}
private:
	TextureManager();
	virtual ~TextureManager();


	std::vector<Texture*> m_Textures;
	std::unordered_map<std::wstring, size_t> m_TextureNameToID;
	Texture* m_DummyTexture = nullptr;

	const std::wstring m_TexturePath = L"Assets\\Textures\\";
};

namespace std {
	template<>
	class hash<TextureDesc> {
	public:
		size_t operator() ( const TextureDesc& desc );
	};
}