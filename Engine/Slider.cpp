#include "Slider.h"

#include "GUI.h"
#include "Texture.h"
#include "InputManager.h"
#include "Game.h"
#include "Logger.h"
#include "Makros.h"

Slider * Slider::Create( const std::wstring & name, const float2 & position, float size, float minVal, float maxVal, 
						 float deltaVal, float initialVal, uint8_t displayAccuracy, const std::wstring & unit ) {
	return new Slider( name, position, size, minVal, maxVal, deltaVal, initialVal, displayAccuracy, unit );
}

void Slider::Delete() {
	delete this;
}

void Slider::SetValue( float value ) {
	m_CurrentVal = Round( clamp( value, m_MinVal, m_MaxVal ), m_DeltaVal );
	float normedVal = ( m_CurrentVal - m_MinVal ) / ( m_MaxVal - m_MinVal );

	float sliderPosX = normedVal * ( m_MaxPos - m_MinPos ) + m_MinPos;

	m_SliderElement->SetPosition( { sliderPosX, m_Position.y } );
	if( m_IsVisible )
		m_Text->SetText( m_Name + L": " + FloatToString( m_CurrentVal, m_DisplayeAccuracy ) + L" " + m_Unit );
}

void Slider::SetVisible( bool visible ) {
	m_IsVisible = visible;
	m_BarElement->SetVisible( visible );
	m_SliderElement->SetVisible( visible );
	if( m_IsVisible ) 
		m_Text->SetText( m_Name + L": " + FloatToString( m_CurrentVal, m_DisplayeAccuracy ) + L" " + m_Unit );
	else
		m_Text->SetText( L"" );
}

Slider::Slider( const std::wstring& name, const float2& position, float size, float minVal, float maxVal,
				float deltaVal, float initialVal, uint8_t displayAccuracy, const std::wstring& unit )
	: m_Name( name )
	, m_Position( position )
	, m_Size( size )
	, m_CurrentVal( initialVal )
	, m_MinVal( minVal )
	, m_MaxVal( maxVal )
	, m_DeltaVal( deltaVal )
	, m_MinPos( position.x - size * 0.09f )
	, m_MaxPos( position.x + size * 0.09f )
	, m_DisplayeAccuracy( displayAccuracy )
	, m_Unit( unit ) {
	float2 textPosition = position;
	textPosition.x -= size * 0.1f;
	textPosition.y -= size * 0.01f;
	m_Text = GUIText::Create( L"", textPosition, 0, size * 10.0f );
	m_BarElement = GUITexture::Create( *Texture::Get( L"DummyTexture" ), position, 0, 0.2f* size, { 1.0f, 0.05f }, SizeType::WinWidthDependant, { 0.1f,0.1f,0.1f,1.0f } );
	m_SliderElement = GUITexture::Create( *Texture::Get( L"DummyTexture" ), position, 0, 0.006f * size, { 1.0f, 1.0f }, SizeType::WinWidthDependant );
	SetValue( m_CurrentVal );
	
	m_SliderElement->AddMouseCollider();

	m_SliderElement->GetMouseCollider()->RegisterKeyDownCallback( Key::MouseLeft, [&]() {
		m_IsSliderMoving = true;
	} );
	Game::GetInput().RegisterKeyUpCallback( Key::MouseLeft, [&]() {
		m_IsSliderMoving = false;
	} );
	Game::GetInput().RegisterMouseMoveCallback( [&]( const float2& deltaPos, const float2& relPos ) {
		if( m_IsSliderMoving ) {
			float sliderXPosition = clamp( relPos.x, m_MinPos, m_MaxPos );
			m_SliderElement->SetPosition( { sliderXPosition, m_Position.y } );
			UpdateValue( sliderXPosition );
		}
	} );
}


Slider::~Slider() {
	m_BarElement->Delete();
	m_SliderElement->Delete();
	m_Text->Delete();
}

void Slider::UpdateValue( float sliderXPosition ) {
	float val = ( sliderXPosition - m_MinPos ) / ( m_MaxPos - m_MinPos );
	val = val * ( m_MaxVal - m_MinVal ) + m_MinVal;
	m_CurrentVal = Round( val, m_DeltaVal );
	if( m_IsVisible )
		m_Text->SetText( m_Name + L": " + FloatToString( m_CurrentVal, m_DisplayeAccuracy ) + L" " + m_Unit );
}
