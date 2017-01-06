#include "Camera.h"
#include "Game.h"
#include "GameObjectManager.h"
#include "InputManager.h"
#include "Logger.h"
#include "Texture.h"
#include "Window.h"
#include "Math.h"

int Camera::s_NumCameraTextures = 0;

Camera::Camera( const CameraDesc & desc )
	: GameObject( desc.Parent, desc.Position, desc.Rotation )
	, m_FoV( desc.Perspective.FoV )
	, m_AspectRatio( desc.Perspective.AspectRatio )
	, m_Width( desc.Ortographic.Width )
	, m_Height( desc.Ortographic.Height )
	, m_IsPerspective( desc.IsPerspective )
	, m_NearPlane( desc.NearPlane )
	, m_FarPlane( desc.FarPlane )
	, m_Viewport( desc.Viewport ) {
	GetViewMat();
	GetProjMat();

	m_Transform.SetOnInvalidateFunction( [&]() {
		m_IsViewValid = false;
		m_IsViewProjValid = false;
		m_IsInvViewProjValid = false;
	} );

	bool success = CreateRTV( desc );
	if( !success ) {
		m_RenderTarget = Game::GetRenderBackend().GetDefaultRenderTarget();
		m_DepthStencil = Game::GetRenderBackend().GetDefaultDepthStencil();
		Game::GetLogger().Log( L"Camera", L"RTV creation for camera failed using default targets." );
	}

	if( desc.UseWindowSizeforVP ) {
		m_Viewport.Width = static_cast<float>( Game::GetWindow().GetWidth() );
		m_Viewport.Height = static_cast<float>( Game::GetWindow().GetHeight() );
		Game::GetGOManager().RegisterResizeCallback( this, [&]( uint32_t width, uint32_t height ) {
			m_Viewport.Width = static_cast<float>( width );
			m_Viewport.Height = static_cast<float>( height );
			SetAspectRatio( static_cast<float>( width ) / static_cast<float>( height ) );
		} );
	}
}

Camera::~Camera() {
}

bool Camera::CreateRTV( const CameraDesc & desc ) {
	if( desc.UseBackBufferRTV ) {
		m_RenderTarget = Game::GetRenderBackend().GetDefaultRenderTarget();
		m_DepthStencil = Game::GetRenderBackend().GetDefaultDepthStencil();
		return true;
	}
	m_RenderTarget = desc.RTV;
	m_DepthStencil = desc.DSV;

	return true;

	/*TextureDesc texDesc;
	texDesc.UniqueName = L"RenderTarget_" + std::to_wstring( s_NumCameraTextures );
	texDesc.TextureType = TextureType::Texture2D;

	Texture2DDesc desc2d;
	desc2d.BindFlags = BindFlag::RenderTarget;
	if( desc.CreateSRV )
		desc2d.BindFlags |= BindFlag::ShaderResource;
	desc2d.Format = Format::R16G16B16A16_Float;
	desc2d.Usage = Usage::Default;
	desc2d.ArraySize = 1;
	desc2d.MipLevels = 1;
	desc2d.SampleDesc = { 1,0 };
	desc2d.CPUAccessFlags = CPUAccessFlag::None;
	desc2d.MiscFlags = ResourceMiscFlag::None;
	desc2d.Height = Game::GetWindow().GetHeight();
	desc2d.Width = Game::GetWindow().GetWidth();

	texDesc.Texture2DDesc = &desc2d;

	m_RenderTarget = Game::GetTextureManager().CreateTexture( texDesc );

	texDesc.UniqueName = L"DepthStencil_" + std::to_wstring( s_NumCameraTextures );
	desc2d.Format = Format::R24G8_Typeless;
	desc2d.BindFlags = BindFlag::DepthStencil;
	desc2d.SampleDesc = { 1,0 };
	if( desc.CreateSRV )
		desc2d.BindFlags |= BindFlag::ShaderResource;

	DSVDesc dsvDesc;
	dsvDesc.Format = Format::D24_Unorm_S8_UInt;
	dsvDesc.ViewDimension = DSVDimension::Texture2D;
	dsvDesc.Flags = DSVReadOnly::None;
	dsvDesc.Texture2D.MipSlice = 0;

	SRVDesc srvDesc;
	srvDesc.Format = Format::R24_Unorm_X8_Typeless;
	srvDesc.ViewDimension = SRVDimension::Texture2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	texDesc.DSVDesc = &dsvDesc;
	texDesc.SRVDesc = &srvDesc;

	m_DepthStencil = Game::GetTextureManager().CreateTexture( texDesc );

	if( !m_RenderTarget || !m_DepthStencil )
		return false;

	return true;*/
}

void Camera::PrepareRender() {
	CameraData data;
	data.ProjMat = GetProjMat();
	data.ViewMat = GetViewMat();
	data.ViewProjMat = GetViewProjMat();
	data.InvViewProjMat = InvertMatrix( GetViewProjMat() );
	data.Position = m_Transform.GetWorldPosition();
	Game::GetGOManager().SetCameraData( data );
}

void Camera::SetPerspective( float FoV, float aspectRatio ) {
	m_IsPerspective = true;
	m_IsProjValid = false;
	m_IsViewProjValid = false;
	m_IsInvViewProjValid = false;
	m_FoV = FoV;
	m_AspectRatio = aspectRatio;
}

void Camera::SetOrthographic( float width, float height ) {
	m_IsPerspective = false;
	m_IsProjValid = false;
	m_IsViewProjValid = false;
	m_IsInvViewProjValid = false;
	m_Width = width;
	m_Height = height;
}

void Camera::SetFoV( float FoV ) {
	if( m_FoV != FoV ) {
		m_FoV = FoV;
		if( m_IsPerspective ) {
			m_IsProjValid = false;
			m_IsViewProjValid = false;
			m_IsInvViewProjValid = false;
		}
	}
}

void Camera::SetAspectRatio( float aspectRatio ) {
	if( m_AspectRatio != aspectRatio ) {
		m_AspectRatio = aspectRatio;
		if( m_IsPerspective ) {
			m_IsProjValid = false;
			m_IsViewProjValid = false;
			m_IsInvViewProjValid = false;
		}
	}
}

void Camera::SetWidthHeight( float width, float height ) {
	if( m_Width != width || m_Height != height ) {
		m_Width = width;
		m_Height = height;
		if( !m_IsPerspective ) {
			m_IsProjValid = false;
			m_IsViewProjValid = false;
			m_IsInvViewProjValid = false;
		}
	}
}

const Matrix& Camera::GetViewMat() {
	if( !m_IsViewValid ) {
		m_ViewMat = BuildViewMatrix( m_Transform.GetWorldPosition(), m_Transform.GetWorldRotation() );
		m_IsViewValid = true;
	}
	return m_ViewMat;
}

const Matrix& Camera::GetProjMat() {
	if( !m_IsProjValid ) {
		if( m_IsPerspective )
			m_ProjMat = BuildPerspectiveMatrix( m_FoV, m_AspectRatio, m_NearPlane, m_FarPlane );
		else
			m_ProjMat = BuildOrthographicMatrix( m_Width, m_Height, m_NearPlane, m_FarPlane );
		m_IsProjValid = true;
	}
	return m_ProjMat;
}

const Matrix& Camera::GetViewProjMat() {
	if( !m_IsViewProjValid ) {
		m_ViewProjMat = MatrixMultiply( GetViewMat(), GetProjMat() );
		m_IsViewProjValid = true;
	}
	return m_ViewProjMat;
}

const Matrix & Camera::GetInvViewProjMat() {
	if( !m_IsInvViewProjValid ) {
		m_InvViewProjMat = InvertMatrix( GetViewProjMat() );
		m_IsInvViewProjValid = true;
	}
	return m_InvViewProjMat;
}

float3 Camera::GetFwdVector() {
	return Rotate( { 1.f, 0.f, 0.f }, GetTransform().GetWorldRotation() );
}

float3 Camera::GetUpVector() {
	return Rotate( { 0.f, 0.f, 1.f }, GetTransform().GetWorldRotation() );
}

float3 Camera::GetRightVector() {
	return Rotate( { 0.f, 1.f, 0.f }, GetTransform().GetWorldRotation() );
}

const std::vector<RenderTargetView*> Camera::GetRTVs() {
	return{ m_RenderTarget->GetRTV() };
}

DepthStencilView* Camera::GetDSV() {
	return m_DepthStencil->GetDSV();
}
