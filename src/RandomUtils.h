#pragma once

#include <random>

std::mt19937& GetRNG();
int RandomInt(int l_min, int l_max);
float RandomFloat(float l_min, float l_max);
