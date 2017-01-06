#include "Transform.h"

#include "Math.h"

Transform::Transform( GameObject& owner )
	: Transform( owner, { 0.0f, 0.0f, 0.0f }, QuaternionIdentity, { 1.0f, 1.0f, 1.0f } ) {
}

Transform::Transform( GameObject& owner, const float3 & position, const Quaternion & rotation, const float3 & scale )
	: m_Owner( owner )
	, m_LocalPosition( position )
	, m_WorldPosition( position )
	, m_LocalRotation( rotation )
	, m_WorldRotation( rotation )
	, m_LocalScale( scale )
	, m_WorldScale( scale ) {
}

Transform::Transform( GameObject& owner, const float3 & position, const Quaternion & rotation, const float3 & scale, Transform* parent, TSpace tSpace )
	: m_Owner( owner )
	, m_Parent( parent ) {
	// treat parameters as world parameters
	
	if( m_Parent ) {
		if( tSpace == TSpace::World ) {
			m_WorldPosition = position;
			m_WorldRotation = rotation;
			m_WorldScale = scale;

			m_LocalPosition = m_WorldPosition - m_Parent->GetWorldPosition();
			m_LocalRotation = QuaternionInvMultiply( m_WorldRotation, m_Parent->GetWorldRotation() );
			m_LocalScale = m_WorldScale - m_Parent->GetWorldScale();
		}
		// treat parameters as local parameters
		else {
			m_LocalPosition = position;
			m_LocalRotation = rotation;
			m_LocalScale = scale;

			m_WorldPosition = m_Parent->GetWorldPosition() + m_LocalPosition;
			m_WorldRotation = QuaternionMultiply( m_LocalRotation, m_Parent->GetWorldRotation() );
			m_WorldScale = m_Parent->GetLocalScale() + m_LocalScale;
		}
	}
	else {
		m_WorldPosition = position;
		m_LocalPosition = position;
		m_WorldRotation = rotation;
		m_LocalRotation = rotation;
		m_WorldScale = scale; 
		m_LocalScale = scale;
	}
}


Transform::~Transform() {
}

float3 Transform::GetLocalPosition() {
	if( !m_IsLocalPosValid ) {
		assert( m_Parent );
		m_LocalPosition = m_WorldPosition - m_Parent->GetWorldPosition();
		m_IsLocalPosValid = true;
	}
	return m_LocalPosition;
}

Quaternion Transform::GetLocalRotation() {
	if( !m_IsLocalRotValid ) {
		assert( m_Parent );
		m_LocalRotation = QuaternionInvMultiply( m_WorldRotation, m_Parent->GetWorldRotation() );
		m_IsLocalRotValid = true;
	}
	return m_LocalRotation;
}

float3 Transform::GetLocalScale() {
	if( !m_IsLocalScaleValid ) {
		assert( m_Parent );
		m_LocalScale = m_WorldScale - m_Parent->GetWorldScale();
		m_IsLocalScaleValid = true;
	}
	return m_LocalScale;
}

float3 Transform::GetWorldPosition() {
	if( !m_IsWorldPosValid ) {
		assert( m_Parent );
		m_WorldPosition = m_Parent->GetWorldPosition() + m_LocalPosition;
		m_IsWorldPosValid = true;
	}
	return m_WorldPosition;
}

Quaternion Transform::GetWorldRotation() {
	if( !m_IsWorldRotValid ) {
		assert( m_Parent );
		m_WorldRotation = QuaternionMultiply( m_LocalRotation, m_Parent->GetWorldRotation() );
		m_IsWorldRotValid = true;
	}
	return m_WorldRotation;
}

float3 Transform::GetWorldScale() {
	if( !m_IsWorldScaleValid ) {
		assert( m_Parent );
		m_WorldScale = m_Parent->GetWorldScale() + m_LocalScale;
		m_IsLocalScaleValid = true;
	}
	return m_WorldScale;
}


void Transform::SetPosition( const float3 & pos, TSpace tSpace ) {
	if( tSpace == TSpace::World ) {
		m_WorldPosition = pos;
		if( m_Parent )
			m_IsLocalPosValid = false;
		else
			m_LocalPosition = pos;
		m_IsWorldPosValid = true;
	}
	else {
		m_LocalPosition = pos;
		if( m_Parent )
			m_IsWorldPosValid = false;
		else
			m_WorldPosition = pos;
		m_IsLocalPosValid = true;
	}
	m_OnInvalidate();
	m_IsMatrixValid = false;
}

void Transform::SetRotation( const Quaternion & rotation, TSpace tSpace ) {
	if( tSpace == TSpace::World ) {
		m_WorldRotation = rotation;
		if( m_Parent )
			m_IsLocalRotValid = false;
		else
			m_LocalRotation = rotation;
		m_IsWorldRotValid = true;
	}
	else {
		m_LocalRotation = rotation;
		if( m_Parent )
			m_IsWorldRotValid = false;
		else
			m_WorldRotation = rotation;
		m_IsLocalRotValid = true;
	}
	m_OnInvalidate();
	m_IsMatrixValid = false;
}

void Transform::SetRotation( const float3 & eulerAngles, TSpace tSpace ) {
	Quaternion rotation = QuaternionFromEuler( eulerAngles );
	if( tSpace == TSpace::World ) {
		m_WorldRotation = rotation;
		if( m_Parent )
			m_IsLocalRotValid = false;
		else
			m_LocalRotation = rotation;
		m_IsWorldRotValid = true;
	}
	else {
		m_LocalRotation = rotation;
		if( m_Parent )
			m_IsWorldRotValid = false;
		else
			m_WorldRotation = rotation;
		m_IsLocalRotValid = true;
	}
	m_OnInvalidate();
	m_IsMatrixValid = false;
}

void Transform::SetScale( const float3 & scale, TSpace tSpace ) {
	if( tSpace == TSpace::World ) {
		m_WorldScale = scale;
		if( m_Parent )
			m_IsLocalRotValid = false;
		else
			m_LocalScale = scale;
		m_IsWorldScaleValid = true;
	}
	else {
		m_LocalScale = scale;
		if( m_Parent )
			m_IsWorldRotValid = false;
		else
			m_WorldScale = scale;
		m_IsLocalScaleValid = true;
	}
	m_OnInvalidate();
	m_IsMatrixValid = false;
}

void Transform::Translate( const float3 & vec, TSpace tSpace ) {
	if( tSpace == TSpace::World ) {
		m_WorldPosition = GetWorldPosition() + vec;
		if( m_Parent )
			m_IsLocalPosValid = false;
		else
			m_LocalPosition = m_WorldPosition;
		m_IsWorldPosValid = true;
	}
	else {
		m_LocalPosition = GetLocalPosition() + vec;
		if( m_Parent )
			m_IsWorldPosValid = false;
		else
			m_WorldPosition = m_LocalPosition;
	}
	m_OnInvalidate();
	m_IsMatrixValid = false;
}

void Transform::Rotate( const Quaternion & quat, TSpace tSpace ) {
	if( tSpace == TSpace::World ) {
		m_WorldRotation = QuaternionMultiply( GetWorldRotation(), quat );
		if( m_Parent )
			m_IsLocalRotValid = false;
		else
			m_LocalRotation = m_WorldRotation;
	}
	else {
		m_LocalRotation = QuaternionMultiply( GetLocalRotation(), quat );
		if( m_Parent )
			m_IsWorldRotValid = false;
		else
			m_WorldRotation = m_LocalRotation;
	}
	if( m_OnInvalidate )
		m_OnInvalidate();
	m_IsMatrixValid = false;
}

void Transform::Scale( const float3 & scale, TSpace tSpace ) {
	if( tSpace == TSpace::World ) {
		m_WorldScale = GetWorldScale() + scale;
		if( m_Parent )
			m_IsLocalPosValid = false;
		else
			m_LocalScale = m_WorldScale;
		m_IsWorldPosValid = true;
	}
	else {
		m_LocalScale = GetLocalScale() + scale;
		if( m_Parent )
			m_IsWorldPosValid = false;
		else
			m_WorldScale = m_LocalScale;
	}
	m_OnInvalidate();
	m_IsMatrixValid = false;
}

void Transform::Scale( float scale, TSpace tSpace ) {
	if( tSpace == TSpace::World ) {
		m_WorldScale = GetWorldScale() + scale;
		if( m_Parent )
			m_IsLocalPosValid = false;
		else
			m_LocalScale = m_WorldScale;
		m_IsWorldPosValid = true;
	}
	else {
		m_LocalScale = GetLocalScale() + scale;
		if( m_Parent )
			m_IsWorldPosValid = false;
		else
			m_WorldScale = m_LocalScale;
	}
	m_OnInvalidate();
	m_IsMatrixValid = false;
}

Matrix Transform::GetWorldTransMat() {
	if( !m_IsMatrixValid ) {
		m_WorldTransformationMat = BuildTransformationMatrix( GetWorldPosition(), GetWorldRotation(), GetWorldScale() );
		m_IsMatrixValid = true;
	}
	return m_WorldTransformationMat;
}
