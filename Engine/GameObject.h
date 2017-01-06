#pragma once

#include "Renderable.h"
#include "Geometry.h"
#include "Types.h"
#include "Transform.h"
#include "Makros.h"

struct GameObjectDesc {
	float3 Position = { 0.0f, 0.0f, 0.0f };
	Quaternion Rotation = QuaternionIdentity;
	float3 Scale = { 1.0f, 1.0f, 1.0f };
	bool HasRenderable = false;
	RenderableDesc RenderableDesc;
	GameObject* Parent = nullptr;
	TSpace InitializerSpace = TSpace::Local;
};

class GameObject {
	friend class GameObjectManager;
	friend class Renderable;
public:
	const Transform& GetTransform() const{
		return m_Transform;
	}
	Transform& GetTransform() {
		return m_Transform;
	}
	GameObject* GetParent() {
		if( m_Transform.GetParent() )
			return m_Transform.GetParent()->GetOwner();
		else
			return nullptr;
	}
	std::vector<GameObject*> GetChildren() {
		std::vector<GameObject*> children;
		for( Transform* child : m_Transform.GetChildren() ) {
			children.push_back( child->GetOwner() );
		}
		return children;
	}
	void SetVisibility( bool visible ) {
		if( m_Renderable )
			m_Renderable->SetVisibility( visible );
	}
	bool IsVisible() const {
		if( m_Renderable )
			return m_Renderable->IsVisible();
		return false;
	}
	void SetParent( GameObject& parent ) {
		m_Transform.SetParent( parent.GetTransform() );
	}
	void RemoveParent() {
		m_Transform.RemoveParent();
	}
	void AddChild( GameObject& child ) {
		m_Transform.AddChild( child.GetTransform() );
	}
	void RemoveChild( GameObject& child ) {
		m_Transform.RemoveChild( child.GetTransform() );
	}
	Renderable* GetRenderable() {
		return m_Renderable;
	}
	void Delete();
protected:
	GameObject( const GameObjectDesc& desc );
	GameObject( GameObject* parent = nullptr, const float3& position = { 0.0f, 0.0f, 0.0f }, const Quaternion& rotation = QuaternionIdentity, const float3& scale = { 1.0f, 1.0f, 1.0f } );
	GameObject( const RenderableDesc& rInit, GameObject* parent = nullptr, const float3& position = { 0.0f, 0.0f, 0.0f }, const Quaternion& rotation = QuaternionIdentity, const float3& scale = { 1.0f, 1.0f, 1.0f } );
	virtual ~GameObject();

	Transform m_Transform;
	Renderable* m_Renderable;
};
