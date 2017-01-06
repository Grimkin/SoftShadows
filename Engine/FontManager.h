#pragma once
#include <unordered_map>
#include <vector>

#include "ft2build.h"
#include FT_FREETYPE_H

#include "Types.h"
#include "GUI.h"
class Texture;
class GUIText;

struct GlyphData {
	float2 DeltaPos;
	float2 TexSize;
	float TexPosX;
	float Advance;
};

class Font {
	friend class FontManager;
public:
	Font() = default;
	Font( const std::wstring& fontName, float fontSize, Texture* fontTexture, uint32_t texWidth, uint32_t texHeight, const std::vector<GlyphData>& glyphData );
	~Font();

	Font& operator=( const Font& font ) = default;

	void Render();
	void AddText( const GUIText& guiText );
	void DeleteText( const GUIText& guiText );

	std::vector<GUIVertex> GetFontVertices( const GUIText& text );
	void OnWindowResize( uint32_t width, uint32_t height );

	const std::wstring& GetName() const {
		return m_FontName;
	}
	float GetSize() const {
		return m_Size;
	}
	const Texture* GetTexture() const {
		return m_FontTexture;
	}
	uint32_t GetTextureWidth() const {
		return m_TextureWidth;
	}
	uint32_t GetTextureHeight() const {
		return m_TextureHeight;
	}
	const std::vector<GlyphData> GetGlyphData() const {
		return m_GlyphData;
	}
	Buffer* GetVertexBuffer() const {
		return m_VertexBuffer;
	}
	const std::vector<GUIVertex> GetBufferData() const {
		return m_BufferData;
	}
	const std::vector<const GUIText*> GetAssignedTexts() const {
		return m_AssignedTexts;
	}
	size_t GetID() const;
private:
	void ReloadData();
	void UpdateData( const std::vector<GUIVertex>& vertices );

	std::wstring m_FontName;
	float m_Size;
	Texture* m_FontTexture;
	uint32_t m_TextureWidth;
	uint32_t m_TextureHeight;
	std::vector<GlyphData> m_GlyphData;
	Buffer* m_VertexBuffer;
	std::vector<GUIVertex> m_BufferData;
	std::vector<const GUIText*> m_AssignedTexts;

	bool m_IsBufferValid = false;
	bool m_IsDataValid = false;

	const uint32_t m_MaxLetters = 200;

	static FontManager* s_FontManager;
};

class FontManager {
public:
	static FontManager& Init();

	bool LoadFont( const std::wstring& font, float fontSize );
	void AddText( const GUIText& guiText );
	void DeleteText( const GUIText& guiText );

	void Draw();
	void OnWindowResize( uint32_t width, uint32_t height );

	void ReloadText( const GUIText& text );
	std::vector<GUIVertex> GetFontVertices( const std::wstring& font, float fontSize, const GUIText& guiText );

	FT_Face GetFace( const std::wstring& font );
	uint2 GetGlyphAtlasSize( FT_Face face, uint32_t fontSize );
	void GetGlyphTexData( FT_Face, uint32_t fontSize, uint2 texSize, char* data, std::vector<GlyphData>& glyphData );
private:
	FontManager();
	virtual ~FontManager();

	const std::wstring m_FontPath = L"Assets\\Fonts\\";
	const size_t m_MaxLetters = 100;

	std::unordered_map<size_t, Font> m_LoadedFonts;
	std::unordered_map<const GUIText*, Font*> m_GUITextToFont;

	FT_Library m_FTLibrary;

	uint32_t m_UsedWinHeight;
};

size_t FontID( std::wstring font, float fontSize );

