#include "GUI.h"

#include "Game.h"
#include "RenderBackend.h"
#include "D3DWrapper.h"
#include "RenderPass.h"
#include "Texture.h"
#include "FontManager.h"
#include "Material.h"
#include "Math.h"
#include "Makros.h"
#include "Window.h"
#include "InputManager.h"

GUI* GUIText::s_GUI = nullptr;
GUI* GUITexture::s_GUI = nullptr;

GUI & GUI::Init() {
	static GUI gui;
	return gui;
}

GUIText * GUI::CreateText( const std::wstring & text, const float2& position, uint8_t layer, float fontSize, Alignment alignment, const Color & color, const std::wstring & font ) {
	size_t id = m_CurrentID++;

	m_GUITexts.emplace_back( new GUIText( id, 0, text, position, layer, fontSize, alignment, color, font ) );
	m_FontManager->AddText( *m_GUITexts.back() );

	return m_GUITexts.back();
}

GUITexture * GUI::CreateElement( const Texture & texture, const float2 & position, uint8_t layer, float size, const float2& scale, SizeType sizeType, const Color& color ) {
	m_GUITextures.emplace_back( new GUITexture( texture, position, layer, size, scale, sizeType, color ) );
	return m_GUITextures.back();
}

void GUI::Render() {
	m_RenderPass->Apply( RenderFlag::None );
	Game::GetRenderBackend().SetTopology( PrimitiveTopology::Pointlist );
	Game::GetRenderBackend().SetSamplerPS( 0, { m_SamplerState } );
	DrawElements();

	m_FontPixelShader->SetShader();
	m_FontManager->Draw();
}

void GUI::ReloadText( const GUIText & guiText ) {
	m_FontManager->ReloadText( guiText );
}

void GUI::DeleteText( const GUIText & guiText ) {
	m_FontManager->DeleteText( guiText );
	for( size_t i = 0; i < m_GUITexts.size(); i++ ) {
		if( &guiText == m_GUITexts[i] ) {
			delete m_GUITexts[i];
			m_GUITexts[i] = m_GUITexts.back();
			m_GUITexts.pop_back();
		}
	}
}

void GUI::DeleteTexture( const GUITexture & guiTexture ) {
	for( size_t i = 0; i < m_GUITextures.size(); i++ ) {
		if( &guiTexture == m_GUITextures[i] ) {
			delete m_GUITextures[i];
			m_GUITextures[i] = m_GUITextures.back();
			m_GUITextures.pop_back();
		}
	}
}

void GUI::OnResize( uint32_t width, uint32_t height ) {
	m_FontManager->OnWindowResize( width, height );
	for( GUITexture* texture : m_GUITextures ) {
		texture->UpdateColliderSize();
	}
	InvalidateElementBuffer();
}

GUI::GUI() {
	GUIText::s_GUI = this;
	GUITexture::s_GUI = this;
	Game::SetGUI( *this );

	RenderPassInit rInit;
	rInit.Name = L"GUIPass";
	rInit.VertexShader = Shader::Get( L"vsGUI" );
	rInit.GeometryShader = Shader::Get( L"gsQuad" );
	rInit.PixelShader = Shader::Get( L"psGUIElement" );
	rInit.RenderPassType = RenderPassType::Manual;
	rInit.DepthStencilDesc.DepthEnable = false;
	rInit.BlendDesc.RenderTarget[0].BlendEnable = true;

	m_RenderPass = RenderPass::Create( rInit );

	m_FontPixelShader = Shader::Get( L"psGUIFont" );

	SamplerDesc sDesc;
	m_SamplerState = Game::GetRenderBackend().GetSamplerState( sDesc );

	BufferDesc bDesc;
	bDesc.BindFlags = BindFlag::VertexBuffer;
	bDesc.Usage = Usage::Dynamic;
	bDesc.ByteWidth = sizeof( GUIVertex ) * m_MaxGUIElements;
	bDesc.CPUAccessFlags = CPUAccessFlag::Write;

	m_ElementBuffer = Game::GetRenderBackend().CreateBuffer( nullptr, bDesc );

	m_FontManager = &FontManager::Init();
}


GUI::~GUI() {
	SDeleteVec<GUIText>( m_GUITexts, []( GUIText* text ) { delete text; } );
	SDeleteVec<GUITexture>( m_GUITextures, []( GUITexture* texture ) { delete texture; } );
}

void GUI::DrawElements() {
	if( !m_IsBufferValid )
		ReloadElementBuffer();

	Game::GetRenderBackend().SetVertexBuffer( 0, { m_ElementBuffer }, { sizeof( GUIVertex ) }, { 0 } );
	int i = 0;
	for( GUITexture* guiTexture : m_GUITextures ) {
		if( guiTexture->IsVisible() ) {
			guiTexture->BindTexture();
			Game::GetRenderBackend().Draw( 1, i );
		}
		++i;
	}
}

void GUI::ReloadElementBuffer() {
	std::vector<GUIVertex> vertices;
	for( GUITexture* guiTexture : m_GUITextures ) {
		GUIVertex newVertex;
		newVertex.Position = 2.0f * guiTexture->GetPosition() - 1.0f;
		newVertex.Position.y = -newVertex.Position.y;
		newVertex.Color = guiTexture->GetColor();
		newVertex.Size = guiTexture->GetRenderSize();
		newVertex.Position.x -= 0.5f * newVertex.Size.x;
		newVertex.Position.y += 0.5f * newVertex.Size.y;
		vertices.emplace_back( newVertex );
	}
	Game::GetRenderBackend().UpdateBuffer( m_ElementBuffer, vertices.data(), static_cast<uint32_t>( vertices.size() * sizeof( GUIVertex ) ), 0, MapType::WriteDiscard );
	m_IsBufferValid = true;
}

void GUIText::SetText( const std::wstring & text ) {
	if( text == m_Text )
		return;

	m_Text = text;
	s_GUI->ReloadText( *this );
}

void GUIText::SetPosition( const float2 & position ) {
	if( position == m_Position )
		return;

	m_Position = position;
	s_GUI->ReloadText( *this );
}

void GUIText::SetColor( const Color & color ) {
	if( m_Color == color )
		return;

	m_Color = color;
	s_GUI->ReloadText( *this );
}

void GUIText::SetSize( float size ) {
	if( m_FontSize == size )
		return;

	m_FontSize = size;
	s_GUI->ReloadText( *this );
}

void GUIText::SetAlignment( Alignment alignment ) {
	if( m_Alignment == alignment )
		return;

	m_Alignment = alignment;
	s_GUI->ReloadText( *this );
}

GUIText * GUIText::Create( const std::wstring & text, const float2& position, uint8_t layer, float fontSize, Alignment alignment, const Color & color, const std::wstring & font ) {
	return s_GUI->CreateText( text, position, layer, fontSize, alignment, color, font );
}

void GUIText::Delete() {
	s_GUI->DeleteText( *this );
}

GUIText::GUIText( size_t id, size_t bufferPos, const std::wstring & text, const float2& position, uint8_t layer, float fontSize, Alignment alignment, const Color & color, const std::wstring & font )
	: m_ID( id )
	, m_BufferPosition( bufferPos )
	, m_Text( text )
	, m_Position( position )
	, m_Layer( layer )
	, m_FontSize( fontSize )
	, m_Color( color )
	, m_Font( font )
	, m_Alignment( alignment ) {
}

GUIText::~GUIText() {
}

GUITexture* GUITexture::Create( const Texture & texture, const float2 & position, uint8_t layer, float size, const float2& scale, SizeType sizeType, const Color& color ) {
	return s_GUI->CreateElement( texture, position, layer, size, scale, sizeType, color );
}

void GUITexture::Delete() {
	s_GUI->DeleteTexture( *this );
}

void GUITexture::AddMouseCollider() {
	if( m_Collider ) {
		Game::GetLogger().Log( L"GUI", L"Element has already a collider. Doing nothing." );
		return;
	}

	float2 size = GetRenderSize() * 0.5f;
	m_Collider = MouseCollider::Create( m_Position, size );
	m_RelativeColliderSize = { 1.0f, 1.0f };
}

void GUITexture::AddMouseCollider( const float2 relativeSize ) {
	if( m_Collider ) {
		Game::GetLogger().Log( L"GUI", L"Element has already a collider. Doing nothing." );
		return;
	}
	m_RelativeColliderSize = relativeSize;
	float2 size = GetRenderSize() * 0.5f * m_RelativeColliderSize;
	m_Collider = MouseCollider::Create( m_Position, size );
}

void GUITexture::BindTexture() const {
	m_Texture->BindSRV( 0, ShaderType::PixelShader );
}

void GUITexture::SetTexture( const Texture & texture ) {
	if( &texture == m_Texture )
		return;

	m_Texture = &texture;
	s_GUI->InvalidateElementBuffer();
}

void GUITexture::SetPosition( const float2 & position ) {
	if( position == m_Position )
		return;

	m_Position = position;
	s_GUI->InvalidateElementBuffer();

	if( m_Collider )
		m_Collider->SetPosition( m_Position );
}

void GUITexture::SetLayer( uint8_t layer ) {
	if( layer == m_Layer )
		return;

	m_Layer = layer;
	s_GUI->InvalidateElementBuffer();
}

void GUITexture::SetSize( float size ) {
	if( size == m_Size )
		return;

	m_Size = size;
	s_GUI->InvalidateElementBuffer();
	UpdateColliderSize();
}

void GUITexture::SetScale( const float2 & scale ) {
	if( scale == m_Scale )
		return;

	m_Scale = scale;
	s_GUI->InvalidateElementBuffer();
	UpdateColliderSize();
}

void GUITexture::SetSizeType( SizeType sizeType ) {
	if( sizeType == m_SizeType )
		return;

	m_SizeType = sizeType;
	s_GUI->InvalidateElementBuffer();
	UpdateColliderSize();
}

void GUITexture::SetColor( const Color & color ) {
	if( color == m_Color )
		return;

	m_Color = color;
	s_GUI->InvalidateElementBuffer();
}

const float2 GUITexture::GetRenderSize() const {
	float2 size;
	switch( m_SizeType ) {
		case SizeType::WinWidthDependant:
		{
			float texAspectRatio = static_cast<float>( m_Texture->GetTextureWidth() ) / static_cast<float>( m_Texture->GetTextureHeight() );
			float winAspectRatio = Game::GetWindow().GetAspectRatio();
			size.x = m_Size;
			size.y = winAspectRatio / texAspectRatio * m_Size;
			break;
		}
		case SizeType::WinHeightDependent:
		{
			float texAspectRatio = static_cast<float>( m_Texture->GetTextureWidth() ) / static_cast<float>( m_Texture->GetTextureHeight() );
			float winAspectRatio = Game::GetWindow().GetAspectRatio();
			size.x = texAspectRatio / winAspectRatio * m_Size;
			size.y = m_Size;
			break;
		}
		case SizeType::WinDependent:
		{
			size.x = size.y = m_Size;
			break;
		}
		case SizeType::TextureDependant:
		{
			size.y = static_cast<float>( m_Texture->GetTextureHeight() ) / static_cast<float>( Game::GetWindow().GetHeight() ) * m_Size;
			size.x = static_cast<float>( m_Texture->GetTextureWidth() ) / static_cast<float>( Game::GetWindow().GetWidth() ) * m_Size;
			break;
		}
		default:
			break;
	}

	size *= 2.0f;
	size *= m_Scale;

	return size;
}

void GUITexture::UpdateColliderSize() {
	if( m_Collider ) {
		float2 size = GetRenderSize() * 0.5f * m_RelativeColliderSize;
		m_Collider->SetSize( size );
	}
}

GUITexture::GUITexture( const Texture & texture, const float2 & position, uint8_t layer, float size, const float2& scale, SizeType sizeType, const Color& color )
	: m_Texture( &texture )
	, m_Position( position )
	, m_Layer( layer )
	, m_Size( size )
	, m_Scale( scale )
	, m_SizeType( sizeType )
	, m_Color( color ) {
	s_GUI->InvalidateElementBuffer();
}

GUITexture::~GUITexture() {
	if( m_Collider )
		m_Collider->Delete();
}
