#pragma once
#include <cassert>
#include <memory>

class RenderBackend;
class Application;
class Window;
class InputManager;
class GameObjectManager;
class Time;
class Logger;
class ShaderManager;
class RenderPassManager;
class Renderer;
class MaterialManager;
class TextureManager;
class GUI;
class FontManager;
class GeometryManager;
class FileLoader;
class ConfigManager;

struct ID3D11Device;
struct ID3D11DeviceContext;


class Game {
	friend class Application;
public:
	static Game& Get() {
		static Game game;
		return game;
	}

	static void SetApplication( Application& application );
	static void SetRenderBackend( RenderBackend& renderBackend );
	static void SetWindow( Window& window );
	static void SetInput( InputManager& inputManager );
	static void SetGameObjectManager( GameObjectManager& gameObjectManager );
	static void SetTime( Time& time );
	static void SetLogger( Logger& logger );
	static void SetShaderManager( ShaderManager& shaderManager );
	static void SetRenderPassManager( RenderPassManager& renderPassManager );
	static void SetDevice( ID3D11Device& device );
	static void SetContext( ID3D11DeviceContext& context );
	static void SetRenderer( Renderer& renderer );
	static void SetMaterialManager( MaterialManager& materialManager );
	static void SetTextureManager( TextureManager& textureManager );
	static void SetGUI( GUI& gui );
	static void SetFontManager( FontManager& fontManager );
	static void SetGeometryManager( GeometryManager& geometryManager );
	static void SetFileLoader( FileLoader& fileLoader );
	static void SetConfig( ConfigManager& config );

	static Application& GetApplication() {
		assert( Get().m_Application );
		return *Get().m_Application;
	}
	static RenderBackend& GetRenderBackend() {
		assert( Get().m_RendererBackend );
		return *Get().m_RendererBackend;
	}
	static Window& GetWindow() {
		assert( Get().m_Window );
		return *Get().m_Window;
	}
	static InputManager& GetInput() {
		assert( Get().m_InputManager );
		return *Get().m_InputManager;
	}
	static Time& GetTime() {
		assert( Get().m_Time );
		return *Get().m_Time;
	}
	static Logger& GetLogger() {
		assert( Get().m_Logger );
		return *Get().m_Logger;
	}
	static GameObjectManager& GetGOManager() {
		assert( Get().m_GameObjectManager );
		return *Get().m_GameObjectManager;
	}
	static ShaderManager& GetShaderManager() {
		assert( Get().m_ShaderManager );
		return *Get().m_ShaderManager;
	}
	static RenderPassManager& GetRenderPassManager() {
		assert( Get().m_RenderPassManager );
		return *Get().m_RenderPassManager;
	}
	static ID3D11Device& GetDevice() {
		assert( Get().m_Device );
		return *Get().m_Device;
	}
	static ID3D11DeviceContext& GetContext() {
		assert( Get().m_Context );
		return *Get().m_Context;
	}
	static Renderer& GetRenderer() {
		assert( Get().m_Renderer );
		return *Get().m_Renderer;
	}
	static MaterialManager& GetMaterialManager() {
		assert( Get().m_MaterialManager );
		return *Get().m_MaterialManager;
	}
	static TextureManager& GetTextureManager() {
		assert( Get().m_TextureManager );
		return *Get().m_TextureManager;
	}
	static GUI& GetGUI() {
		assert( Get().m_GUI );
		return *Get().m_GUI;
	}
	static FontManager& GetFontManager() {
		assert( Get().m_FontManager );
		return *Get().m_FontManager;
	}
	static GeometryManager& GetGeometryManager() {
		assert( Get().m_GeometryManager );
		return *Get().m_GeometryManager;
	}
	static FileLoader& GetFileLoader() {
		assert( Get().m_FileLoader );
		return *Get().m_FileLoader;
	}

	static ConfigManager& GetConfig() {
		assert( Get().m_Config );
		return *Get().m_Config;
	}

private:
	Game();
	~Game();

	RenderBackend* m_RendererBackend = nullptr;
	Application* m_Application = nullptr;
	Window* m_Window = nullptr;
	InputManager* m_InputManager = nullptr;
	GameObjectManager* m_GameObjectManager = nullptr;
	Time* m_Time = nullptr;
	Logger* m_Logger = nullptr;
	ShaderManager* m_ShaderManager = nullptr;
	RenderPassManager* m_RenderPassManager = nullptr;
	ID3D11Device* m_Device = nullptr;
	ID3D11DeviceContext* m_Context = nullptr;
	Renderer* m_Renderer = nullptr;
	MaterialManager* m_MaterialManager = nullptr;
	TextureManager* m_TextureManager = nullptr;
	GUI* m_GUI = nullptr;
	FontManager* m_FontManager = nullptr;
	GeometryManager* m_GeometryManager = nullptr;
	FileLoader* m_FileLoader = nullptr;
	ConfigManager* m_Config = nullptr;
};
