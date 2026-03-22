#pragma once

#include <cmath>
#include <string>

namespace Easing
{
	// ── Linear ───────────────────────────────────────────────────────

	inline float Linear(float t) { return t; }

	// ── Quadratic ────────────────────────────────────────────────────

	inline float EaseInQuad(float t) { return t * t; }
	inline float EaseOutQuad(float t) { return 1.f - (1.f - t) * (1.f - t); }
	inline float EaseInOutQuad(float t)
	{
		return t < 0.5f ? 2.f * t * t : 1.f - 0.5f * (2.f * (1.f - t)) * (2.f * (1.f - t));
	}

	// ── Cubic ────────────────────────────────────────────────────────

	inline float EaseInCubic(float t) { return t * t * t; }
	inline float EaseOutCubic(float t) { float u = 1.f - t; return 1.f - u * u * u; }
	inline float EaseInOutCubic(float t)
	{
		return t < 0.5f ? 4.f * t * t * t : 1.f - 0.5f * std::pow(2.f * (1.f - t), 3.f);
	}

	// ── Quartic ──────────────────────────────────────────────────────

	inline float EaseInQuart(float t) { return t * t * t * t; }
	inline float EaseOutQuart(float t) { float u = 1.f - t; return 1.f - u * u * u * u; }
	inline float EaseInOutQuart(float t)
	{
		return t < 0.5f ? 8.f * t * t * t * t
					    : 1.f - 0.5f * std::pow(2.f * (1.f - t), 4.f);
	}

	// ── Quintic ──────────────────────────────────────────────────────

	inline float EaseInQuint(float t) { return t * t * t * t * t; }
	inline float EaseOutQuint(float t) { float u = 1.f - t; return 1.f - u * u * u * u * u; }
	inline float EaseInOutQuint(float t)
	{
		return t < 0.5f ? 16.f * t * t * t * t * t
					    : 1.f - 0.5f * std::pow(2.f * (1.f - t), 5.f);
	}

	// ── Exponential ──────────────────────────────────────────────────

	inline float EaseInExpo(float t)
	{
		return t <= 0.f ? 0.f : std::pow(2.f, 10.f * t - 10.f);
	}
	inline float EaseOutExpo(float t)
	{
		return t >= 1.f ? 1.f : 1.f - std::pow(2.f, -10.f * t);
	}
	inline float EaseInOutExpo(float t)
	{
		if (t <= 0.f) return 0.f;
		if (t >= 1.f) return 1.f;
		return t < 0.5f ? 0.5f * std::pow(2.f, 20.f * t - 10.f)
					    : 1.f - 0.5f * std::pow(2.f, -20.f * t + 10.f);
	}

	// ── Back ─────────────────────────────────────────────────────────

	inline float EaseInBack(float t)
	{
		const float c = 1.70158f;
		return (c + 1.f) * t * t * t - c * t * t;
	}
	inline float EaseOutBack(float t)
	{
		const float c = 1.70158f;
		return 1.f + (c + 1.f) * std::pow(t - 1.f, 3.f) + c * std::pow(t - 1.f, 2.f);
	}
	inline float EaseInOutBack(float t)
	{
		const float c = 1.70158f * 1.525f;
		if (t < 0.5f)
			return 0.5f * (4.f * t * t * ((c + 1.f) * 2.f * t - c));
		float u = 2.f * t - 2.f;
		return 0.5f * (u * u * ((c + 1.f) * u + c) + 2.f);
	}

	// ── Elastic ──────────────────────────────────────────────────────

	inline float EaseInElastic(float t)
	{
		if (t <= 0.f) return 0.f;
		if (t >= 1.f) return 1.f;
		return -std::pow(2.f, 10.f * t - 10.f) * std::sin((t * 10.f - 10.75f) * 2.094f);
	}
	inline float EaseOutElastic(float t)
	{
		if (t <= 0.f) return 0.f;
		if (t >= 1.f) return 1.f;
		return std::pow(2.f, -10.f * t) * std::sin((t * 10.f - 0.75f) * 2.094f) + 1.f;
	}
	inline float EaseInOutElastic(float t)
	{
		if (t <= 0.f) return 0.f;
		if (t >= 1.f) return 1.f;
		if (t < 0.5f)
			return -0.5f * std::pow(2.f, 20.f * t - 10.f)
				   * std::sin((20.f * t - 11.125f) * 1.3963f);
		return 0.5f * std::pow(2.f, -20.f * t + 10.f)
			   * std::sin((20.f * t - 11.125f) * 1.3963f) + 1.f;
	}

	// ── Bounce ───────────────────────────────────────────────────────

	inline float BounceOut(float t)
	{
		if (t < 1.f / 2.75f)
			return 7.5625f * t * t;
		if (t < 2.f / 2.75f)
		{
			t -= 1.5f / 2.75f;
			return 7.5625f * t * t + 0.75f;
		}
		if (t < 2.5f / 2.75f)
		{
			t -= 2.25f / 2.75f;
			return 7.5625f * t * t + 0.9375f;
		}
		t -= 2.625f / 2.75f;
		return 7.5625f * t * t + 0.984375f;
	}
	inline float BounceIn(float t) { return 1.f - BounceOut(1.f - t); }
	inline float BounceInOut(float t)
	{
		return t < 0.5f ? 0.5f * BounceIn(2.f * t)
					    : 0.5f * BounceOut(2.f * t - 1.f) + 0.5f;
	}

	// ── Spring (damped oscillation) ──────────────────────────────────

	inline float Spring(float t)
	{
		const float freq = 4.71238f; // 1.5 * PI
		const float decay = 6.f;
		return 1.f - std::cos(t * freq) * std::exp(-t * decay);
	}

	// ── Name lookup (for JSON loader) ────────────────────────────────

	using Func = float(*)(float);

	inline Func FromName(const std::string& name)
	{
		if (name == "Linear")          return Linear;
		if (name == "EaseInQuad")      return EaseInQuad;
		if (name == "EaseOutQuad")     return EaseOutQuad;
		if (name == "EaseInOutQuad")   return EaseInOutQuad;
		if (name == "EaseInCubic")     return EaseInCubic;
		if (name == "EaseOutCubic")    return EaseOutCubic;
		if (name == "EaseInOutCubic")  return EaseInOutCubic;
		if (name == "EaseInQuart")     return EaseInQuart;
		if (name == "EaseOutQuart")    return EaseOutQuart;
		if (name == "EaseInOutQuart")  return EaseInOutQuart;
		if (name == "EaseInQuint")     return EaseInQuint;
		if (name == "EaseOutQuint")    return EaseOutQuint;
		if (name == "EaseInOutQuint")  return EaseInOutQuint;
		if (name == "EaseInExpo")      return EaseInExpo;
		if (name == "EaseOutExpo")     return EaseOutExpo;
		if (name == "EaseInOutExpo")   return EaseInOutExpo;
		if (name == "EaseInBack")      return EaseInBack;
		if (name == "EaseOutBack")     return EaseOutBack;
		if (name == "EaseInOutBack")   return EaseInOutBack;
		if (name == "EaseInElastic")   return EaseInElastic;
		if (name == "EaseOutElastic")  return EaseOutElastic;
		if (name == "EaseInOutElastic") return EaseInOutElastic;
		if (name == "BounceIn")        return BounceIn;
		if (name == "BounceOut")       return BounceOut;
		if (name == "BounceInOut")     return BounceInOut;
		if (name == "Spring")          return Spring;
		return EaseOutQuad; // default
	}
}

using EasingFunc = float(*)(float);
