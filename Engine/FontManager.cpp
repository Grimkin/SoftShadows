#include "FontManager.h"

#include "Game.h"
#include "Logger.h"
#include "Makros.h"
#include "Texture.h"
#include "GUI.h"
#include "Math.h"
#include "Window.h"
#include "RenderBackend.h"
#include "Material.h"

FontManager* Font::s_FontManager = nullptr;

FontManager & FontManager::Init() {
	static FontManager fontManager;
	return fontManager;
}

bool FontManager::LoadFont( const std::wstring & font, float fontSize ) {
	if( m_LoadedFonts.count( FontID( font, fontSize ) ) )
		return true;

	FT_Face face = GetFace( font );
	if( !face )
		return false;

	// use the font size to make it similar to pt size for fullhd
	uint32_t size = static_cast<FT_UInt>( fontSize / 720.0f * Game::GetWindow().GetHeight() );
	uint2 atlasSize = GetGlyphAtlasSize( face, size );
	uint32_t width = atlasSize.x;
	uint32_t height = atlasSize.y;

	char* tempData = new char[width * height];

	std::vector<GlyphData> glyphData;
	GetGlyphTexData( face, size, atlasSize, tempData, glyphData );

	TextureDesc tDesc;
	tDesc.UniqueName = font + FloatToString( fontSize, 1 );
	tDesc.TextureType = TextureType::Texture2D;
	SubresourceData srData;
	srData.pSysMem = tempData;
	srData.SysMemPitch = width;
	tDesc.InitialData = &srData;
	Texture2DDesc tex2dDesc;
	tDesc.Texture2DDesc = &tex2dDesc;
	tex2dDesc.Width = width;
	tex2dDesc.Height = height;
	tex2dDesc.BindFlags = BindFlag::ShaderResource;
	tex2dDesc.Format = Format::R8_Unorm;
	tex2dDesc.Usage = Usage::Default;

	Texture* fontTex = Game::GetTextureManager().CreateTexture( tDesc );
	delete[] tempData;

	FT_Done_Face( face );

	if( !fontTex ) {
		Game::GetLogger().Log( L"Font", L"Creating Texture for font " + font + L" failed." );
		return false;
	}

	m_LoadedFonts.emplace( std::pair<size_t, Font>{ FontID( font, fontSize ), { font, fontSize, fontTex, width, height, glyphData } } );
	return true;
}

void FontManager::AddText( const GUIText& guiText ) {
	size_t fontID = FontID( guiText.GetFont(), guiText.GetSize() );

	if( m_LoadedFonts.count( fontID ) == 0 ) {
		if( !LoadFont( guiText.GetFont(), guiText.GetSize() ) )
			return;
	}
	if( m_GUITextToFont.count( &guiText ) > 0 ) {
		if( m_GUITextToFont[&guiText]->GetID() == fontID ) {
			m_LoadedFonts[fontID].ReloadData();
			return;
		}
		else
			m_GUITextToFont[&guiText]->DeleteText( guiText );
	}

	Font& font = m_LoadedFonts[fontID];
	m_GUITextToFont[&guiText] = &font;

	font.AddText( guiText );
}

void FontManager::DeleteText( const GUIText & guiText ) {
	size_t fontID = FontID( guiText.GetFont(), guiText.GetSize() );

	if( m_LoadedFonts.count( fontID ) > 0 )
		m_LoadedFonts[fontID].DeleteText( guiText );
}

void FontManager::Draw() {
	Game::GetMaterialManager().SetCurrentMaterialID( -1 );
	for( auto& fontIT : m_LoadedFonts ) {
		fontIT.second.Render();
	}
}

void FontManager::OnWindowResize( uint32_t width, uint32_t height ) {
	for( auto& fontIT : m_LoadedFonts ) {
		fontIT.second.OnWindowResize( width, height );
	}
}


void FontManager::ReloadText( const GUIText & guiText ) {
	AddText( guiText );
}

std::vector<GUIVertex> FontManager::GetFontVertices( const std::wstring& fontName, float fontSize, const GUIText& guiText ) {
	if( m_LoadedFonts.count( FontID( fontName, fontSize ) ) == 0 ) {
		if( !LoadFont( fontName, fontSize ) )
			return std::vector<GUIVertex>();
	}

	Font& font = m_LoadedFonts[FontID( fontName, fontSize )];

	return font.GetFontVertices( guiText );
}

FT_Face FontManager::GetFace( const std::wstring & font ) {
	FT_Face face;
	FT_Error error;
	std::string path = ws2s( m_FontPath + font );
	error = FT_New_Face( m_FTLibrary, path.c_str(), 0, &face );
	if( error ) {
		Game::GetLogger().Log( L"Font", L"Loading of font " + font + L" failed." );
		return nullptr;
	}
	return face;
}

FontManager::FontManager() {
	FT_Error error = FT_Init_FreeType( &m_FTLibrary );
	if( error ) {
		Game::GetLogger().FatalError( L"Initializing FreeType failed" );
	}
	Game::SetFontManager( *this );
	Font::s_FontManager = this;
	m_UsedWinHeight = Game::GetWindow().GetHeight();
}


FontManager::~FontManager() {
	FT_Done_FreeType( m_FTLibrary );
}

uint2 FontManager::GetGlyphAtlasSize( FT_Face face, uint32_t fontSize ) {
	FT_Error error;
	// use the font size to make it similar to pt size for fullhd
	FT_Set_Pixel_Sizes( face, 0, static_cast<FT_UInt>( fontSize / 720.0f * Game::GetWindow().GetHeight() ) );
	uint32_t width = 0;
	uint32_t height = 0;
	for( uint32_t i = 32; i < 128; ++i ) {
		error = FT_Load_Char( face, i, FT_LOAD_RENDER );
		if( error ) {
			FT_Done_Face( face );
			Game::GetLogger().Log( L"Font", L"Loading of charactor " + std::to_wstring( char( i ) ) + L" failed." );
			return uint2();
		}
		FT_GlyphSlot g = face->glyph;

		width += g->bitmap.width + 2;
		height = std::max( height, g->bitmap.rows + 1 );
	}
	return{ width, height };
}

void FontManager::GetGlyphTexData( FT_Face face, uint32_t fontSize, uint2 texSize, char * data, std::vector<GlyphData>& glyphData ) {
	uint32_t width = texSize.x;
	uint32_t height = texSize.y;

	memset( data, 0, width * height );
	uint32_t x = 0;
	for( int i = 32; i < 128; i++ ) {
		if( FT_Load_Char( face, i, FT_LOAD_RENDER ) )
			continue;

		FT_GlyphSlot g = face->glyph;

		Update2DArray( data, { width, height }, g->bitmap.buffer, { x, 0 }, { g->bitmap.width, g->bitmap.rows } );

		GlyphData gData;
		gData.TexPosX = static_cast<float>( x ) / static_cast<float>( width );
		gData.Advance = g->advance.x / 64.0f;
		gData.TexSize.x = static_cast<float>( g->bitmap.width + 1 );
		gData.TexSize.y = static_cast<float>( g->bitmap.rows + 1 );
		gData.DeltaPos.x = static_cast<float>( g->bitmap_left );
		gData.DeltaPos.y = static_cast<float>( g->bitmap_top );
		glyphData.push_back( gData );

		x += g->bitmap.width + 2;
	}
}

size_t FontID( std::wstring font, float fontSize ) {
	std::wstring fontId = font + FloatToString( fontSize, 1 );
	return std::hash<std::wstring>()( fontId );
}


Font::Font( const std::wstring & fontName, float fontSize, Texture * fontTexture, uint32_t texWidth, uint32_t texHeight, const std::vector<GlyphData>& glyphData )
	: m_FontName( fontName )
	, m_Size( fontSize )
	, m_FontTexture( fontTexture )
	, m_TextureWidth( texWidth )
	, m_TextureHeight( texHeight )
	, m_GlyphData( glyphData ) {
	BufferDesc bDesc;
	bDesc.BindFlags = BindFlag::VertexBuffer;
	bDesc.Usage = Usage::Dynamic;
	bDesc.ByteWidth = sizeof( GUIVertex ) * m_MaxLetters;
	bDesc.CPUAccessFlags = CPUAccessFlag::Write;
	m_VertexBuffer = Game::GetRenderBackend().CreateBuffer( nullptr, bDesc );
}

Font::~Font() {
}

void Font::Render() {
	if( !m_IsDataValid )
		ReloadData();
	if( !m_IsBufferValid ) {
		Game::GetRenderBackend().UpdateBuffer( m_VertexBuffer, m_BufferData.data(), static_cast<uint32_t>( m_BufferData.size() * sizeof( m_BufferData[0] ) ), 0, MapType::WriteDiscard );
		m_IsBufferValid = true;
	}
	if( m_BufferData.empty() )
		return;

	RenderBackend* renderBackend = &Game::GetRenderBackend();
	renderBackend->SetSRVsPS( 0, { m_FontTexture->GetSRV() } );
	renderBackend->SetVertexBuffer( 0, { m_VertexBuffer }, { sizeof( GUIVertex ) }, { 0 } );
	renderBackend->Draw( static_cast<uint32_t>( m_BufferData.size() ), 0 );
}

void Font::AddText( const GUIText & guiText ) {
	m_AssignedTexts.push_back( &guiText );
	UpdateData( GetFontVertices( guiText ) );
	m_IsDataValid = true;
}

std::vector<GUIVertex> Font::GetFontVertices( const GUIText& guiText ) {
	std::vector<GUIVertex> fontVertices;
	std::string sText = ws2s( guiText.GetText() );
	float2 letterPos = guiText.GetPosition() * 2.0f - float2( 1.0f, 1.0f );
	GUIVertex curVertex;
	curVertex.Color = guiText.GetColor();
	float texWidth_1 = 1.0f / m_TextureWidth;
	float texHeight_1 = 1.0f / m_TextureHeight;
	float winWidth_1 = 2.0f / Game::GetWindow().GetWidth();
	float winHeight_1 = 2.0f / Game::GetWindow().GetHeight();
	for( size_t i = 0; i < sText.size(); ++i ) {
		unsigned char c = sText[i] - 32;
		if( c >= m_GlyphData.size() )
			c = '?' - 32;
		curVertex.Position.x = letterPos.x + m_GlyphData[c].DeltaPos.x * winWidth_1;
		curVertex.Position.y = -letterPos.y + m_GlyphData[c].DeltaPos.y * winHeight_1;
		curVertex.TexPosX = m_GlyphData[c].TexPosX;
		curVertex.TexSize.x = m_GlyphData[c].TexSize.x * texWidth_1;
		curVertex.TexSize.y = m_GlyphData[c].TexSize.y * texHeight_1;
		curVertex.Size.x = m_GlyphData[c].TexSize.x * winWidth_1;
		curVertex.Size.y = m_GlyphData[c].TexSize.y * winHeight_1;
		letterPos.x += m_GlyphData[c].Advance * winWidth_1;
		fontVertices.push_back( curVertex );
	}
	if( guiText.GetAlignment() != Alignment::Left ) {
		float correctionVal = letterPos.x - ( guiText.GetPosition().x * 2.0f - 1.0f );
		if( guiText.GetAlignment() == Alignment::Center )
			correctionVal /= 2.0f;
		for( GUIVertex& vertex : fontVertices ) {
			vertex.Position.x -= correctionVal;
		}
	}

	return fontVertices;
}

void Font::OnWindowResize( uint32_t width, uint32_t height ) {
	FT_Face face = s_FontManager->GetFace( m_FontName );
	if( !face )
		return;
	uint32_t fontSize = static_cast<FT_UInt>( m_Size / 720.0f * height );
	uint2 atlasSize = s_FontManager->GetGlyphAtlasSize( face, fontSize );

	if( m_TextureWidth == atlasSize.x ) {
		ReloadData();
		return;
	}
	m_GlyphData.clear();
	m_BufferData.clear();
	m_IsBufferValid = false;
	m_IsDataValid = false;

	m_TextureWidth = atlasSize.x;
	m_TextureHeight = atlasSize.y;

	m_FontTexture->ResizeResources( atlasSize.x, atlasSize.y );
	char* texData = new char[atlasSize.x * atlasSize.y];

	s_FontManager->GetGlyphTexData( face, fontSize, atlasSize, texData, m_GlyphData );

	m_FontTexture->UpdateData( texData, atlasSize.x, 0 );

	delete[] texData;

	FT_Done_Face( face );
}

inline size_t Font::GetID() const {
	return FontID( m_FontName, m_Size );
}

void Font::ReloadData() {
	m_BufferData.clear();
	for( const GUIText* guiText : m_AssignedTexts ) {
		UpdateData( GetFontVertices( *guiText ) );
	}
	m_IsDataValid = true;
}

void Font::UpdateData( const std::vector<GUIVertex>& vertices ) {
	if( vertices.size() == 0 )
		return;

	m_BufferData.insert( m_BufferData.end(), vertices.begin(), vertices.end() );

	if( m_BufferData.size() > m_MaxLetters )
		m_BufferData.resize( m_MaxLetters );

	m_IsBufferValid = false;
}

void Font::DeleteText( const GUIText & guiText ) {
	for( size_t i = 0; i < m_AssignedTexts.size(); ++i ) {
		if( &guiText == m_AssignedTexts[i] ) {
			m_AssignedTexts[i] = m_AssignedTexts.back();
			m_AssignedTexts.pop_back();
			break;
		}
	}

	m_IsDataValid = false;
	m_IsBufferValid = false;
}
