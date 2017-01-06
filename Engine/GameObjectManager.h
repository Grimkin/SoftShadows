#pragma once
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <functional>

#include "ConstantBuffer.h"

class GameObject;

template<typename Owner, typename ...Params>
using MemberCallbackFunction = void( Owner::* )( Params... );

template<typename ...Params>
using CallbackFunction = std::function<void( Params... )>;

class GameObjectManager {
	friend class Game;
public:
	static GameObjectManager& Init();

	void Start();
	void Exit();

	void EarlyUpdate( float dt );
	void Update( float dt );
	void LateUpdate( float dt );
	void OnFrameEnd();

	template<typename T>
	T* Instantiate();

	template<typename T, typename ...Args>
	T* Instantiate( Args&& ... args );

	void DeleteObject( GameObject& gameObject );

	template<class T>
	void RegisterStartCallback( T* gameObject, MemberCallbackFunction<T> function );
	void RegisterStartCallback( GameObject* gameObject, CallbackFunction<> function );
	template<class T>
	void RegisterExitCallback( T* gameObject, MemberCallbackFunction<T> function );
	void RegisterExitCallback( GameObject* gameObject, CallbackFunction<> function );
	template<class T>
	void RegisterEarlyUpdate( T* gameObject, MemberCallbackFunction<T, float> function );
	void RegisterEarlyUpdate( GameObject* gameObject, CallbackFunction<float> function );
	template<class T>
	void RegisterUpdate( T* gameObject, MemberCallbackFunction<T, float> function );
	void RegisterUpdate( GameObject* gameObject, CallbackFunction<float> function );
	template<class T>
	void RegisterLateUpdate( T* gameObject, MemberCallbackFunction<T, float> function );
	void RegisterLateUpdate( GameObject* gameObject, CallbackFunction<float> function );
	template<class T>
	void RegisterResizeCallback( T* gameObject, MemberCallbackFunction<T, uint32_t, uint32_t> function );
	void RegisterResizeCallback( GameObject* gameObject, CallbackFunction<uint32_t, uint32_t> function );

	void SetObjectData( ObjectData& data );
	void SetCameraData( CameraData& data );

	void OnResize( uint32_t width, uint32_t height );
private:
	GameObjectManager();
	virtual ~GameObjectManager();

	void CleanUp();

	std::vector<GameObject*> m_GameObjects;
	std::unordered_map<const GameObject*, int> m_GameObjectIndices;
	std::vector<GameObject*> m_GameObjectsToDelete;

	std::unordered_map<const GameObject*, std::function<void()>> m_StartCallbacks;
	std::unordered_map<const GameObject*, std::function<void()>> m_ExitCallbacks;
	std::unordered_map<const GameObject*, std::function<void( float )>> m_EarlyUpdateCallbacks;
	std::unordered_map<const GameObject*, std::function<void( float )>> m_UpdateCallbacks;
	std::unordered_map<const GameObject*, std::function<void( float )>> m_LateUpdateCallbacks;
	std::unordered_map<const GameObject*, std::function< void( uint32_t, uint32_t )>> m_OnResizeCallbacks;

	ConstantBuffer<CameraData> m_CameraDataBuffer;
	ConstantBuffer<ObjectData> m_ObjectDataBuffer;
};

template<typename T>
inline T * GameObjectManager::Instantiate() {
	static_assert( std::is_base_of<GameObject, T>::value, "Object must be a GameObject" );
	T* go = new T();
	m_GameObjectIndices[go] = m_GameObjects.size();
	m_GameObjects.push_back( go );
	return go;
}

template<typename T, typename ...Args>
inline T * GameObjectManager::Instantiate( Args && ... args ) {
	static_assert( std::is_base_of<GameObject, T>::value, "Object must be a GameObject" );
	T* go = new T( args... );
	m_GameObjectIndices[go] = static_cast<uint32_t>( m_GameObjects.size() );
	m_GameObjects.push_back( go );
	return go;
}

template<class T>
inline void GameObjectManager::RegisterStartCallback( T * gameObject, MemberCallbackFunction<T> function ) {
	m_StartCallbacks[gameObject] = std::bind( function, gameObject );
}

template<class T>
inline void GameObjectManager::RegisterExitCallback( T * gameObject, MemberCallbackFunction<T> function ) {
	m_ExitCallbacks[gameObject] = std::bind( function, gameObject );
}

template<class T>
inline void GameObjectManager::RegisterEarlyUpdate( T * gameObject, MemberCallbackFunction<T, float> function ) {
	using namespace std::placeholders;
	m_EarlyUpdateCallbacks[gameObject] = std::bind( function, gameObject, _1 );
}

template<class T>
inline void GameObjectManager::RegisterUpdate( T * gameObject, MemberCallbackFunction<T, float> function ) {
	using namespace std::placeholders;
	m_UpdateCallbacks[gameObject] = std::bind( function, gameObject, _1 );
}

template<class T>
inline void GameObjectManager::RegisterLateUpdate( T * gameObject, MemberCallbackFunction<T, float> function ) {
	using namespace std::placeholders;
	m_LateUpdateCallbacks[gameObject] = std::bind( function, gameObject, _1 );
}

template<class T>
inline void GameObjectManager::RegisterResizeCallback( T * gameObject, MemberCallbackFunction<T, uint32_t, uint32_t> function ) {
	using namespace std::placeholders;
	m_OnResizeCallbacks[gameObject] = std::bind( function, gameObject, _1 );
}
