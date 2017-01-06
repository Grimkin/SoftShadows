#pragma once
#include "GameObject.h"
#include "Math.h"

class Texture;

struct CameraDesc {
	GameObject* Parent = nullptr;
	float3 Position = { 0.0f, 0.0f, 0.0f };
	Quaternion Rotation = QuaternionIdentity;
	bool IsPerspective = true;
	struct {
		float FoV = Deg2Rad( 45.0f );
		float AspectRatio = 16.0f / 9.0f;
	} Perspective;
	struct {
		float Width = 100.0f;
		float Height = 100.0f;
	} Ortographic;
	float NearPlane = 0.01f;
	float FarPlane = 10000.0f;

	bool UseWindowSizeforVP = true;
	Viewport Viewport;
	bool UseBackBufferRTV = true;
	bool UseDefaultDSV = true;
	// only use when not wanting standard types
	Texture * RTV = nullptr;
	Texture * DSV = nullptr;
};

class Camera :
	public GameObject {
	friend class GameObjectManager;
public:
	void PrepareRender();

	void SetPerspective( float FoV, float aspectRatio );
	void SetOrthographic( float width, float height );
	void SetFoV( float FoV );
	void SetAspectRatio( float aspectRatio );
	void SetWidthHeight( float width, float height );

	const Matrix& GetViewMat();
	const Matrix& GetProjMat();
	const Matrix& GetViewProjMat();
	const Matrix& GetInvViewProjMat();

	float3 GetFwdVector();
	float3 GetUpVector();
	float3 GetRightVector();
	float GetFoV() {
		return m_FoV;
	}
	float GetAspectRatio() {
		return m_AspectRatio;
	}

	const std::vector<RenderTargetView*> GetRTVs();
	DepthStencilView* GetDSV();
	const Viewport GetViewport() {
		return m_Viewport;
	}

protected:
	Camera( const CameraDesc& cDesc );
	virtual ~Camera();
private:
	bool CreateRTV( const CameraDesc& desc );

	bool m_IsPerspective;
	float m_FoV;
	float m_AspectRatio;
	float m_Width;
	float m_Height;
	float m_NearPlane;
	float m_FarPlane;

	bool m_DependantOnWindowSize = true;

	Matrix m_ViewMat;
	Matrix m_ProjMat;
	Matrix m_ViewProjMat;
	Matrix m_InvViewProjMat;

	bool m_IsViewValid = false;
	bool m_IsProjValid = false;
	bool m_IsViewProjValid = false;
	bool m_IsInvViewProjValid = false;

	Viewport m_Viewport;
	Texture* m_RenderTarget;
	Texture* m_DepthStencil;

	static int s_NumCameraTextures;
};

