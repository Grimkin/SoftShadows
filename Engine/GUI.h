#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#include "Types.h"

class RenderPass;
class FontManager;
class Texture;
class Shader;
class MouseCollider;

enum class Alignment {
	Left,
	Right,
	Center
};

enum class SizeType {
	WinWidthDependant,
	WinHeightDependent,
	WinDependent,
	TextureDependant
};

_declspec( align( 16 ) )
struct GUIVertex {
	float4 Color;
	float2 Position;
	float2 TexSize = { 1.0f, 1.0f };
	float2 Size;
	float TexPosX = 0.0f;
};

class GUITexture {
	friend class GUI;
public:
	static GUITexture* Create( const Texture& texture, const float2& position, uint8_t layer, float size, const float2& scale = { 1.0f,1.0f }, SizeType sizeType = SizeType::WinHeightDependent, const Color& color = { 1.0f,1.0f,1.0f,1.0f } );
	void Delete();

	void AddMouseCollider();
	void AddMouseCollider( const float2 relativeSize );
	bool HasCollider() {
		return m_Collider != nullptr;
	}

	MouseCollider* GetMouseCollider() {
		assert( m_Collider );
		return m_Collider;
	}

	void BindTexture() const;

	void SetTexture( const Texture& texture );
	void SetPosition( const float2& position );
	void SetLayer( uint8_t layer );
	void SetSize( float size );
	void SetScale( const float2& scale );
	void SetSizeType( SizeType );
	void SetColor( const Color& color );
	void SetVisible( bool visible ) {
		m_IsVisible = visible;
	}

	const Texture* GetTexture() const {
		return m_Texture;
	}
	const float2& GetPosition() const {
		return m_Position;
	}
	uint8_t GetLayer() const {
		return m_Layer;
	}
	const float2& GetScale() const {
		return m_Scale;
	}
	const SizeType GetSizeType() const {
		return m_SizeType;
	}
	const Color& GetColor() const {
		return m_Color;
	}
	float GetSize() const {
		return m_Size;
	}
	bool IsVisible() const {
		return m_IsVisible;
	}

	const float2 GetRenderSize() const;
	void UpdateColliderSize();
private:
	GUITexture( const Texture& texture, const float2& position, uint8_t layer, float size, const float2& scale = { 1.0f,1.0f }, SizeType sizeType = SizeType::WinHeightDependent, const Color& color = { 1.0f,1.0f,1.0f,1.0f } );
	virtual ~GUITexture();

	const Texture* m_Texture;
	float2 m_Position;
	uint8_t m_Layer;
	float m_Size;
	float2 m_Scale;
	SizeType m_SizeType;
	Color m_Color;

	bool m_IsVisible = true;

	MouseCollider* m_Collider;
	float2 m_RelativeColliderSize;

	static GUI* s_GUI;
};

class GUIText {
	friend class GUI;
public:
	void SetText( const std::wstring& text );
	void SetPosition( const float2& position );
	void SetColor( const Color& color );
	void SetSize( float size );
	void SetAlignment( Alignment alignment );

	const std::wstring& GetText() const {
		return m_Text;
	}
	const float2& GetPosition() const {
		return m_Position;
	}
	const Color& GetColor() const {
		return m_Color;
	}
	float GetSize() const {
		return m_FontSize;
	}
	const std::wstring& GetFont() const {
		return m_Font;
	}
	Alignment GetAlignment() const {
		return m_Alignment;
	}

	static GUIText* Create( const std::wstring& text, const float2& position, uint8_t layer, float fontSize = 10.0f, Alignment alignment = Alignment::Left,
							const Color& color = { 0.0f, 0.0f, 0.0f, 1.0f }, const std::wstring& font = L"Arial.ttf" );

	void Delete();
private:
	GUIText( size_t id, size_t bufferPos, const std::wstring& text, const float2& position, uint8_t layer, float fontSize = 10.0f, Alignment alignment = Alignment::Left,
			 const Color& color = { 0.0f, 0.0f, 0.0f, 1.0f }, const std::wstring& font = L"Arial.ttf" );
	virtual ~GUIText();

	size_t m_ID;
	std::wstring m_Text;
	float2 m_Position;
	uint8_t m_Layer;
	Color m_Color;
	std::wstring m_Font;
	float m_FontSize;
	Alignment m_Alignment;

	size_t m_BufferPosition;

	static GUI* s_GUI;
};

class GUI {
public:
	static GUI& Init();

	GUIText* CreateText( const std::wstring& text, const float2& position, uint8_t layer, float fontSize = 10.0f, Alignment alignment = Alignment::Left,
						 const Color& color = { 0.0f, 0.0f, 0.0f, 1.0f }, const std::wstring& font = L"Arial.ttf" );
	GUITexture* CreateElement( const Texture& texture, const float2& position, uint8_t layer, float size, const float2& scale = { 1.0f,1.0f }, SizeType sizeType = SizeType::WinHeightDependent, const Color& color = { 1.0f,1.0f,1.0f,1.0f } );

	void Render();

	void ReloadText( const GUIText& guiText );
	void DeleteText( const GUIText& guiText );

	void DeleteTexture( const GUITexture& guiElement );
	void InvalidateElementBuffer() {
		m_IsBufferValid = false;
	}
	void OnResize( uint32_t width, uint32_t height );
private:
	GUI();
	virtual ~GUI();

	void DrawElements();
	void ReloadElementBuffer();

	size_t m_CurrentID = 0;

	RenderPass* m_RenderPass;
	std::vector<GUIText*> m_GUITexts;
	std::vector<GUITexture*> m_GUITextures;
	Buffer* m_ElementBuffer;
	FontManager* m_FontManager;
	Shader* m_FontPixelShader;
	SamplerState* m_SamplerState;

	bool m_IsBufferValid = true;

	uint32_t m_MaxGUIElements = 100;
};

