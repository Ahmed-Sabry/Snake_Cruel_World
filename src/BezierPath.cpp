#include "BezierPath.h"
#include "CutsceneState.h"
#include <cmath>
#include <algorithm>

namespace { constexpr float kPi = 3.14159265358979323846f; }

// ── BezierPath ───────────────────────────────────────────────────

void BezierPath::AddCubic(sf::Vector2f l_p0, sf::Vector2f l_c1,
						  sf::Vector2f l_c2, sf::Vector2f l_p3)
{
	m_segments.push_back({l_p0, l_c1, l_c2, l_p3});
}

void BezierPath::AddQuadratic(sf::Vector2f l_p0, sf::Vector2f l_control, sf::Vector2f l_p2)
{
	// Convert quadratic to cubic: C1 = P0 + 2/3*(Q - P0), C2 = P2 + 2/3*(Q - P2)
	sf::Vector2f c1 = l_p0 + (2.f / 3.f) * (l_control - l_p0);
	sf::Vector2f c2 = l_p2 + (2.f / 3.f) * (l_control - l_p2);
	m_segments.push_back({l_p0, c1, c2, l_p2});
}

bool BezierPath::MapToSegment(float l_t, int& l_seg, float& l_localT) const
{
	if (m_segments.empty())
		return false;
	l_t = std::max(0.f, std::min(1.f, l_t));
	float scaledT = l_t * (float)m_segments.size();
	l_seg = std::min((int)scaledT, (int)m_segments.size() - 1);
	l_localT = std::max(0.f, std::min(1.f, scaledT - (float)l_seg));
	return true;
}

sf::Vector2f BezierPath::Evaluate(float l_t) const
{
	int seg; float localT;
	if (!MapToSegment(l_t, seg, localT))
		return {0.f, 0.f};
	return EvalCubic(m_segments[seg], localT);
}

sf::Vector2f BezierPath::EvaluateTangent(float l_t) const
{
	int seg; float localT;
	if (!MapToSegment(l_t, seg, localT))
		return {1.f, 0.f};
	return EvalCubicTangent(m_segments[seg], localT);
}

sf::Vector2f BezierPath::EvalCubic(const BezierSegment& seg, float t)
{
	float u = 1.f - t;
	float uu = u * u;
	float uuu = uu * u;
	float tt = t * t;
	float ttt = tt * t;

	return uuu * seg.p0
		 + 3.f * uu * t * seg.p1
		 + 3.f * u * tt * seg.p2
		 + ttt * seg.p3;
}

sf::Vector2f BezierPath::EvalCubicTangent(const BezierSegment& seg, float t)
{
	float u = 1.f - t;
	// B'(t) = 3(1-t)²(P1-P0) + 6(1-t)t(P2-P1) + 3t²(P3-P2)
	return 3.f * u * u * (seg.p1 - seg.p0)
		 + 6.f * u * t * (seg.p2 - seg.p1)
		 + 3.f * t * t * (seg.p3 - seg.p2);
}

// ── BezierMoveAction ─────────────────────────────────────────────

BezierMoveAction::BezierMoveAction(const std::string& l_entityName, BezierPath l_path,
								   float l_duration, EasingFunc l_easing,
								   bool l_orientToPath)
	: m_entityName(l_entityName), m_path(std::move(l_path)),
	  m_duration(l_duration), m_easing(l_easing), m_orientToPath(l_orientToPath)
{
}

void BezierMoveAction::ApplyPathState(float l_t)
{
	if (!CutsceneState::s_active || m_path.IsEmpty())
		return;

	CutsceneEntity* entity = CutsceneState::s_active->GetScene().Get(m_entityName);
	if (!entity)
		return;

	entity->position = m_path.Evaluate(l_t);
	if (m_orientToPath)
	{
		sf::Vector2f tangent = m_path.EvaluateTangent(l_t);
		float len = std::sqrt(tangent.x * tangent.x + tangent.y * tangent.y);
		if (len > 0.001f)
			entity->rotation = std::atan2(tangent.y, tangent.x) * 180.f / kPi;
	}
}

void BezierMoveAction::Start(StateManager& l_sm)
{
	(void)l_sm;
	m_elapsed = 0.f;
	if (m_duration <= 0.f)
	{
		m_elapsed = m_duration;
		ApplyPathState(1.f);
	}
	else
	{
		ApplyPathState(0.f);
	}
}

bool BezierMoveAction::Update(float l_dt, StateManager& l_sm)
{
	(void)l_sm;
	if (m_duration <= 0.f || m_path.IsEmpty())
		return true;

	m_elapsed += l_dt;
	float t = std::min(1.f, m_elapsed / m_duration);
	ApplyPathState(m_easing(t));
	return m_elapsed >= m_duration;
}

void BezierMoveAction::Skip()
{
	m_elapsed = m_duration;
	ApplyPathState(1.f);
}

// ── Convenience Factory ──────────────────────────────────────────

CutsceneActionPtr BezierMove::Create(const std::string& l_name, BezierPath l_path,
									 float l_duration, EasingFunc l_easing,
									 bool l_orientToPath)
{
	return std::make_unique<BezierMoveAction>(l_name, std::move(l_path),
											  l_duration, l_easing, l_orientToPath);
}
