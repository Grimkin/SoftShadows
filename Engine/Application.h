#pragma once

#include <Windows.h>

class Window;
class Logger;
class RenderBackend;
class InputManager;
class Time;
class GameObjectManager;
class RenderPassManager;
class Renderer;
class MaterialManager;
class TextureManager;
class GUI;
class FontManager;
class GeometryManager;
class ConfigManager;

class Application
{
	friend class Game;
public:
	static Application& Init( HINSTANCE hinstance );

	void Run();
	void Exit();
private:
	Application( HINSTANCE hinstance );
	virtual ~Application();

	void Initialize( HINSTANCE hInstance );

	bool m_Run = true;

	Logger* m_Logger = nullptr;
	Window* m_Window = nullptr;
	GameObjectManager* m_GameObjectManager = nullptr;
	RenderBackend* m_RenderBackend = nullptr;
	InputManager* m_InputManager = nullptr;
	Time* m_Time = nullptr;
	RenderPassManager* m_RenderPassManager = nullptr;
	Renderer* m_Renderer = nullptr;
	MaterialManager* m_MaterialManager = nullptr;
	TextureManager* m_TextureManager = nullptr;
	GUI* m_GUI = nullptr;
	FontManager* m_FontManager = nullptr;
	GeometryManager* m_GeometryManager = nullptr;
	ConfigManager* m_ConfigManager = nullptr;
};

