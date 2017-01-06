#pragma once
#include <vector>
#include <functional>

#include "Types.h"	

class GameObject;

enum class TSpace {
	Local,
	World
};

class Transform {
public:
	Transform( GameObject& owner );
	Transform( GameObject& owner, const float3 & position, const Quaternion & rotation, const float3 & scale );
	Transform( GameObject& owner, const float3 & position, const Quaternion & rotation, const float3 & scale, Transform* parent, TSpace tSpace = TSpace::Local );
	virtual ~Transform();

	float3 GetLocalPosition();
	Quaternion GetLocalRotation();
	float3 GetLocalScale();

	float3 GetWorldPosition();
	Quaternion GetWorldRotation();
	float3 GetWorldScale();

	void SetPosition( const float3& pos, TSpace tSpace = TSpace::Local );
	void SetRotation( const Quaternion& rotation, TSpace tSpace = TSpace::Local );
	void SetRotation( const float3& eulerAngles, TSpace tSpace = TSpace::Local );
	void SetScale( const float3& Scale, TSpace tSpace = TSpace::Local );

	void Translate( const float3& vec, TSpace tSpace = TSpace::Local );
	void Rotate( const Quaternion& deltaRotation, TSpace tSpace = TSpace::Local );
	void Scale( const float3& scale, TSpace tSpace = TSpace::Local );
	void Scale( float scale, TSpace tSpace = TSpace::Local );

	Matrix GetWorldTransMat();

	GameObject* GetOwner() {
		return &m_Owner;
	}
	Transform* GetParent() {
		return m_Parent;
	}
	const std::vector<Transform*>& GetChildren() {
		return m_Children;
	}
	void SetParent( Transform& parent) {
		// validate local position
		GetLocalPosition();
		RemoveParent();
		m_Parent = &parent;
		m_Parent->AddChild( *this );
		m_IsWorldPosValid = false;
	}
	void RemoveParent() {
		if( m_Parent ) {
			m_LocalPosition = GetWorldPosition();
			m_Parent->RemoveChild( *this );
			m_Parent = nullptr;
		}
	}
	void AddChild( Transform& child ) {
		// test if already child
		for( size_t i = 0; i < m_Children.size(); ++i ) {
			if( m_Children[i] == &child )
				return;
		}
		m_Children.push_back( &child );
		child.m_Parent = this;
	}
	void RemoveChild( Transform& child ) {
		for( size_t i = 0; i < m_Children.size(); ++i ) {
			if( m_Children[i] == &child ) {
				child.m_Parent = nullptr;
				child.m_LocalPosition = child.GetWorldPosition();
				m_Children[i] = m_Children.back();
				m_Children.pop_back();
			}
		}
	}

	// for now used for the camera view matrix invalidation on change
	void SetOnInvalidateFunction( std::function<void()> function ) {
		m_OnInvalidate = function;
	}
private:
	GameObject& m_Owner;

	Transform* m_Parent;
	std::vector<Transform*> m_Children;
	
	float3 m_LocalPosition = { 0.0f, 0.0f, 0.0f };
	float3 m_LocalScale = { 1.0f, 1.0f, 1.0f };
	Quaternion m_LocalRotation = QuaternionIdentity;

	float3 m_WorldPosition = { 0.0f, 0.0f, 0.0f };
	float3 m_WorldScale = { 1.0f, 1.0f, 1.0f };
	Quaternion m_WorldRotation = QuaternionIdentity;

	Matrix m_WorldTransformationMat;

	bool m_IsMatrixValid = false;
	bool m_IsLocalPosValid = true;
	bool m_IsWorldPosValid = true;
	bool m_IsLocalRotValid = true;
	bool m_IsWorldRotValid = true;
	bool m_IsLocalScaleValid = true;
	bool m_IsWorldScaleValid = true;

	std::function<void()> m_OnInvalidate;
};

