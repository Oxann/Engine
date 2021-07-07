#pragma once
#include <random>
#include <chrono>


class Random final
{
public:
	Random() = delete;
	~Random() = delete;
	Random(const Random&) = delete;
	Random(Random&&) = delete;
	Random& operator= (const Random&) = delete;
	Random& operator= (Random&&) = delete;
	inline static int Range(int min, int max)
	{
		std::uniform_int_distribution<int>Dist(min, max);
		return Dist(engine);
	}
	inline static float Range(float min, float max)
	{
		std::uniform_real_distribution<float>Dist(min, max);
		return Dist(engine);
	}
private:
	inline static std::mt19937 engine = std::mt19937(std::chrono::system_clock::now().time_since_epoch().count());
};

