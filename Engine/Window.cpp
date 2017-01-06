#include "Window.h"

#include <cassert>

#include "Game.h"
#include "Logger.h"
#include "InputManager.h"
#include "Application.h"

#include "imgui.h"
#include "ImGUI_Impl.h"

std::wstring Window::s_WindowClassName = L"Engine";
ATOM Window::s_Atom = 0;

Window::Window() {
}

Window::Window( HINSTANCE hinstance, uint32_t width/*=800*/, uint32_t height/*=600*/ )
	: m_AppInstance( hinstance )
	, m_Size( { width, height } ) {
	if( !s_Atom )
		Register( hinstance );

	bool success = Initialize();
	if( !success )
		Game::GetLogger().FatalError( L"Window creation failed" );
}


Window::~Window() {
}

Window& Window::InitMainWindow( HINSTANCE hinstance, int width, int height ) {
	static Window mainWindow( hinstance, width, height );
	Game::SetWindow( mainWindow );
	return mainWindow;
}

void Window::SetFullScreen( bool isFullscreen ) {
	if( m_IsFullscreen == isFullscreen )
		return;

	m_IsFullscreen = isFullscreen;

	uint2 deskSize = GetDesktopSize();

	if( m_IsFullscreen ) {
		m_SavedPosition = m_Position;
		m_SavedSize = m_Size;
		m_Size = deskSize;
		m_Position = { 0, 0 };
		SetWindowLongPtr( m_Handle, GWL_STYLE, WS_VISIBLE | WS_POPUP );
		SetWindowPos( m_Handle, nullptr, 0, 0, deskSize.x, deskSize.y, SWP_NOZORDER | SWP_FRAMECHANGED );
	}
	else {
		RECT rc = { 0, 0, static_cast<LONG>( m_SavedSize.x ), static_cast<LONG>( m_SavedSize.y ) };
		AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
		m_Size.x = rc.right - rc.left;
		m_Size.y = rc.bottom - rc.top;
		m_Position = m_SavedPosition;
		SetWindowLongPtr( m_Handle, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE );
		SetWindowPos( m_Handle, nullptr, m_Position.x, m_Position.y, m_Size.x, m_Size.y, SWP_NOZORDER | SWP_FRAMECHANGED );
	}
}

bool Window::Initialize() {
	RECT rc = { 0, 0, static_cast<LONG>( m_Size.x ), static_cast<LONG>( m_Size.y ) };
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );

	uint2 deskSize = GetDesktopSize();

	m_Position.x = deskSize.x / 2 - m_Size.x / 2;
	m_Position.y = deskSize.y / 2 - m_Size.y / 2;

	m_Handle = CreateWindow( m_ApplicationName.c_str(), m_ApplicationName.c_str(),
							 WS_OVERLAPPEDWINDOW | DS_NOIDLEMSG, m_Position.x, m_Position.y, rc.right - rc.left,
							 rc.bottom - rc.top, NULL, NULL, m_AppInstance, this );

	if( !m_Handle )
		return false;

	ShowWindow( m_Handle, SW_SHOWNORMAL );

	return true;
}

void Window::OnSizeChange( UINT msg, WPARAM wParam, LPARAM lParam ) {
	m_Size.x = LOWORD( lParam );
	m_Size.y = HIWORD( lParam );

	m_IsMimized = wParam == SIZE_MINIMIZED;

	if( m_OnSizeChange )
		m_OnSizeChange( false, { m_Size.x, m_Size.y } );
}

inline LRESULT Window::InitialWndProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam ) {
	if( Msg == WM_NCCREATE ) {
		assert( s_Atom );
		LPCREATESTRUCT create_struct = reinterpret_cast<LPCREATESTRUCT>( lParam );
		void * lpCreateParam = create_struct->lpCreateParams;
		Window * this_window = reinterpret_cast<Window *>( lpCreateParam );

		assert( this_window->m_Handle == 0 ); // this should be the first (and only) time
										   // WM_NCCREATE is processed
		this_window->m_Handle = hWnd;
		SetWindowLongPtr( hWnd,
						  GWLP_USERDATA,
						  reinterpret_cast<LONG_PTR>( this_window ) );
		SetWindowLongPtr( hWnd,
						  GWLP_WNDPROC,
						  reinterpret_cast<LONG_PTR>( &Window::StaticWndProc ) );
		return this_window->ActualWndProc( Msg, wParam, lParam );
	}
	// if it isn't WM_NCCREATE, do something sensible and wait until
	//   WM_NCCREATE is sent                                   
	return DefWindowProc( hWnd, Msg, wParam, lParam );
}
extern LRESULT ImGui_ImplDX11_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

inline LRESULT Window::ActualWndProc( UINT msg, WPARAM wParam, LPARAM lParam ) {
	ImGui_ImplDX11_WndProcHandler( m_Handle, msg, wParam, lParam );
	switch( msg ) {
		case WM_SYSCOMMAND:
		{
			if( wParam == SC_KEYMENU )
				return 0;
			break;
		}
		case WM_DESTROY:
		case WM_QUIT:
		case WM_CLOSE:
			Game::GetApplication().Exit();
			return 0;
		case WM_CREATE:
		{
			// all this does is perform a sanity check as it should be
			//   called after WM_NCCREATE
			LPCREATESTRUCT create_struct = reinterpret_cast<LPCREATESTRUCT>( lParam );
			void * lpCreateParam = create_struct->lpCreateParams;
			Window * this_window = reinterpret_cast<Window *>( lpCreateParam );
			assert( this_window == this );
			break;
		}
		case WM_SIZE:
		{
			ImGui_ImplDX11_InvalidateDeviceObjects();
			OnSizeChange( msg, wParam, lParam );
			ImGui_ImplDX11_CreateDeviceObjects();
			break;
		}
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_MOUSELEAVE:
		case WM_MOUSEMOVE:
			Game::GetInput().HandleMessage( msg, wParam, lParam );
			break;
		default:
			break;
	}
	return DefWindowProc( m_Handle, msg, wParam, lParam );
}

inline LRESULT Window::StaticWndProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam ) {
	LONG_PTR user_data = GetWindowLongPtr( hWnd, GWLP_USERDATA );
	Window * this_window = reinterpret_cast<Window *>( user_data );
	assert( this_window ); // WM_NCCREATE should have assigned the pointer
	assert( hWnd == this_window->m_Handle );
	return this_window->ActualWndProc( Msg, wParam, lParam );
}

void Window::Register( HINSTANCE hInstance ) {
	WNDCLASS wc = {
		CS_HREDRAW | CS_VREDRAW,
		&Window::InitialWndProc,
		0,
		sizeof( Window * ), // need to store a pointer in the user data
							// area per instance
		hInstance, // HINSTANCE for this application run
		NULL, // use default icon
		LoadCursor( nullptr, IDC_ARROW), // use default cursor
		NULL, // use the system's background color
													// change to NULL if you're making a
													//   GL window or something similar
		NULL, // no menu
		Window::s_WindowClassName.c_str()
	};

	s_Atom = RegisterClass( &wc );
	if( !s_Atom )
		Game::GetLogger().FatalError( L"Window Registration failed" );
}

bool Window::HandleMessages() {
	MSG msg;
	while( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) ) {
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
	return true;
}

uint2 Window::GetDesktopSize() {
	RECT desktop;
	HWND hDesktop = GetDesktopWindow();
	GetWindowRect( hDesktop, &desktop );
	return uint2( desktop.right - desktop.left, desktop.bottom - desktop.top );
}

void Window::SetSize( uint2 size ) {
	uint2 deskSize = GetDesktopSize();
	SetWindowPos( m_Handle, NULL, deskSize.x / 2 - size.x / 2, deskSize.y / 2 - size.y / 2, size.x, size.y, SWP_NOZORDER );
}
