#pragma once
#include <Windows.h>
#include <string>
#include <functional>
#include "Types.h"

class InputManager;

class Window
{
public:
	Window();
	Window( HINSTANCE hinstance, uint32_t width = 800, uint32_t height = 600 );
	virtual ~Window();
	static Window& InitMainWindow( HINSTANCE hinstance, int width = 800, int height = 600 );

	static void Register( HINSTANCE hInstance );
	static bool HandleMessages();

	HWND GetHandle() {
		return m_Handle;
	}
	void SetGraphicsCallback( std::function<void( bool, uint2 )>&& callback ) {
		m_OnSizeChange = callback;
	}
	uint2 GetDesktopSize();
	
	void SetSize( uint2 size );

	int2 GetSize() {
		return int2( m_Size.x, m_Size.y);
	}
	unsigned int GetWidth() {
		return m_Size.x;
	}
	unsigned int GetHeight() {
		return m_Size.y;
	}
	float GetAspectRatio() {
		return static_cast<float>( m_Size.x ) / static_cast<float>( m_Size.y );
	}
	void SetFullScreen( bool isFullscreen );
private:
	bool Initialize();
	void OnSizeChange( UINT msg, WPARAM wParam, LPARAM lParam );

	static LRESULT CALLBACK InitialWndProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
	LRESULT CALLBACK ActualWndProc( UINT Msg, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK StaticWndProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );

	HINSTANCE m_AppInstance = nullptr;
	HWND m_Handle = nullptr;
	std::wstring m_ApplicationName = L"Engine";

	static std::wstring s_WindowClassName;
	static ATOM s_Atom;

	uint2 m_Size = { 800,600 };
	uint2 m_Position = { 1920 / 2 - m_Size.x / 2, 1080 / 2 - m_Size.y / 2 };
	bool m_IsFullscreen = false;
	bool m_IsMimized;

	uint2 m_SavedSize;
	uint2 m_SavedPosition;

	std::function<void( bool, uint2 )> m_OnSizeChange;
};

