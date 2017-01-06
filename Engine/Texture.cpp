#include "Texture.h"

#include "Game.h"
#include "Logger.h"
#include "Makros.h"
#include "RenderBackend.h"
#include "FileLoader.h"

TextureManager* Texture::s_TextureManager = nullptr;

Texture::Texture( const std::wstring& uniqueName, size_t id, TextureType type, uint3 dim, TextureResource* texResource, 
				  ShaderResourceView* srv, RenderTargetView* rtv, DepthStencilView* dsv, UnorderedAccessView* uav )
	: m_Name( uniqueName )
	, m_ID( id )
	, m_TextureType( type )
	, m_TextureResource( texResource )
	, m_ShaderResourceView( srv )
	, m_RenderTargetView( rtv )
	, m_DepthStencilView( dsv )
	, m_UnordedAccessView( uav )
	, m_Dimension( dim ) {
}


Texture::~Texture() {
	ReleaseResources();
}

void Texture::ReleaseResources() {
	SRelease( m_TextureResource );
	SRelease( m_ShaderResourceView );
	SRelease( m_RenderTargetView );
	SRelease( m_DepthStencilView );
	SRelease( m_UnordedAccessView );
}

void Texture::ResizeResources( uint32_t width, uint32_t height, uint32_t depth ) {
	if( m_IsLoadedFromFile ) {
		Game::GetLogger().Log( L"Texture", L"Attempt to resize loaded texture \"" + m_Name + L"\"." );
		return;
	}
	if( m_Dimension.x == width && m_Dimension.y == height && m_Dimension.z == depth )
		return;

	m_Dimension = { width, height, depth };

	TextureDesc desc;
	desc.UniqueName = m_Name;
	desc.TextureType = m_TextureType;

	RenderBackend* renderBackend = &Game::GetRenderBackend();

	DSVDesc dsvDesc;
	RTVDesc rtvDesc;
	SRVDesc srvDesc;
	UAVDesc uavDesc;

	Texture1DDesc tDesc1;
	Texture2DDesc tDesc2;
	Texture3DDesc tDesc3;

	switch( m_TextureType ) {
		case TextureType::Texture1D:
		case TextureType::Texture1DArray:
		{
			renderBackend->GetTextureDesc( m_TextureResource, &tDesc1 );
			desc.Texture1DDesc = &tDesc1;
			desc.Texture1DDesc->Width = width;
			break;
		}
		case TextureType::Texture2D:
		case TextureType::Texture2DArray:
		case TextureType::TextureCube:
		{
			renderBackend->GetTextureDesc( m_TextureResource, &tDesc2 );
			desc.Texture2DDesc = &tDesc2;
			desc.Texture2DDesc->Width = width;
			desc.Texture2DDesc->Height = height;
			break;
		}
		case TextureType::Texture3D:
		{
			renderBackend->GetTextureDesc( m_TextureResource, &tDesc3 );
			desc.Texture3DDesc = &tDesc3;
			desc.Texture3DDesc->Width = width;
			desc.Texture3DDesc->Height = height;
			desc.Texture3DDesc->Depth = depth;
			break;
		}
		default:
			break;
	}

	if( m_ShaderResourceView ) {
		renderBackend->GetSRVDesc( m_ShaderResourceView, srvDesc );
		desc.SRVDesc = &srvDesc;
	}
	else
		desc.SRVDesc = nullptr;

	if( m_RenderTargetView ) {
		renderBackend->GetRTVDesc( m_RenderTargetView, rtvDesc );
		desc.RTVDesc = &rtvDesc;
	}
	else
		desc.RTVDesc = nullptr;

	if( m_DepthStencilView ) {
		renderBackend->GetDSVDesc( m_DepthStencilView, dsvDesc );
		desc.DSVDesc = &dsvDesc;
	}
	else
		desc.DSVDesc = nullptr;

	if( m_UnordedAccessView ) {
		renderBackend->GetUAVDesc( m_UnordedAccessView, uavDesc );
		desc.UAVDesc = &uavDesc;
	}
	else
		desc.UAVDesc = nullptr;

	ReleaseResources();

	Game::GetRenderBackend().CreateTexture( desc, &m_TextureResource, &m_ShaderResourceView, &m_RenderTargetView, &m_DepthStencilView, &m_UnordedAccessView );
}

void Texture::UpdateResources( TextureResource** texRes, ShaderResourceView** srv, RenderTargetView** rtv, DepthStencilView** dsv, const uint3& newSize ) {
	m_Dimension = newSize;
	if( texRes ) {
		SRelease( m_TextureResource );
		m_TextureResource = *texRes;
	}
	if( srv ) {
		SRelease( m_ShaderResourceView );
		m_ShaderResourceView = *srv;
	}
	if( rtv ) {
		SRelease( m_RenderTargetView );
		m_RenderTargetView = *rtv;
	}
	if( dsv ) {
		SRelease( m_DepthStencilView );
		m_DepthStencilView = *dsv;
	}
}

void Texture::UpdateData( void * data, uint32_t rowPitch, uint32_t depthPitch ) {
	Game::GetRenderBackend().UpdateSubresource( m_TextureResource, 0, nullptr, data, rowPitch, depthPitch );
}

void Texture::BindSRV( uint8_t slot, ShaderType shaderType ) const {
	if( !m_ShaderResourceView ) {
		Game::GetLogger().Log( L"Texture", L"Attempt to bind Texture with no SRV" );
		return;
	}

	switch( shaderType ) {
		case ShaderType::VertexShader:
			Game::GetRenderBackend().SetSRVsVS( slot, { m_ShaderResourceView } );
			break;
		case ShaderType::PixelShader:
			Game::GetRenderBackend().SetSRVsPS( slot, { m_ShaderResourceView } );
			break;
		case ShaderType::GeometryShader:
			Game::GetRenderBackend().SetSRVsGS( slot, { m_ShaderResourceView } );
			break;
		case ShaderType::HullShader:
			Game::GetRenderBackend().SetSRVsHS( slot, { m_ShaderResourceView } );
			break;
		case ShaderType::ComputeShader:
			Game::GetRenderBackend().SetSRVsCS( slot, { m_ShaderResourceView } );
			break;
		case ShaderType::DomainShader:
			Game::GetRenderBackend().SetSRVsDS( slot, { m_ShaderResourceView } );
			break;
		default:
			break;
	}
}

Texture * Texture::Create( TextureDesc desc ) {
	return s_TextureManager->CreateTexture( desc );
}

Texture * Texture::Create( const std::wstring & uniqueName, TextureResource* texRes, TextureType texType, ShaderResourceView* srv, RenderTargetView* rtv, DepthStencilView* dsv ) {
	return s_TextureManager->CreateTexture( uniqueName, texRes, texType, srv, rtv, dsv );
}

Texture * Texture::CreateFromFile( const std::wstring& fileName, const std::wstring& uniqueName ) {
	return s_TextureManager->CreateTextureFromFile( fileName, uniqueName );
}

Texture * Texture::Get( const std::wstring & name ) {
	return s_TextureManager->GetTexture( name );
}

Texture * Texture::Get( size_t id ) {
	return s_TextureManager->GetTexture( id );
}

const std::wstring & Texture::GetTexturePath() {
	return s_TextureManager->GetTexturePath();
}

TextureManager& TextureManager::Init() {
	static TextureManager textureManager;
	return textureManager;
}

Texture* TextureManager::CreateTexture( TextureDesc desc ) {
	// check for double unique name existance
	auto nameId = m_TextureNameToID.find( desc.UniqueName );
	if( nameId != m_TextureNameToID.end() ) {
		Game::GetLogger().Log( L"Texture", L"Texture with name \"" + desc.UniqueName + L"\" already exists. Choose different name or use GetTexture() to use the texture.\nOld Texture is returned" );
		return m_Textures[nameId->second];
	}

	// create new texture
	size_t newId = m_Textures.size();

	ShaderResourceView* srv;
	RenderTargetView* rtv;
	DepthStencilView* dsv;
	TextureResource* tex;
	UnorderedAccessView* uav;
	bool success = Game::GetRenderBackend().CreateTexture( desc, &tex, &srv, &rtv, &dsv, &uav );	

	if( !success ) {
		return nullptr;
	}

	uint3 dim = Game::GetRenderBackend().GetTextureDim( tex );

	Texture* newTexture = new Texture( desc.UniqueName, newId, desc.TextureType, dim, tex, srv, rtv, dsv, uav );
	m_TextureNameToID[desc.UniqueName] = newId;
	m_Textures.push_back( newTexture );
	return newTexture;
}

Texture * TextureManager::CreateTexture( const std::wstring& uniqueName, TextureResource* texRes, TextureType texType, ShaderResourceView* srv, RenderTargetView* rtv, DepthStencilView* dsv ) {
	size_t newId = m_Textures.size();
	Texture* newTex;
	uint3 dim;
	if( texRes )
		dim = Game::GetRenderBackend().GetTextureDim( texRes );
	if( m_TextureNameToID.count( uniqueName ) > 0 ) {
		Game::GetLogger().Log( L"Texture", L"Texture with name \"" + uniqueName + L"\" already exists. New Texture is created but can't be access through the name." );
		std::wstring name = L"Texture_" + newId;
		// in case the texture name is already used by the user
		while( m_TextureNameToID.count( name ) > 0 ) {
			name += L"1";
		}

		newTex = new Texture( name, newId, texType, dim, texRes, srv, rtv, dsv );
		m_TextureNameToID[name] = newId;
	}
	else {
		newTex = new Texture( uniqueName, newId, texType, dim, texRes, srv, rtv, dsv );
		m_TextureNameToID[uniqueName] = newId;
	}

	m_Textures.push_back( newTex );

	return newTex;
}

Texture* TextureManager::CreateTextureFromFile( const std::wstring& fileName, const std::wstring& uniqueName ) {
	if( m_TextureNameToID.count( fileName ) > 0 ) {
		Texture* existingTex = m_Textures[m_TextureNameToID[fileName]];
		if( uniqueName == existingTex->GetName() ) {
			Game::GetLogger().Log( L"Texture", L"File already loaded with same unique name." );
			return existingTex;
		}
		else {
			if( m_TextureNameToID.count( uniqueName ) > 0 ) {
				Game::GetLogger().Log( L"Texture", L"File already loaded with different unique name \"" +
									   existingTex->GetName() + L"\". \nRequested unique name \"" + uniqueName + L"\" is already taken" );
				return existingTex;
			}
			Game::GetLogger().Log( L"Texture", L"File already loaded with different unique name \"" +
								   existingTex->GetName() + L"\". \nNew unique name \"" + uniqueName + L"\" is added" );
			m_TextureNameToID[uniqueName] = existingTex->GetID();
			return existingTex;
		}
	}
	std::wstring usedName = uniqueName;
	if( m_TextureNameToID.count( uniqueName ) > 0 ) {
		Game::GetLogger().Log( L"Texture", L"Unique name is alread taken. Choose another name please. Texture is loaded and stored with file name \"" + fileName + L"\" as unique name." );
		usedName = L"";
	}


	TextureResource* tex = nullptr;
	ShaderResourceView* srv = nullptr;

	bool success = Game::GetFileLoader().LoadTexFile( m_TexturePath + fileName, tex, srv );

	if( !success ) {
		Game::GetLogger().Log( L"Texture", L"Loading of Texture with file name \"" + fileName + L"\" and unique name \"" + uniqueName + L"\" failed. Returning dummy texture" );
		return GetDummyTexture();
	}
	size_t id = m_Textures.size();
	Texture* newTexture;
	uint3 dim = Game::GetRenderBackend().GetTextureDim( tex );
	if( !usedName.empty() ) {
		newTexture = new Texture( usedName, id, TextureType::Texture2D, dim, tex, srv );
		m_TextureNameToID[usedName] = id;
	}
	else {
		newTexture = new Texture( fileName, id, TextureType::Texture2D, dim, tex, srv );
	}
	m_Textures.push_back( newTexture );
	m_TextureNameToID[fileName] = id;
	newTexture->m_IsLoadedFromFile = true;
	return newTexture;
}

Texture * TextureManager::GetDummyTexture() {
	if( !m_DummyTexture )
		CreateDummyTexture();
	return m_DummyTexture;
}

Texture * TextureManager::GetTexture( std::wstring uniqueName ) {
	auto nameId = m_TextureNameToID.find( uniqueName );
	if( nameId != m_TextureNameToID.end() ) {
		return m_Textures[nameId->second];
	}
	return nullptr;
}

Texture * TextureManager::GetTexture( size_t id ) {
	if( id < m_Textures.size() ) {
		return m_Textures[id];
	}
	return nullptr;
}

TextureManager::TextureManager() {
	Game::SetTextureManager( *this );
	Texture::s_TextureManager = this;
}

TextureManager::~TextureManager() {
	SDeleteVec<Texture>( m_Textures, []( Texture* ptr ) { delete ptr; } );
}

void TextureManager::CreateDummyTexture() {
	if( m_DummyTexture )
		return;
	TextureDesc desc;
	SubresourceData initialData;
	float4 data = { 1.0f, 1.0f, 1.0f, 1.0f };
	initialData.pSysMem = &data;
	initialData.SysMemPitch = sizeof( float4 ) * 4;
	initialData.SysMemSlicePitch = 0;
	desc.InitialData = &initialData;
	desc.UniqueName = L"DummyTexture";
	desc.TextureType = TextureType::Texture2D;
	Texture2DDesc tex2dDesc;
	tex2dDesc.Format = Format::R32G32B32A32_Float;
	tex2dDesc.Width = 1;
	tex2dDesc.Height = 1;
	tex2dDesc.BindFlags = BindFlag::ShaderResource;
	desc.Texture2DDesc = &tex2dDesc;
	
	m_DummyTexture = CreateTexture( desc );
}

size_t std::hash<TextureDesc>::operator()( const TextureDesc& desc ) {
	size_t out;
	out = static_cast<size_t>( desc.TextureType );
	if( desc.Texture3DDesc ) {
		out ^= desc.Texture3DDesc->Width << 4;
		out ^= desc.Texture3DDesc->Height << 8;
		out ^= desc.Texture3DDesc->Depth << 12;
	}
	if( desc.DSVDesc )
		out ^= std::hash<DSVDesc>()( *desc.DSVDesc );
	if( desc.RTVDesc )
		out ^= std::hash<RTVDesc>() ( *desc.RTVDesc );
	if( desc.SRVDesc )
		out ^= std::hash<SRVDesc>()( *desc.SRVDesc );
	if( desc.InitialData )
		out ^= reinterpret_cast<const size_t*>( desc.InitialData->pSysMem )[0];
	return out;
}