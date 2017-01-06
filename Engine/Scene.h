#pragma once

class GameObject;
class Camera;
class Window;
class Voxelizer;

class Scene {
public:
	Scene( Window& mainWindow );
	virtual ~Scene();

	virtual void LoadScene();
private:
	void InitCamera();
	void InitTestSetup();
	void InitScene();
	void InitHotKeys();
	GameObject* m_TestGameObject = nullptr;
	Camera* m_TestCamera = nullptr;
	Window* m_MainWindow = nullptr;
	Voxelizer* m_Voxelizer = nullptr;
};

