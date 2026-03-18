#include "RandomUtils.h"

std::mt19937& GetRNG()
{
	static std::mt19937 rng(std::random_device{}());
	return rng;
}

int RandomInt(int l_min, int l_max)
{
	std::uniform_int_distribution<int> dist(l_min, l_max);
	return dist(GetRNG());
}

float RandomFloat(float l_min, float l_max)
{
	std::uniform_real_distribution<float> dist(l_min, l_max);
	return dist(GetRNG());
}
