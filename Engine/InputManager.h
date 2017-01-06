#pragma once
#include <Windows.h>
#include <bitset>
#include <vector>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Types.h"

class Logger;
class Window;
class InputManager;

enum class Key {
	N0,
	N1,
	N2,
	N3,
	N4,
	N5,
	N6,
	N7,
	N8,
	N9,
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	Sz,
	OpenBracket = Sz,
	Ae,
	Quote = Ae,
	Oe,
	Tilde = Oe,
	Ue,
	Semicolon = Ue,
	Apostrophe,
	CloseBracket = Apostrophe,
	Plus,
	Minus,
	Hash,
	Comma,
	Dot,
	Angle,
	Space,
	BackSpace,
	Enter,
	Return = Enter,
	Shift,
	LShift,
	RShift,
	Ctrl,
	LCtrl,
	RCtrl,
	Alt,
	LAlt,
	RAlt,
	Caps,
	Tab,
	Caret,
	BackSlash = Caret,
	Esc,
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	Insert,
	Delete,
	Print,
	Pause,
	Home,
	Pos1 = Home,
	End,
	PageUp,
	PageDown,
	Left,
	Right,
	Up,
	Down,
	NP_0,
	NP_1,
	NP_2,
	NP_3,
	NP_4,
	NP_5,
	NP_6,
	NP_7,
	NP_8,
	NP_9,
	NP_Plus,
	NP_Minus,
	NP_Mult,
	NP_Divide,
	NP_Decimal,
	MouseRight,
	MouseLeft,
	MouseMiddle,
	MouseSide1,
	MouseSide2,
	MouseWheelUp,
	MouseWheelDown,
	Any
};

enum class AnalogAxis {
	MouseHorizontal,
	MouseVertical
};

class MouseCollider {
	friend class InputManager;
public:
	static MouseCollider* Create( const float2& position, const float2& size );
	void Delete();

	void SetPosition( const float2& position ) {
		m_Position = position;
	}
	void SetSize( const float2& size ) {
		m_Size = size;
	}
	const float2& GetSize() {
		return m_Size;
	}
	const float2& GetPosition() {
		return m_Position;
	}

	void RegisterMouseEnterCallback( std::function<void()> function );
	void RegisterMouseExitCallback( std::function<void()> function );
	void RegisterKeyDownCallback( Key key, std::function<void()> function );
	void RegisterKeyUpCallback( Key key, std::function<void()> function );

	void CheckMouseCollision( const float2& mousePos );

	static void SetInputManager( InputManager* inputManager ) {
		s_InputManager = inputManager;
	}
private:
	MouseCollider( const float2& position, const float2& size );
	virtual ~MouseCollider();

	float2 m_Position;
	float2 m_Size;

	bool m_IsMouseInside = false;

	std::vector<std::function<void()>> m_EnterCallbacks;
	std::vector<std::function<void()>> m_ExitCallbacks;
	std::vector<std::pair<Key,std::function<void()>>> m_DownCallbacks;
	std::vector<std::pair<Key,std::function<void()>>> m_UpCallbacks;

	static InputManager* s_InputManager;
};

class InputManager {
	friend class Game;
public:
	static InputManager& Init( Window& mainWindow );

	int HandleMessage( UINT message, WPARAM wparam, LPARAM lparam );
	void Update();

	bool KeyPressed( Key key );
	bool KeyPressed( const std::wstring& identifier );
	bool KeyDown( Key key );
	bool KeyDown( const std::wstring& identifier );
	bool KeyUp( Key key );
	bool KeyUp( const std::wstring& identifier );


	float GetAxisVal( const std::wstring& identifier );

	float2 GetMousePos();
	float2 GetMouseDeltaPos();
	float2 GetRelativeMousePos();

	void AddAxis( const std::wstring& identifier, Key neg, Key pos );
	void AddAxis( const std::wstring& identifier, AnalogAxis analogAxis );
	void AddButton( const std::wstring& identifier, Key key );

	void DeleteAxis( const std::wstring& identifier );
	void DeleteAxis( const std::wstring& identifier, Key neg, Key pos );
	void DeleteButton( const std::wstring& identifier );
	void DeleteButton( const std::wstring& identifier, Key key );

	void RegisterKeyUpCallback( Key key, std::function<void()> function );
	void RegisterKeyUpCallback( const std::wstring& identifier, std::function<void()> function );

	void RegisterKeyDownCallback( Key key, std::function<void()> function );
	void RegisterKeyDownCallback( const std::wstring& identifier, std::function<void()> function );

	void RegisterMouseWheelCallback( std::function<void( float )> );
	void RegisterMouseMoveCallback( std::function<void( const float2& deltaPos, const float2& relPos )> );

	void RegisterMouseEnterCollider( MouseCollider& collider, std::function<void()> function );
	void RegisterMouseExitCollider( MouseCollider& collider, std::function<void()> function );
	void RegisterKeyDownInCollider( MouseCollider& collider, Key key, std::function<void()> function );
	void RegisterKeyUpInCollider( MouseCollider& collider, Key key, std::function<void()> function );

	void CatchMouseAtCenter();
	void CatchMouseAtCurrentPosition();
	void ReleaseMouse();
	void ShowCursor();
	void HideCursor();

	void RegisterMouseCollider( MouseCollider& mouseArea );
	void UnRegisterMouseCollider( const MouseCollider& mouseArea );
private:
	InputManager( Window& mainWindow );
	~InputManager();

	void FillTranslateMap();
	void OnKeyDown( Key key );
	void OnKeyUp( Key key );
	void OnMouseWheel( float delta );

	void ResetInput();

	std::unordered_map<WPARAM, Key> m_ParamToKey;
	std::bitset<128> m_PressedKeys;
	std::bitset<128> m_KeysDown;
	std::bitset<128> m_KeysUp;

	float2 m_MousePosition;
	float2 m_DeltaMousePosition;
	float2 m_RelativeMousePosition;
	bool m_ResetMouse = false;
	float2 m_ResetPosition = { 0.5f, 0.5f };
	bool m_IsMouseInWindow = false;

	std::unordered_map<Key, std::vector<std::function<void()>>> m_KeyDownCallbacks;
	std::unordered_map<Key, std::vector<std::function<void()>>> m_KeyUpCallbacks;
	std::unordered_map<std::wstring, std::vector<std::function<void()>>> m_IdentifierKeyUpCallbacks;
	std::unordered_map<std::wstring, std::vector<std::function<void()>>> m_IdentifierKeyDownCallbacks;
	std::vector<std::function<void( float )>> m_MouseWheelCallbacks;
	std::vector<std::function<void( const float2&, const float2& )>> m_MouseMoveCallbacks;

	std::vector<MouseCollider*> m_RegisteredMouseColliders;

	std::unordered_map<std::wstring, std::vector<std::pair<Key, Key>>> m_CustomAxises;
	std::unordered_map<std::wstring, std::vector<std::function<float()>>> m_CustomAnalogAxises;
	std::unordered_map<std::wstring, std::unordered_set<Key>> m_CustomButtons;
	std::unordered_map<Key, std::unordered_set<std::wstring>> m_KeyToCustomMapping;

	Logger* m_Logger;

	Window* m_MainWindow;
};

