#pragma once

#include <cmath>

namespace Easing
{
	inline float Linear(float t) { return t; }

	inline float EaseInQuad(float t) { return t * t; }
	inline float EaseOutQuad(float t) { return 1.f - (1.f - t) * (1.f - t); }
	inline float EaseInOutQuad(float t)
	{
		return t < 0.5f ? 2.f * t * t : 1.f - 0.5f * (2.f * (1.f - t)) * (2.f * (1.f - t));
	}

	inline float EaseInCubic(float t) { return t * t * t; }
	inline float EaseOutCubic(float t) { float u = 1.f - t; return 1.f - u * u * u; }
	inline float EaseInOutCubic(float t)
	{
		return t < 0.5f ? 4.f * t * t * t : 1.f - 0.5f * std::pow(2.f * (1.f - t), 3.f);
	}

	inline float EaseOutBack(float t)
	{
		const float c = 1.70158f;
		return 1.f + (c + 1.f) * std::pow(t - 1.f, 3.f) + c * std::pow(t - 1.f, 2.f);
	}

	inline float EaseOutElastic(float t)
	{
		if (t <= 0.f) return 0.f;
		if (t >= 1.f) return 1.f;
		return std::pow(2.f, -10.f * t) * std::sin((t * 10.f - 0.75f) * 2.094f) + 1.f;
	}
}

using EasingFunc = float(*)(float);
