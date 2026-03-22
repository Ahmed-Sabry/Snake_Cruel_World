#pragma once

#include "CutsceneAction.h"
#include "CutsceneActions.h"
#include "Easing.h"
#include <vector>
#include <string>

struct BezierSegment
{
	sf::Vector2f p0, p1, p2, p3; // cubic bezier: start, control1, control2, end
};

class BezierPath
{
public:
	void AddCubic(sf::Vector2f l_p0, sf::Vector2f l_c1, sf::Vector2f l_c2, sf::Vector2f l_p3);
	void AddQuadratic(sf::Vector2f l_p0, sf::Vector2f l_control, sf::Vector2f l_p2);

	sf::Vector2f Evaluate(float l_t) const;
	sf::Vector2f EvaluateTangent(float l_t) const;

	int SegmentCount() const { return (int)m_segments.size(); }
	bool IsEmpty() const { return m_segments.empty(); }

private:
	bool MapToSegment(float l_t, int& l_seg, float& l_localT) const;

	std::vector<BezierSegment> m_segments;

	static sf::Vector2f EvalCubic(const BezierSegment& seg, float t);
	static sf::Vector2f EvalCubicTangent(const BezierSegment& seg, float t);
};

class BezierMoveAction : public CutsceneAction
{
public:
	BezierMoveAction(const std::string& l_entityName, BezierPath l_path,
					 float l_duration, EasingFunc l_easing = Easing::EaseInOutCubic,
					 bool l_orientToPath = false);
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
	void Skip() override;

private:
	void ApplyPathState(float l_t);

	std::string m_entityName;
	BezierPath m_path;
	float m_duration;
	EasingFunc m_easing;
	bool m_orientToPath;
	float m_elapsed = 0.f;
};

namespace BezierMove
{
	CutsceneActionPtr Create(const std::string& l_name, BezierPath l_path,
							 float l_duration, EasingFunc l_easing = Easing::EaseInOutCubic,
							 bool l_orientToPath = false);
}
