#include "Scene.h"

#include "Shader\VoxelDefines.hlsli"

#include "Camera.h"
#include "GameObjectManager.h"
#include "GameObject.h"
#include "Window.h"
#include "Game.h"
#include "Logger.h"
#include "GUI.h"
#include "InputManager.h"
#include "Texture.h"
#include "Slider.h"
#include "Renderer.h"
#include "DebugElements.h"
#include "Voxelizer.h"
#include "ConfigManager.h"
#include "FileLoader.h"
#include "Application.h"
#include "imgui.h"

Scene::Scene( Window& mainWindow )
	: m_MainWindow( &mainWindow ) {
}


Scene::~Scene() {
	delete m_Voxelizer;
}

void Scene::InitCamera() {
	CameraDesc cDesc;
	cDesc.Position = { 5.f,5.f,5.f };
	cDesc.Rotation = QuaternionMultiply( QuaternionFromEuler( { 0,Deg2Rad( 35.2f ),0 } ), QuaternionFromEuler( { 0,0,Deg2Rad( 225.f ) } ) );
	cDesc.IsPerspective = true;
	cDesc.Perspective.AspectRatio = m_MainWindow->GetAspectRatio();
	cDesc.Viewport.Height = static_cast<float>( m_MainWindow->GetHeight() );
	cDesc.Viewport.Width = static_cast<float>( m_MainWindow->GetWidth() );
	m_TestCamera = Game::GetGOManager().Instantiate<Camera>( cDesc );
	Game::GetRenderer().SetActiveCamera( *m_TestCamera );

	InputManager& input = Game::GetInput();

	input.RegisterKeyDownCallback( Key::MouseRight, [&]() {
		input.CatchMouseAtCurrentPosition();
		input.HideCursor();
	} );
	input.RegisterKeyUpCallback( Key::MouseRight, [&]() {
		input.ReleaseMouse();
		input.ShowCursor();
	} );

	Game::GetGOManager().RegisterUpdate( m_TestCamera, [&]( float dt ) {
		Transform& transform = m_TestCamera->GetTransform();
		float3 forward = Rotate( { 1.0f, 0.0f,0.0f }, transform.GetWorldRotation() );
		float3 right = Rotate( { 0.0f, 1.0f,0.0f }, transform.GetWorldRotation() );
		float3 up = Rotate( { 0.0f,0.0f,1.0f }, transform.GetWorldRotation() );
		float fwd = Game::GetInput().GetAxisVal( L"Vertical" );
		transform.Translate( fwd * forward * dt );
		float side = Game::GetInput().GetAxisVal( L"Horizontal" );
		transform.Translate( side * right * dt );
		if( input.KeyPressed( Key::PageUp ) ) {
			transform.Translate( { 0.f,0.f,1.f * dt } );
		}
		if( input.KeyPressed( Key::PageDown ) ) {
			transform.Translate( { 0.f,0.f,-1.f * dt } );
		}

		if( input.KeyPressed( Key::MouseRight ) ) {
			const float minAngle = Deg2Rad( -80.0f );
			const float maxAngle = Deg2Rad( 80.0f );
			float lookup = Game::GetInput().GetAxisVal( L"VerticalLook" );
			float currentAngleX = std::asinf( forward.z );
			float deltaRotX = lookup* dt;
			if( currentAngleX - deltaRotX > maxAngle )
				deltaRotX = currentAngleX - maxAngle;
			else if( currentAngleX - deltaRotX < minAngle )
				deltaRotX = currentAngleX - minAngle;
			Quaternion rotX = QuaternionFromEuler( right * deltaRotX );
			transform.Rotate( rotX );
			float lookSide = Game::GetInput().GetAxisVal( L"HorizontalLook" );
			Quaternion rotY = QuaternionFromEuler( { 0.0f, 0.0f, lookSide* dt } );
			transform.Rotate( rotY );
		}
	} );
}

void Scene::InitTestSetup() {
	Game::GetRenderBackend().SetLightSamples( 10 );
	Game::GetWindow().SetFullScreen( true );
	static std::wstring dir = Game::GetConfig().GetString( L"DebugDir" );
	Game::GetLogger().SetLogDir( dir );
	Game::GetGOManager().RegisterUpdate( m_TestCamera, [&]( float dt ) {
		static int frame = 0;
		struct {
			float3 Pos;
			Quaternion Rot;
		} params;
		if( frame == 0 ) {
			Game::GetLogger().LoadFromFile( L"Cam1.save", &params );
			m_TestCamera->GetTransform().SetPosition( params.Pos, TSpace::World );
			m_TestCamera->GetTransform().SetRotation( params.Rot, TSpace::World );

			Game::GetRenderBackend().TakeScreenShot( dir + L"Pos1_4" );
		}
		if( frame == 1 ) {
			Game::GetRenderBackend().SetLightSizeAngle( 2.f );
			Game::GetRenderBackend().TakeScreenShot( dir + L"Pos1_2" );
		}
		if( frame == 2 ) {
			Game::GetLogger().LoadFromFile( L"Cam2.save", &params );
			m_TestCamera->GetTransform().SetPosition( params.Pos, TSpace::World );
			m_TestCamera->GetTransform().SetRotation( params.Rot, TSpace::World );
			Game::GetRenderBackend().TakeScreenShot( dir + L"Pos2" );
			Game::GetApplication().Exit();
		}
		++frame;
	} );
}

void Scene::InitScene() {
	float horLightDir = Deg2Rad( Game::GetConfig().GetFloat( L"HorizontalLightDir", 315.f ) );
	float verLightDir = Deg2Rad( Game::GetConfig().GetFloat( L"VerticalLightDir", 45.f ) );
	Game::GetRenderBackend().SetLightDir( horLightDir, verLightDir );

	std::wstring sceneName = Game::GetConfig().GetString( L"SceneName", L"Pillar" );
	if( sceneName == L"Simple" ) {
		GameObjectDesc goDesc;
		goDesc.HasRenderable = true;
		goDesc.RenderableDesc.Geometry = Geometry::Create( L"Sphere.mesh" );
		if( !goDesc.RenderableDesc.Geometry ) {
			Game::GetLogger().FatalError( L"Failed to load Sphere.mesh" );
			return;
		}
		goDesc.Position = { 0.f, 0.f, .8f };
		goDesc.Scale = { .5f, .5f, .5f };
		m_TestGameObject = Game::GetGOManager().Instantiate<GameObject>( goDesc );

		goDesc.RenderableDesc.Geometry = &Geometry::GetPlaneGeometry();
		goDesc.Position = { 0.f, 0.f, 0.f };
		goDesc.Scale = { 2.5f, 2.5f, 2.5f };
		//goDesc.Rotation = QuaternionFromEuler( { 0.f, Deg2Rad( 45 ), 0.f } );
		GameObject* gameObject = Game::GetGOManager().Instantiate<GameObject>( goDesc );
		//m_TestGameObject = Game::GetGOManager().Instantiate<GameObject>( goDesc );

		RenderBackend *renderBackend = &Game::GetRenderBackend();
		renderBackend->AddToVoxelization( *m_TestGameObject );
		renderBackend->AddToVoxelization( *gameObject );
	}
	else if( sceneName == L"Pillar" ) {
		std::vector<GameObject*> gameObjects;

		GameObjectDesc goDesc;
		goDesc.HasRenderable = true;
		goDesc.RenderableDesc.Geometry = &Geometry::GetPlaneGeometry();
		goDesc.Position = { 0.f, 0.f, 0.f };
		goDesc.Scale = { 2.5f, 2.5f, 2.5f };
		gameObjects.push_back( Game::GetGOManager().Instantiate<GameObject>( goDesc ) );

		goDesc.Scale = { .5f,.5f,.5f };
		goDesc.RenderableDesc.Geometry = Geometry::Create( L"Pillar.mesh" );
		if( !goDesc.RenderableDesc.Geometry ) {
			Game::GetLogger().FatalError( L"Failed to load Pillar.mesh" );
			return;
		}
		gameObjects.push_back( Game::GetGOManager().Instantiate<GameObject>( goDesc ) );
		goDesc.Rotation = QuaternionFromEuler( { -.2f, -.4f, 0.f } );
		goDesc.Position = { 1.f, 0.7f, 0.f };
		gameObjects.push_back( Game::GetGOManager().Instantiate<GameObject>( goDesc ) );
		goDesc.Rotation = QuaternionFromEuler( { .4f, -.3f, 0.f } );
		goDesc.Position = { -0.7f, 0.5f, 0.f };
		gameObjects.push_back( Game::GetGOManager().Instantiate<GameObject>( goDesc ) );
		goDesc.Rotation = QuaternionFromEuler( { .4f, .3f, 0.f } );
		goDesc.Position = { -.9f, -.7f, 0.f };
		gameObjects.push_back( Game::GetGOManager().Instantiate<GameObject>( goDesc ) );
		goDesc.Rotation = QuaternionFromEuler( { -.2f, .2f, 0.f } );
		goDesc.Position = { .6f,-.8f, 0.f };
		gameObjects.push_back( Game::GetGOManager().Instantiate<GameObject>( goDesc ) );

		static GameObject* testGO = gameObjects[1];
		Game::GetInput().RegisterKeyDownCallback( Key::E, [&]() {
			static bool visible = false;
			testGO->SetVisibility( visible );
			visible = !visible;
		} );

		RenderBackend* renderBackend = &Game::GetRenderBackend();
		for( auto& go : gameObjects ) {
			renderBackend->AddToVoxelization( *go );
		}
	}
	else if( sceneName == L"Dragon" ) {
		GameObjectDesc goDesc;
		goDesc.HasRenderable = true;
		goDesc.RenderableDesc.Geometry = &Geometry::GetPlaneGeometry();
		goDesc.Position = { 0.f, 0.f, -.5f };
		goDesc.Scale = { 2.5f, 2.5f, 2.5f };
		GameObject* gameObject = Game::GetGOManager().Instantiate<GameObject>( goDesc );

		goDesc.RenderableDesc.Geometry = Geometry::Create( L"Dragon.mesh" );
		if( !goDesc.RenderableDesc.Geometry ) {
			Game::GetLogger().FatalError( L"Failed to load Dragon.mesh" );
			return;
		}
		goDesc.Position = { 0.f, 0.f, 0.f };
		goDesc.Scale = { 2.f, 2.f, 2.f };
		goDesc.Rotation = QuaternionFromEuler( { Deg2Rad( 90.f ), 0.f, 0.f } );
		m_TestGameObject = Game::GetGOManager().Instantiate<GameObject>( goDesc );

		RenderBackend *renderBackend = &Game::GetRenderBackend();
		renderBackend->AddToVoxelization( *gameObject );
		renderBackend->AddToVoxelization( *m_TestGameObject );
	}
	else if( sceneName == L"Outpost" ) {
		GameObjectDesc goDesc;
		goDesc.HasRenderable = true;
		goDesc.RenderableDesc.Geometry = Geometry::Create( L"Outpost.mesh" );
		if( !goDesc.RenderableDesc.Geometry ) {
			Game::GetLogger().FatalError( L"Failed to load Outpost.mesh" );
			return;
		}
		goDesc.Position = { -2.f, -2.f, 0.f };
		goDesc.Scale = { 2.f, 2.f, 2.f };
		goDesc.Rotation = QuaternionFromEuler( { Deg2Rad( 90.f ), 0.f, 0.f } );
		m_TestGameObject = Game::GetGOManager().Instantiate<GameObject>( goDesc );

		RenderBackend *renderBackend = &Game::GetRenderBackend();
		renderBackend->AddToVoxelization( *m_TestGameObject );
	}
	else if( sceneName == L"SanMiguel" ) {
		GameObjectDesc goDesc;
		goDesc.HasRenderable = true;
		goDesc.RenderableDesc.Geometry = Geometry::Create( L"SanMiguel.mesh" );
		if( !goDesc.RenderableDesc.Geometry ) {
			Game::GetLogger().FatalError( L"Failed to load SanMiguel.mesh" );
			return;
		}
		goDesc.Position = { .3f, 0.f, 0.f };
		goDesc.Scale = { 0.03f, 0.03f, 0.03f };
		goDesc.Rotation = QuaternionFromEuler( { Deg2Rad( 180.f ), 0.f, 0.f } );
		m_TestGameObject = Game::GetGOManager().Instantiate<GameObject>( goDesc );

		RenderBackend *renderBackend = &Game::GetRenderBackend();
		renderBackend->AddToVoxelization( *m_TestGameObject );
	}
	else if( sceneName == L"Walls" ) {
		std::vector<GameObject*> gameObjects;

		GameObjectDesc goDesc;
		goDesc.HasRenderable = true;
		goDesc.RenderableDesc.Geometry = &Geometry::GetPlaneGeometry();
		goDesc.Position = { 0.f, 0.f, 0.f };
		goDesc.Scale = { 2.5f, 2.5f, 2.5f };
		gameObjects.push_back( Game::GetGOManager().Instantiate<GameObject>( goDesc ) );

		goDesc.Position = { 1.f, .5f, .25f };
		goDesc.Scale = { .5f,1.f,2.f };
		goDesc.Rotation = QuaternionFromEuler( { 0.f, Deg2Rad( -90.f ), 0.f } );
		gameObjects.push_back( Game::GetGOManager().Instantiate<GameObject>( goDesc ) );

		goDesc.Position = { -.3f, 0.f, .25f };
		goDesc.Scale = { .5f,1.f,2.f };
		goDesc.Rotation = QuaternionFromEuler( { Deg2Rad( -45.f ), Deg2Rad( -90.f ), 0.f } );
		gameObjects.push_back( Game::GetGOManager().Instantiate<GameObject>( goDesc ) );

		RenderBackend* renderBackend = &Game::GetRenderBackend();
		for( auto& go : gameObjects ) {
			renderBackend->AddToVoxelization( *go );
		}
	}
	else {
		GameObjectDesc goDesc;
		goDesc.HasRenderable = true;
		if( sceneName.rfind('.') == std::wstring::npos )
			sceneName += L".mesh";
		goDesc.RenderableDesc.Geometry = Geometry::Create( sceneName );
		if( !goDesc.RenderableDesc.Geometry ) {
			Game::GetLogger().FatalError( L"Failed to load " + sceneName );
			return;
		}
		float3 scenePos = Game::GetConfig().GetFloat3( L"ScenePosition" );
		float3 sceneScale = Game::GetConfig().GetFloat3( L"SceneScale", { 1.f, 1.f, 1.f } );
		float3 rotation = Game::GetConfig().GetFloat3( L"SceneRotation" );
		goDesc.Position = scenePos;
		goDesc.Scale = sceneScale;
		goDesc.Rotation = QuaternionFromEuler( { Deg2Rad( rotation.x ), Deg2Rad( rotation.y ), Deg2Rad( rotation.z ) } );
		m_TestGameObject = Game::GetGOManager().Instantiate<GameObject>( goDesc );
		RenderBackend *renderBackend = &Game::GetRenderBackend();

		renderBackend->AddToVoxelization( *m_TestGameObject );
	}
}

void Scene::InitHotKeys() {
	Game::GetInput().RegisterKeyDownCallback( Key::F, [&]() {
		Game::GetRenderBackend().TakeScreenShot();
	} );

	static Camera* mainCamera = m_TestCamera;
	Game::GetInput().RegisterKeyDownCallback( Key::F5, [&]() {
		struct {
			float3 Position;
			Quaternion Rotation;
		} params;
		params.Position = mainCamera->GetTransform().GetWorldPosition();
		params.Rotation = mainCamera->GetTransform().GetWorldRotation();

		Game::GetLogger().SaveToFile( L"CameraPosition", params );
	} );
	Game::GetInput().RegisterKeyDownCallback( Key::F6, [&]() {
		struct {
			float3 Position;
			Quaternion Rotation;
		} params;

		if( Game::GetLogger().LoadFromFile( L"CameraPosition", &params ) ) {
			mainCamera->GetTransform().SetPosition( params.Position, TSpace::World );
			mainCamera->GetTransform().SetRotation( params.Rotation, TSpace::Local );
		}
	} );

	Game::GetInput().RegisterKeyDownCallback( Key::F10, [&]() {
		static bool fullScreen = true;
		Game::GetWindow().SetFullScreen( fullScreen );
		fullScreen = !fullScreen;
	} );
}

void Scene::LoadScene() {
	InitCamera();

	if( Game::GetConfig().GetBool( L"DebugSetup", false ) ) {
		InitTestSetup();
	}
	
	InitScene();

	InitHotKeys();
}

