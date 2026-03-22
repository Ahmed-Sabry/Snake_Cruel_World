#pragma once

#include "CutsceneAction.h"
#include "CutsceneActions.h"
#include <functional>
#include <string>
#include <cmath>

// Expression function: takes (elapsed_time, entity_pointer) → value
using ExprFunc = std::function<float(float, const CutsceneEntity*)>;

namespace Expr
{
	// ── Oscillators ──────────────────────────────────────────────

	inline ExprFunc Sin(float l_freq, float l_amplitude, float l_phase = 0.f, float l_offset = 0.f)
	{
		return [=](float t, const CutsceneEntity*) -> float {
			return std::sin(t * l_freq + l_phase) * l_amplitude + l_offset;
		};
	}

	inline ExprFunc Cos(float l_freq, float l_amplitude, float l_phase = 0.f, float l_offset = 0.f)
	{
		return [=](float t, const CutsceneEntity*) -> float {
			return std::cos(t * l_freq + l_phase) * l_amplitude + l_offset;
		};
	}

	inline ExprFunc Triangle(float l_freq, float l_amplitude, float l_offset = 0.f)
	{
		return [=](float t, const CutsceneEntity*) -> float {
			float period = (l_freq != 0.f) ? 1.f / l_freq : 1.f;
			float phase = std::fmod(t, period) / period;
			float tri = (phase < 0.5f) ? (4.f * phase - 1.f) : (3.f - 4.f * phase);
			return tri * l_amplitude + l_offset;
		};
	}

	inline ExprFunc Sawtooth(float l_freq, float l_amplitude, float l_offset = 0.f)
	{
		return [=](float t, const CutsceneEntity*) -> float {
			float period = (l_freq != 0.f) ? 1.f / l_freq : 1.f;
			float phase = std::fmod(t, period) / period;
			return (2.f * phase - 1.f) * l_amplitude + l_offset;
		};
	}

	inline ExprFunc Breathing(float l_baseValue, float l_amplitude, float l_period)
	{
		return [=](float t, const CutsceneEntity*) -> float {
			float freq = (l_period != 0.f) ? 2.f * 3.14159265f / l_period : 0.f;
			return l_baseValue + std::sin(t * freq) * l_amplitude;
		};
	}

	// ── Combinators ──────────────────────────────────────────────

	inline ExprFunc Add(ExprFunc l_a, ExprFunc l_b)
	{
		return [a = std::move(l_a), b = std::move(l_b)](float t, const CutsceneEntity* e) -> float {
			return a(t, e) + b(t, e);
		};
	}

	inline ExprFunc Multiply(ExprFunc l_a, ExprFunc l_b)
	{
		return [a = std::move(l_a), b = std::move(l_b)](float t, const CutsceneEntity* e) -> float {
			return a(t, e) * b(t, e);
		};
	}

	inline ExprFunc Clamp(ExprFunc l_inner, float l_min, float l_max)
	{
		return [inner = std::move(l_inner), l_min, l_max](float t, const CutsceneEntity* e) -> float {
			return std::max(l_min, std::min(l_max, inner(t, e)));
		};
	}

	inline ExprFunc Constant(float l_value)
	{
		return [=](float, const CutsceneEntity*) -> float { return l_value; };
	}
}

class ExpressionAction : public CutsceneAction
{
public:
	ExpressionAction(const std::string& l_entityName, AnimProperty l_prop,
					 ExprFunc l_expression);
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
	void Skip() override;

private:
	std::string m_entityName;
	AnimProperty m_prop;
	ExprFunc m_expression;
};

namespace ExpressionAnim
{
	CutsceneActionPtr Create(const std::string& l_name, AnimProperty l_prop,
							 ExprFunc l_expression);
}
