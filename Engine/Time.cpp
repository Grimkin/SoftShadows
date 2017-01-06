#include "Time.h"

#include "Game.h"


Time::Time() {
	Game::SetTime( *this );
	m_LastScaleTime = m_CurrentTime = m_StartTime = std::chrono::high_resolution_clock::now();
}

Time::~Time() {
}

Time& Time::Init() {
	static Time time;
	return time;
}

void Time::Update() {
	HighResTimePoint oldFrameTime = m_CurrentTime;
	m_CurrentTime = std::chrono::high_resolution_clock::now();
	HighResDuration newDeltaTime = m_CurrentTime - oldFrameTime;

	if( m_RealDeltaTime == 0.0f ) {
		m_RealDeltaTime = newDeltaTime.count();
		m_DeltaTime = m_RealDeltaTime * m_Multiplier;
		m_DeltaTimes.fill( m_RealDeltaTime );
		m_DeltaTimeIdx = 0;
	}
	else {
		// Average over last 5 frames
		m_DeltaTimeIdx = ( m_DeltaTimeIdx + 1 ) % m_DeltaTimes.size();
		m_DeltaTimes[m_DeltaTimeIdx] = newDeltaTime.count();
		float sumDeltaTimes = 0.0f;
		for( size_t i = 0; i < m_DeltaTimes.size(); ++i ) {
			sumDeltaTimes += m_DeltaTimes[i];
		}
		m_RealDeltaTime = sumDeltaTimes / m_DeltaTimes.size();
		m_DeltaTime = m_RealDeltaTime * m_Multiplier;
	}

	// complex calculation because of accuracy
	
	HighResDuration currentScaleDuration = m_CurrentTime - m_LastScaleTime;
	m_GameTime = m_AccGameTime + m_Multiplier * currentScaleDuration.count();

}

void Time::Reset() {
	m_LastScaleTime = m_CurrentTime = m_StartTime = std::chrono::high_resolution_clock::now();
	m_DeltaTime = 0.0f;
	m_DeltaTimes.fill( 0.0f );
}

float Time::GetDeltaTime() {
	return m_DeltaTime;
}

float Time::GetRealDeltaTime() {
	return m_RealDeltaTime;
}

float Time::GetGameTime() {
	return m_GameTime;
}

float Time::GetRealTime() {
	return HighResDuration(std::chrono::high_resolution_clock::now() - m_StartTime).count();
}

void Time::SetTimeMultiplier( float multiplier ) {
	if( multiplier == m_Multiplier )
		return;

	HighResDuration currentScaleDuration = m_CurrentTime - m_LastScaleTime;
	m_AccGameTime += currentScaleDuration.count() * m_Multiplier;
	m_LastScaleTime = m_CurrentTime;
	m_Multiplier = multiplier < 0.0f ? 0.0f : multiplier;
}

float Time::GetTimeMultiplier() {
	return m_Multiplier;
}
