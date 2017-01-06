#pragma once

#include <chrono>
#include <array>

typedef std::chrono::time_point<std::chrono::high_resolution_clock> HighResTimePoint;
typedef std::chrono::duration<float> HighResDuration;

class Time {
	friend class Game;
public:
	static Time& Init();

	void Update();
	void Reset();

	float GetDeltaTime();
	float GetRealDeltaTime();
	float GetGameTime();
	float GetRealTime();

	void SetTimeMultiplier( float multiplier );
	float GetTimeMultiplier();

	float GetFPS() {
		return 1.0f / m_RealDeltaTime;
	}
private:
	Time();
	virtual ~Time();

	float m_DeltaTime = 0.0f;
	float m_RealDeltaTime = 0.0f;
	std::array<float,5> m_DeltaTimes = { 0.0f };
	int m_DeltaTimeIdx = 0;
	float m_GameTime = 0.0f;
	float m_Multiplier = 1.0f;

	HighResTimePoint m_StartTime, m_CurrentTime, m_LastScaleTime;
	float m_AccGameTime = 0.0f; 
};

