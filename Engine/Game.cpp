#include "Game.h"

#include "Application.h"
#include "RenderBackend.h"
#include "Window.h"
#include "InputManager.h"
#include "GameObjectManager.h"
#include "Time.h"
#include "Logger.h"

void Game::SetApplication( Application & application ) {
	Get().m_Application = &application;
}

void Game::SetRenderBackend( RenderBackend & renderBackend ) {
	Get().m_RendererBackend = &renderBackend;
}

void Game::SetWindow( Window & window ) {
	Get().m_Window = &window;
}

void Game::SetInput( InputManager & inputManager ) {
	Get().m_InputManager = &inputManager;
}

void Game::SetGameObjectManager( GameObjectManager & gameObjectManager ) {
	Get().m_GameObjectManager = &gameObjectManager;
}

void Game::SetTime( Time & time ) {
	Get().m_Time = &time;
}

void Game::SetLogger( Logger & logger ) {
	Get().m_Logger = &logger;
}

void Game::SetShaderManager( ShaderManager & shaderManager ) {
	Get().m_ShaderManager = &shaderManager;
}

void Game::SetRenderPassManager( RenderPassManager & renderPassManager ) {
	Get().m_RenderPassManager = &renderPassManager;
}

void Game::SetDevice( ID3D11Device & device ) {
	Get().m_Device = &device;
}

void Game::SetContext( ID3D11DeviceContext & context ) {
	Get().m_Context = &context;
}

void Game::SetRenderer( Renderer & renderer ) {
	Get().m_Renderer = &renderer;
}

void Game::SetMaterialManager( MaterialManager & materialManager ) {
	Get().m_MaterialManager = &materialManager;
}

void Game::SetTextureManager( TextureManager & textureManager ) {
	Get().m_TextureManager = &textureManager;
}

void Game::SetGUI( GUI & gui ) {
	Get().m_GUI = &gui;
}

void Game::SetFontManager( FontManager & fontManager ) {
	Get().m_FontManager = &fontManager;
}

void Game::SetGeometryManager( GeometryManager & geometryManager ) {
	Get().m_GeometryManager = &geometryManager;
}

void Game::SetFileLoader( FileLoader & fileLoader ) {
	Get().m_FileLoader = &fileLoader;
}

void Game::SetConfig( ConfigManager & config ) {
	Get().m_Config = &config;
}

Game::Game() {
}

Game::~Game() {
}
