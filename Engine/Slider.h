#pragma once
#include <string>

#include "Types.h"

class GUIText;
class GUITexture;
class MouseCollider;

class Slider {
public:
	static Slider* Create( const std::wstring& name, const float2& position, float size, float minVal, float maxVal, 
						   float deltaVal, float initialVal, uint8_t displayAccuracy = 2, const std::wstring& unit = L"" );
	void Delete();

	void SetValue( float value );
	float GetValue() const {
		return m_CurrentVal;
	}
	void SetVisible( bool visible );
private:
	Slider( const std::wstring& name, const float2& position, float size, float minVal, float maxVal,
			float deltaVal, float initialVal, uint8_t displayAccuracy = 2, const std::wstring& unit = L"" );
	virtual ~Slider();

	void UpdateValue( float sliderXPosition );

	std::wstring m_Name;
	float2 m_Position;
	float m_Size;
	float m_CurrentVal;
	float m_MinVal;
	float m_MaxVal;
	float m_DeltaVal;
	float m_MinPos;
	float m_MaxPos;
	uint8_t m_DisplayeAccuracy;
	std::wstring m_Unit;

	bool m_IsSliderMoving = false;

	GUIText* m_Text;
	GUITexture* m_SliderElement;
	GUITexture* m_BarElement;

	bool m_IsVisible = true;
};

