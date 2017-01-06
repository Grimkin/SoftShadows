#include "InputManager.h"

#include <cassert>

#include "Logger.h"
#include "Math.h"
#include "Window.h"
#include "Makros.h"
#include "Game.h"
#include "Time.h"

InputManager* MouseCollider::s_InputManager = nullptr;

InputManager::InputManager( Window& mainWindow )
	: m_MainWindow( &mainWindow ) {
	Game::SetInput( *this );
	MouseCollider::SetInputManager( this );

	AddAxis( L"Horizontal", Key::Left, Key::Right );
	AddAxis( L"Horizontal", Key::A, Key::D );
	AddAxis( L"Vertical", Key::Down, Key::Up );
	AddAxis( L"Vertical", Key::S, Key::W );
	AddAxis( L"VerticalLook", AnalogAxis::MouseVertical );
	AddAxis( L"HorizontalLook", AnalogAxis::MouseHorizontal );

	FillTranslateMap();
}


InputManager::~InputManager() {
	SDeleteVec<MouseCollider>( m_RegisteredMouseColliders, []( MouseCollider*collider ) { delete collider; } );
}

InputManager & InputManager::Init( Window& window ) {
	static InputManager inputManager( window );
	return inputManager;
}

int InputManager::HandleMessage( UINT message, WPARAM wParam, LPARAM lParam ) {
	int result = 1;
	switch( message ) {
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			// ignore pressed keys
			if( ( lParam & ( lParam & 0x40000000 ) ) != 0 )
				result = 0;
			WPARAM keyCode = wParam;
			switch( keyCode ) {
				case VK_MENU:
					// use the bit 24, that indicates, whether right or left
					keyCode = ( ( lParam & 0x01000000 ) != 0 ) ? VK_RMENU : VK_LMENU;
					break;
				case VK_CONTROL:
					// use the bit 24, that indicates, whether right or left
					keyCode = ( ( lParam & 0x01000000 ) != 0 ) ? VK_RCONTROL : VK_LCONTROL;
					break;
				case VK_SHIFT:
					// use the scancode of the key at bit 16 - 23
					keyCode = MapVirtualKey( ( lParam & 0x00ff0000 ) >> 16, MAPVK_VSC_TO_VK_EX );
					break;
				default:
					break;
			}
			auto key = m_ParamToKey.find( keyCode );
			if( key != m_ParamToKey.end() ) {
				OnKeyDown( key->second );
				result = 0;
			}
			else {
				result = 1;
			}
			break;
		}
		case WM_SYSKEYUP:
		case WM_KEYUP:
		{
			WPARAM keyCode = wParam;
			switch( keyCode ) {
				case VK_MENU:
					// use the bit 24, that indicates, whether right or left
					keyCode = ( ( lParam & 0x01000000 ) != 0 ) ? VK_RMENU : VK_LMENU;
					break;
				case VK_CONTROL:
					// use the bit 24, that indicates, whether right or left
					keyCode = ( ( lParam & 0x01000000 ) != 0 ) ? VK_RCONTROL : VK_LCONTROL;
					break;
				case VK_SHIFT:
					// use the scancode of the key at bit 16 - 23
					keyCode = MapVirtualKey( ( lParam & 0x00ff0000 ) >> 16, MAPVK_VSC_TO_VK_EX );
					break;
				default:
					break;
			}
			auto key = m_ParamToKey.find( keyCode );
			if( key != m_ParamToKey.end() ) {
				OnKeyUp( key->second );
				result = 0;
			}
			else {
				result = 1;
			}
			break;
		}
		case WM_LBUTTONDOWN:
			OnKeyDown( Key::MouseLeft );
			result = 0;
			break;
		case WM_LBUTTONUP:
			OnKeyUp( Key::MouseLeft );
			result = 0;
			break;
		case WM_RBUTTONDOWN:
			OnKeyDown( Key::MouseRight );
			result = 0;
			break;
		case WM_RBUTTONUP:
			OnKeyUp( Key::MouseRight );
			result = 0;
			break;
		case WM_MBUTTONDOWN:
			OnKeyDown( Key::MouseMiddle );
			result = 0;
			break;
		case WM_MBUTTONUP:
			OnKeyUp( Key::MouseMiddle );
			result = 0;
			break;
		case WM_XBUTTONDOWN:
		{
			if( ( wParam & 0x000f0000 ) >> 16 == 1 )
				OnKeyDown( Key::MouseSide1 );
			else
				OnKeyDown( Key::MouseSide2 );
			result = 0;
			break;
		}
		case WM_XBUTTONUP:
		{
			if( ( wParam & 0xffff0000 ) >> 16 == 1 )
				OnKeyUp( Key::MouseSide1 );
			else
				OnKeyUp( Key::MouseSide2 );
			result = 0;
			break;
		}
		case WM_MOUSEWHEEL:
			OnMouseWheel( static_cast<float>( GET_WHEEL_DELTA_WPARAM( wParam ) ) / 120.0f );
			result = 0;
			break;
		case WM_MOUSELEAVE:
			m_IsMouseInWindow = false;
			result = 0;
			break;
		case WM_MOUSEMOVE:
			if( !m_IsMouseInWindow ) {
				m_IsMouseInWindow = true;
				ResetInput();
				result = 0;
			}
			break;
		default:
			break;
	}
	return result;
}

void InputManager::Update() {
	POINT cursorPos;
	GetCursorPos( &cursorPos );
	if( ScreenToClient( m_MainWindow->GetHandle(), &cursorPos ) && cursorPos.x > 0 && cursorPos.y > 0 ) {
		float2 fCursorPos( static_cast<float>( cursorPos.x ), static_cast<float>( cursorPos.y ) );
		m_DeltaMousePosition = ( m_MousePosition - fCursorPos );
		m_MousePosition = fCursorPos;
		float2 winSize = make_float2( m_MainWindow->GetSize() );
		m_RelativeMousePosition = m_MousePosition / winSize;
	}
	if( m_ResetMouse ) {
		m_MousePosition = make_float2( m_MainWindow->GetSize() ) * m_ResetPosition;
		m_RelativeMousePosition = m_ResetPosition;
		POINT pos;
		pos.x = static_cast<LONG>( m_MousePosition.x );
		pos.y = static_cast<LONG>( m_MousePosition.y );
		ClientToScreen( m_MainWindow->GetHandle(), &pos );
		SetCursorPos( pos.x, pos.y );
	}
	if( m_DeltaMousePosition != float2( 0.0f, 0.0f ) ) {
		for( auto& function : m_MouseMoveCallbacks ) {
			function( m_DeltaMousePosition, m_RelativeMousePosition );
		}
	}

	for( MouseCollider* collider : m_RegisteredMouseColliders ) {
		collider->CheckMouseCollision( m_RelativeMousePosition );
	}
	// reset key up / downs so it is only true for one frame
	m_KeysDown.reset();
	m_KeysUp.reset();
}

bool InputManager::KeyPressed( Key key ) {
	if( key == Key::Any ) {
		return m_PressedKeys.any();
	}
	return m_PressedKeys[static_cast<size_t>( key )];
}

bool InputManager::KeyPressed( const std::wstring& identifier ) {
	// if button doesn't exist break the game
	assert( m_CustomButtons.find( identifier ) != m_CustomButtons.end() );
	for( Key key : m_CustomButtons[identifier] ) {
		if( KeyPressed( key ) )
			return true;
	}
	return false;
}

bool InputManager::KeyDown( Key key ) {
	if( key == Key::Any ) {
		return m_KeysDown.any();
	}
	return m_KeysDown[static_cast<size_t>( key )];
}

bool InputManager::KeyDown( const std::wstring& identifier ) {
	// if button doesn't exist break the game
	assert( m_CustomButtons.find( identifier ) != m_CustomButtons.end() );
	for( Key key : m_CustomButtons[identifier] ) {
		if( KeyDown( key ) )
			return true;
	}
	return false;
}

bool InputManager::KeyUp( Key key ) {
	if( key == Key::Any ) {
		return m_KeysUp.any();
	}
	return m_KeysUp[static_cast<size_t>( key )];
}

bool InputManager::KeyUp( const std::wstring& identifier ) {
	// if button doesn't exist break the game
	assert( m_CustomButtons.find( identifier ) != m_CustomButtons.end() );
	for( Key key : m_CustomButtons[identifier] ) {
		if( KeyUp( key ) )
			return true;
	}
	return false;
}

float InputManager::GetAxisVal( const std::wstring& identifier ) {
	float val = 0.0f;
	if( m_CustomAxises.count( identifier ) > 0 ) {
		for( auto& keys : m_CustomAxises[identifier] ) {
			val -= KeyPressed( keys.first ) ? 1.0f : 0.0f;
			val += KeyPressed( keys.second ) ? 1.0f : 0.0f;
		}
	}
	val = clamp( val, -1.0f, 1.0f );
	if( val == 0.0f ) {
		if( m_CustomAnalogAxises.count( identifier ) > 0 ) {
			for( auto& function : m_CustomAnalogAxises[identifier] ) {
				float newVal = function();
				if( std::abs( newVal ) > std::abs( val ) )
					val = newVal;
			}
		}
	}

	return val;
}

float2 InputManager::GetMousePos() {
	return m_MousePosition;
}

float2 InputManager::GetMouseDeltaPos() {
	return m_DeltaMousePosition;
}

float2 InputManager::GetRelativeMousePos() {
	return m_RelativeMousePosition;
}

void InputManager::AddAxis( const std::wstring& identifier, Key neg, Key pos ) {
	// test if the axis already exists
	if( m_CustomAxises.find( identifier ) != m_CustomAxises.end() ) {
		// test if keys are already defined in this axis
		for( auto& keys : m_CustomAxises[identifier] ) {
			// do nothing if same axis already defined
			if( neg == keys.first && pos == keys.second ) {
				m_Logger->Log( L"Double Axis definition for " + identifier );
				return;
			}
			// if same keys but different order, switch it
			if( neg == keys.second && pos == keys.first ) {
				m_Logger->Log( L"Axis got switched for " + identifier );
				keys.first = neg;
				keys.second = pos;
				return;
			}
		}
	}
	// axis not defined with these keys, create it
	m_CustomAxises[identifier].push_back( { neg, pos } );
}

void InputManager::AddAxis( const std::wstring & identifier, AnalogAxis analogAxis ) {
	std::function<float()> function;
	switch( analogAxis ) {
		case AnalogAxis::MouseHorizontal:
		{
			function = [&]() {
				// multiply with fps and some value to make normal mousemovement around 1 
				return -GetMouseDeltaPos().x * Game::GetTime().GetFPS() * 0.0003f;
			};
			break;
		}
		case AnalogAxis::MouseVertical:
		{
			function = [&]() {
				// multiply with fps and some value to make normal mousemovement around 1 
				return -GetMouseDeltaPos().y * Game::GetTime().GetFPS() * 0.0003f;
			};
			break;
		}
		default:
			Game::GetLogger().Log( L"Input", L"TODO implement Axis" );
			return;
	}

	m_CustomAnalogAxises[identifier].push_back( function );
}

void InputManager::AddButton( const std::wstring& identifier, Key key ) {
	m_CustomButtons[identifier].insert( key );
	m_KeyToCustomMapping[key].insert( identifier );
}

void InputManager::DeleteAxis( const std::wstring& identifier ) {
	m_CustomAxises.erase( identifier );
}

void InputManager::DeleteAxis( const std::wstring& identifier, Key neg, Key pos ) {
	if( m_CustomAxises.find( identifier ) != m_CustomAxises.end() ) {
		for( size_t i = 0; i < m_CustomAxises[identifier].size(); ++i ) {
			auto& keys = m_CustomAxises[identifier][i];
			if( ( keys.first == neg && keys.second == pos ) || ( keys.first == pos && keys.second == neg ) ) {
				m_CustomAxises[identifier][i] = m_CustomAxises[identifier].back();
				m_CustomAxises[identifier].pop_back();
			}
		}
	}
}

void InputManager::DeleteButton( const std::wstring& identifier ) {
	if( m_CustomButtons.find( identifier ) != m_CustomButtons.end() ) {
		for( auto& key : m_CustomButtons[identifier] ) {
			m_KeyToCustomMapping[key].erase( identifier );
		}
		m_CustomButtons.erase( identifier );
	}
}

void InputManager::DeleteButton( const std::wstring& identifier, Key key ) {
	if( m_CustomButtons.find( identifier ) != m_CustomButtons.end() ) {
		m_CustomButtons[identifier].erase( key );
		m_KeyToCustomMapping[key].erase( identifier );
	}
}

void InputManager::RegisterKeyUpCallback( Key key, std::function<void()> function ) {
	m_KeyUpCallbacks[key].push_back( function );
}

void InputManager::RegisterKeyUpCallback( const std::wstring& identifier, std::function<void()> function ) {
	m_IdentifierKeyUpCallbacks[identifier].push_back( function );
}

void InputManager::RegisterKeyDownCallback( Key key, std::function<void()> function ) {
	m_KeyDownCallbacks[key].push_back( function );
}

void InputManager::RegisterKeyDownCallback( const std::wstring& identifier, std::function<void()> function ) {
	m_IdentifierKeyDownCallbacks[identifier].push_back( function );
}

void InputManager::RegisterMouseWheelCallback( std::function<void( float )> function ) {
	m_MouseWheelCallbacks.push_back( function );
}

void InputManager::RegisterMouseMoveCallback( std::function<void( const float2&deltaPos, const float2&relPos )> function ) {
	m_MouseMoveCallbacks.push_back( function );
}

void InputManager::RegisterMouseEnterCollider( MouseCollider & collider, std::function<void()> function ) {
	collider.RegisterMouseEnterCallback( function );
}

void InputManager::RegisterMouseExitCollider( MouseCollider & collider, std::function<void()> function ) {
	collider.RegisterMouseExitCallback( function );
}

void InputManager::RegisterKeyDownInCollider( MouseCollider & collider, Key key, std::function<void()> function ) {
	collider.RegisterKeyDownCallback( key, function );
}

void InputManager::RegisterKeyUpInCollider( MouseCollider & collider, Key key, std::function<void()> function ) {
	collider.RegisterKeyUpCallback( key, function );
}


void InputManager::CatchMouseAtCenter() {
	m_ResetMouse = true;
	m_ResetPosition = { 0.5f, 0.5f };
}

void InputManager::CatchMouseAtCurrentPosition() {
	m_ResetMouse = true;
	m_ResetPosition = m_RelativeMousePosition;
}

void InputManager::ReleaseMouse() {
	m_ResetMouse = false;
}

void InputManager::ShowCursor() {
	::ShowCursor( true );
}

void InputManager::HideCursor() {
	::ShowCursor( false );
}

void InputManager::RegisterMouseCollider( MouseCollider & collider ) {
	m_RegisteredMouseColliders.push_back( &collider );
}

void InputManager::UnRegisterMouseCollider( const MouseCollider & collider ) {
	for( size_t i = 0; i < m_RegisteredMouseColliders.size(); i++ ) {
		if( m_RegisteredMouseColliders[i] == &collider ) {
			m_RegisteredMouseColliders[i] = m_RegisteredMouseColliders.back();
			m_RegisteredMouseColliders.pop_back();
		}
	}
}

void InputManager::FillTranslateMap() {
	// KeyboardNumbers
	m_ParamToKey[0x30] = Key::N0;
	m_ParamToKey[0x31] = Key::N1;
	m_ParamToKey[0x32] = Key::N2;
	m_ParamToKey[0x33] = Key::N3;
	m_ParamToKey[0x34] = Key::N4;
	m_ParamToKey[0x35] = Key::N5;
	m_ParamToKey[0x36] = Key::N6;
	m_ParamToKey[0x37] = Key::N7;
	m_ParamToKey[0x38] = Key::N8;
	m_ParamToKey[0x39] = Key::N9;
	// Characters
	m_ParamToKey[0x41] = Key::A;
	m_ParamToKey[0x42] = Key::B;
	m_ParamToKey[0x43] = Key::C;
	m_ParamToKey[0x44] = Key::D;
	m_ParamToKey[0x45] = Key::E;
	m_ParamToKey[0x46] = Key::F;
	m_ParamToKey[0x47] = Key::G;
	m_ParamToKey[0x48] = Key::H;
	m_ParamToKey[0x49] = Key::I;
	m_ParamToKey[0x4A] = Key::J;
	m_ParamToKey[0x4B] = Key::K;
	m_ParamToKey[0x4C] = Key::L;
	m_ParamToKey[0x4D] = Key::M;
	m_ParamToKey[0x4E] = Key::N;
	m_ParamToKey[0x4F] = Key::O;
	m_ParamToKey[0x50] = Key::P;
	m_ParamToKey[0x51] = Key::Q;
	m_ParamToKey[0x52] = Key::R;
	m_ParamToKey[0x53] = Key::S;
	m_ParamToKey[0x54] = Key::T;
	m_ParamToKey[0x55] = Key::U;
	m_ParamToKey[0x56] = Key::V;
	m_ParamToKey[0x57] = Key::W;
	m_ParamToKey[0x58] = Key::X;
	m_ParamToKey[0x59] = Key::Y;
	m_ParamToKey[0x5A] = Key::Z;
	// German Layout Keys
	m_ParamToKey[0xDE] = Key::Ae;
	m_ParamToKey[0xC0] = Key::Oe;
	m_ParamToKey[0xBA] = Key::Ue;
	m_ParamToKey[0xBD] = Key::Sz;
	m_ParamToKey[0xDD] = Key::Apostrophe;
	m_ParamToKey[0xE2] = Key::Angle;
	m_ParamToKey[0xDC] = Key::Caret;
	m_ParamToKey[VK_OEM_MINUS] = Key::Minus;
	m_ParamToKey[VK_OEM_PLUS] = Key::Plus;
	m_ParamToKey[VK_OEM_PERIOD] = Key::Dot;
	m_ParamToKey[VK_OEM_COMMA] = Key::Comma;
	m_ParamToKey[VK_OEM_2] = Key::Hash;
	m_ParamToKey[VK_SPACE] = Key::Space;
	// Function Keys
	m_ParamToKey[VK_SHIFT] = Key::Shift;
	m_ParamToKey[VK_LSHIFT] = Key::LShift;
	m_ParamToKey[VK_RSHIFT] = Key::RShift;
	m_ParamToKey[VK_MENU] = Key::Alt;
	m_ParamToKey[VK_LMENU] = Key::LAlt;
	m_ParamToKey[VK_RMENU] = Key::RAlt;
	m_ParamToKey[VK_CONTROL] = Key::Ctrl;
	m_ParamToKey[VK_LCONTROL] = Key::LCtrl;
	m_ParamToKey[VK_RCONTROL] = Key::RCtrl;
	m_ParamToKey[VK_RETURN] = Key::Return;
	m_ParamToKey[VK_BACK] = Key::BackSpace;
	m_ParamToKey[VK_ESCAPE] = Key::Esc;
	m_ParamToKey[VK_INSERT] = Key::Insert;
	m_ParamToKey[VK_DELETE] = Key::Delete;
	m_ParamToKey[VK_HOME] = Key::Home;
	m_ParamToKey[VK_END] = Key::End;
	m_ParamToKey[VK_PRINT] = Key::Print;
	m_ParamToKey[VK_NEXT] = Key::PageDown;
	m_ParamToKey[VK_PRIOR] = Key::PageUp;
	m_ParamToKey[VK_PAUSE] = Key::Pause;
	m_ParamToKey[VK_CAPITAL] = Key::Caps;
	m_ParamToKey[VK_TAB] = Key::Tab;
	// F-Keys
	m_ParamToKey[VK_F1] = Key::F1;
	m_ParamToKey[VK_F2] = Key::F2;
	m_ParamToKey[VK_F3] = Key::F3;
	m_ParamToKey[VK_F4] = Key::F4;
	m_ParamToKey[VK_F5] = Key::F5;
	m_ParamToKey[VK_F6] = Key::F6;
	m_ParamToKey[VK_F7] = Key::F7;
	m_ParamToKey[VK_F8] = Key::F8;
	m_ParamToKey[VK_F9] = Key::F9;
	m_ParamToKey[VK_F10] = Key::F10;
	m_ParamToKey[VK_F11] = Key::F11;
	m_ParamToKey[VK_F12] = Key::F12;
	// Numpad keys
	m_ParamToKey[VK_NUMPAD0] = Key::NP_0;
	m_ParamToKey[VK_NUMPAD1] = Key::NP_1;
	m_ParamToKey[VK_NUMPAD2] = Key::NP_2;
	m_ParamToKey[VK_NUMPAD3] = Key::NP_3;
	m_ParamToKey[VK_NUMPAD4] = Key::NP_4;
	m_ParamToKey[VK_NUMPAD5] = Key::NP_5;
	m_ParamToKey[VK_NUMPAD6] = Key::NP_6;
	m_ParamToKey[VK_NUMPAD7] = Key::NP_7;
	m_ParamToKey[VK_NUMPAD8] = Key::NP_8;
	m_ParamToKey[VK_NUMPAD9] = Key::NP_9;
	m_ParamToKey[VK_ADD] = Key::NP_Plus;
	m_ParamToKey[VK_SUBTRACT] = Key::NP_Minus;
	m_ParamToKey[VK_MULTIPLY] = Key::NP_Mult;
	m_ParamToKey[VK_DIVIDE] = Key::NP_Divide;
	m_ParamToKey[VK_DECIMAL] = Key::NP_Decimal;
	// Arrow keys
	m_ParamToKey[VK_LEFT] = Key::Left;
	m_ParamToKey[VK_RIGHT] = Key::Right;
	m_ParamToKey[VK_UP] = Key::Up;
	m_ParamToKey[VK_DOWN] = Key::Down;
}

void InputManager::OnKeyDown( Key key ) {
	if( m_PressedKeys[static_cast<size_t>( key )] )
		return;

	m_PressedKeys[static_cast<size_t>( key )] = true;
	m_KeysDown[static_cast<size_t>( key )] = true;

	// invoke key down callbacks
	for( auto& function : m_KeyDownCallbacks[key] ) {
		function();
	}
	// invoke identifier key down callbacks
	if( m_KeyToCustomMapping.find( key ) != m_KeyToCustomMapping.end() ) {
		for( const std::wstring& identifier : m_KeyToCustomMapping[key] ) {
			for( auto& function : m_IdentifierKeyDownCallbacks[identifier] ) {
				function();
			}
		}
	}
}

void InputManager::OnKeyUp( Key key ) {
	m_PressedKeys[static_cast<size_t>( key )] = false;
	m_KeysUp[static_cast<size_t>( key )] = true;
	// invoke key up callbacks
	for( auto& function : m_KeyUpCallbacks[key] ) {
		function();
	}
	// invoke identifier key up callbacks
	if( m_KeyToCustomMapping.find( key ) != m_KeyToCustomMapping.end() ) {
		for( const std::wstring& identifier : m_KeyToCustomMapping[key] ) {
			for( auto& function : m_IdentifierKeyUpCallbacks[identifier] ) {
				function();
			}
		}
	}
}

void InputManager::OnMouseWheel( float delta ) {
	if( delta < 0.0f ) {
		for( auto& function : m_KeyDownCallbacks[Key::MouseWheelDown] ) {
			function();
		}
		for( auto& function : m_KeyUpCallbacks[Key::MouseWheelDown] ) {
			function();
		}
	}
	else {
		for( auto& function : m_KeyDownCallbacks[Key::MouseWheelUp] ) {
			function();
		}
		for( auto& function : m_KeyUpCallbacks[Key::MouseWheelUp] ) {
			function();
		}
	}
	for( auto& function : m_MouseWheelCallbacks ) {
		function( delta );
	}
}

void InputManager::ResetInput() {
	m_KeysDown.reset();
	m_KeysUp.reset();
	m_PressedKeys.reset();

	m_DeltaMousePosition = { 0.0f, 0.0f };
	POINT cursorPos;
	GetCursorPos( &cursorPos );
	ScreenToClient( m_MainWindow->GetHandle(), &cursorPos );
	float2 fCursorPos( static_cast<float>( cursorPos.x ), static_cast<float>( cursorPos.y ) );
	m_MousePosition = fCursorPos;
	float2 winSize = make_float2( m_MainWindow->GetSize() );
	m_RelativeMousePosition = m_MousePosition / winSize;
}

MouseCollider * MouseCollider::Create( const float2 & position, const float2 & size ) {
	MouseCollider* newMouseCollider = new MouseCollider( position, size );
	s_InputManager->RegisterMouseCollider( *newMouseCollider );
	return newMouseCollider;
}

void MouseCollider::Delete() {
	s_InputManager->UnRegisterMouseCollider( *this );
	delete this;
}

void MouseCollider::RegisterMouseEnterCallback( std::function<void()> function ) {
	m_EnterCallbacks.emplace_back( function );
}

void MouseCollider::RegisterMouseExitCallback( std::function<void()> function ) {
	m_ExitCallbacks.emplace_back( function );
}

void MouseCollider::RegisterKeyDownCallback( Key key, std::function<void()> function ) {
	m_DownCallbacks.emplace_back( key, function );
}

void MouseCollider::RegisterKeyUpCallback( Key key, std::function<void()> function ) {
	m_UpCallbacks.emplace_back( key, function );
}

void MouseCollider::CheckMouseCollision( const float2 & mousePos ) {
	bool isInside = false;
	float2 halfSize = 0.5f * m_Size;
	if( mousePos.x > m_Position.x - halfSize.x && mousePos.x < m_Position.x + halfSize.x ) {
		if( mousePos.y > m_Position.y - halfSize.y && mousePos.y < m_Position.y + halfSize.y )
			isInside = true;
	}
	if( isInside && !m_IsMouseInside ) {
		m_IsMouseInside = true;
		for( auto& function : m_EnterCallbacks ) {
			function();
		}
	}
	if( !isInside && m_IsMouseInside ) {
		m_IsMouseInside = false;
		for( auto& function : m_ExitCallbacks ) {
			function();
		}
	}
	if( m_IsMouseInside ) {
		for( auto& callBack : m_DownCallbacks ) {
			if( s_InputManager->KeyDown( callBack.first ) )
				callBack.second();
		}

		for( auto& callBack : m_UpCallbacks ) {
			if( s_InputManager->KeyUp( callBack.first ) )
				callBack.second();
		}
	}
}

MouseCollider::MouseCollider( const float2 & position, const float2 & size )
	: m_Position( position )
	, m_Size( size ) {
}

MouseCollider::~MouseCollider() {
}
