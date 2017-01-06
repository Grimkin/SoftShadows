#include "GameObjectManager.h"

#include "GameObject.h"
#include "Game.h"
#include "Makros.h"

GameObjectManager::GameObjectManager() {
	Game::SetGameObjectManager( *this );
	m_CameraDataBuffer.Bind( ShaderFlag::VertexShader | ShaderFlag::PixelShader, 0 );
	m_ObjectDataBuffer.Bind( ShaderFlag::VertexShader | ShaderFlag::PixelShader, 1 );
}


GameObjectManager::~GameObjectManager() {
	CleanUp();
}

void GameObjectManager::CleanUp() {
	SDeleteVec<GameObject>( m_GameObjects, []( GameObject* gameObject ) { delete gameObject; } );
	m_GameObjectIndices.clear();
	m_GameObjectsToDelete.clear();
	m_EarlyUpdateCallbacks.clear();
	m_ExitCallbacks.clear();
	m_LateUpdateCallbacks.clear();
	m_UpdateCallbacks.clear();
	m_OnResizeCallbacks.clear();
	m_StartCallbacks.clear();
}

GameObjectManager& GameObjectManager::Init() {
	static GameObjectManager goManager;
	return goManager;
}

void GameObjectManager::Start() {
	for( auto& function : m_StartCallbacks ) {
		function.second();
	}
}

void GameObjectManager::Exit() {
	for( auto& function : m_ExitCallbacks ) {
		function.second();
	}
	CleanUp();
}

void GameObjectManager::EarlyUpdate( float dt ) {
	for( auto& function : m_EarlyUpdateCallbacks ) {
		function.second( dt );
	}
}

void GameObjectManager::Update( float dt ) {
	for( auto& function : m_UpdateCallbacks ) {
		function.second( dt );
	}
}

void GameObjectManager::LateUpdate( float dt ) {
	for( auto& function : m_LateUpdateCallbacks ) {
		function.second( dt );
	}
}

void GameObjectManager::OnFrameEnd() {
	for( GameObject* gameObject : m_GameObjectsToDelete ) {
		delete gameObject;
	}
	m_GameObjectsToDelete.clear();
}

void GameObjectManager::DeleteObject( GameObject& gameObject ) {
	auto position = m_GameObjectIndices.find( &gameObject );
	if( position != m_GameObjectIndices.end() ) {
		int idx = position->second;
		if( m_GameObjects[idx] == m_GameObjects.back() ) {
			m_GameObjects.pop_back();
			m_GameObjectIndices.erase( &gameObject );
		}
		else {
			std::swap( m_GameObjects[idx], m_GameObjects.back() );
			m_GameObjectIndices[m_GameObjects[idx]] = idx;
			m_GameObjectIndices.erase( &gameObject );
			m_GameObjects.pop_back();
		}
		m_GameObjectsToDelete.push_back( &gameObject );
		m_EarlyUpdateCallbacks.erase( &gameObject );
		m_UpdateCallbacks.erase( &gameObject );
		m_LateUpdateCallbacks.erase( &gameObject );
		m_OnResizeCallbacks.erase( &gameObject );
	}
}

void GameObjectManager::RegisterStartCallback( GameObject * gameObject, CallbackFunction<> function ) {
	m_StartCallbacks[gameObject] = function;
}

void GameObjectManager::RegisterExitCallback( GameObject * gameObject, CallbackFunction<> function ) {
	m_ExitCallbacks[gameObject] = function;
}

void GameObjectManager::RegisterEarlyUpdate( GameObject * gameObject, CallbackFunction<float> function ) {
	m_EarlyUpdateCallbacks[gameObject] = function;
}

void GameObjectManager::RegisterUpdate( GameObject * gameObject, CallbackFunction<float> function ) {
	m_UpdateCallbacks[gameObject] = function;
}

void GameObjectManager::RegisterLateUpdate( GameObject * gameObject, CallbackFunction<float> function ) {
	m_LateUpdateCallbacks[gameObject] = function;
}

void GameObjectManager::RegisterResizeCallback( GameObject * gameObject, CallbackFunction<uint32_t, uint32_t> function ) {
	m_OnResizeCallbacks[gameObject] = function;
}

void GameObjectManager::SetObjectData( ObjectData & data ) {
	m_ObjectDataBuffer.Update( data );
	m_ObjectDataBuffer.Bind( ShaderFlag::VertexShader | ShaderFlag::PixelShader, 1 );
}

void GameObjectManager::SetCameraData( CameraData & data ) {
	m_CameraDataBuffer.Update( data );
	m_CameraDataBuffer.Bind( ShaderFlag::VertexShader | ShaderFlag::PixelShader, 0 );
}

void GameObjectManager::OnResize( uint32_t width, uint32_t height ) {
	for( auto& function : m_OnResizeCallbacks ) {
		function.second( width, height );
	}
}
