#include "GameObject.h"
#include "GameObjectManager.h"
#include "InputManager.h"
#include "Math.h"
#include "Game.h"

void GameObject::Delete() {
	Game::GetGOManager().DeleteObject( *this );
}

GameObject::GameObject( const GameObjectDesc & desc )
	: m_Transform( *this, desc.Position, desc.Rotation, desc.Scale, desc.Parent ? &desc.Parent->GetTransform() : nullptr, desc.InitializerSpace ){
	if( desc.HasRenderable )
		m_Renderable = new Renderable( desc.RenderableDesc, *this );
}

GameObject::GameObject( GameObject* parent, const float3 & position, const Quaternion & rotation, const float3 & scale )
	: m_Transform( *this, position, rotation, scale ) {
}

GameObject::GameObject(const RenderableDesc& rDesc, GameObject* parent, const float3 & position, const Quaternion & rotation, const float3 & scale )
	: m_Transform( *this, position, rotation, scale )
	, m_Renderable( new Renderable( rDesc, *this ) ) {
}

GameObject::~GameObject() {
	delete m_Renderable;
}
