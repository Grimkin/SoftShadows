#include "Application.h"

#include "Window.h"
#include "Logger.h"
#include "Makros.h"
#include "D3DRenderBackend.h"
#include "InputManager.h"
#include "Time.h"
#include "GameObjectManager.h"
#include "Game.h"
#include "Renderer.h"
#include "Material.h"
#include "Texture.h"
#include "GUI.h"
#include "FontManager.h"
#include "Geometry.h"
#include "FileLoader.h"
#include "RenderPass.h"
#include "DebugElements.h"
#include "ConfigManager.h"

Application::Application( HINSTANCE hInstance ) {
	Game::SetApplication( *this );
	Initialize( hInstance );
}

void Application::Initialize( HINSTANCE hInstance ) {
	m_Logger = &Logger::InitMainLogger();
	m_ConfigManager = &ConfigManager::Init( L"Assets\\Config\\main.config" );
	m_Time = &Time::Init();
	int2 winSize = m_ConfigManager->GetInt2( L"WinSize", { 800,800 } );
	m_Window = &Window::InitMainWindow( hInstance, winSize.x, winSize.y );
	m_RenderPassManager = &RenderPassManager::Init();
	m_TextureManager = &TextureManager::Init();
	m_RenderBackend = &D3DRenderBackend::Init( *m_Window );
	FileLoader::Init( &Game::GetDevice(), &Game::GetContext() );
	m_GameObjectManager = &GameObjectManager::Init();
	m_InputManager = &InputManager::Init( *m_Window );
	m_Renderer = &Renderer::Init();
	m_MaterialManager = &MaterialManager::Init();
	m_GUI = &GUI::Init();
	m_GeometryManager = &GeometryManager::Init();
	DebugElementsManager::Init( *m_RenderBackend );
	m_TextureManager->CreateDummyTexture();

	m_RenderBackend->Start();
}

Application& Application::Init( HINSTANCE hinstance ) {
	static Application application( hinstance );
	return application;
}

void Application::Run() {
	while( m_Run ) {
		Window::HandleMessages();

		m_Time->Update();
		float dt = m_Time->GetDeltaTime();
		m_GameObjectManager->EarlyUpdate( dt );
		m_GameObjectManager->Update( dt );
		m_RenderBackend->Render();
		m_GameObjectManager->LateUpdate( dt );
		m_GameObjectManager->OnFrameEnd();
		m_InputManager->Update();
	}
	m_GameObjectManager->Exit();
	m_RenderBackend->Exit();
}

void Application::Exit() {
	m_Run = false;
}


Application::~Application() {
}
