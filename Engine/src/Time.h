#pragma once
#include <chrono>

class Time
{
public:
	Time() = delete;
	~Time() = delete;
	Time(const Time&) = delete;
	Time(Time&&) = delete;
	Time& operator=(const Time&) = delete;
	Time& operator=(Time&&) = delete;
	inline static float GetAppTime() { return AppTime; };
	inline static float GetDeltaTime() { return DeltaTime; };
public:
	//Do not modify time parameters.
	inline static float AppTime = 0.0f;
	inline static float DeltaTime = 0.0f;
};

