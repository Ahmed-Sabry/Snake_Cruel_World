#pragma once

#include "Platform/Platform.hpp"
#include <string>

class CutsceneScene;

class CutsceneCamera
{
public:
	CutsceneCamera();

	void Init(sf::Vector2u l_windowSize);
	void Update(float l_dt, const CutsceneScene& l_scene);
	void Apply(sf::RenderTarget& l_target);
	void Reset(sf::RenderTarget& l_target);

	// Camera properties (animatable via AnimProperty::CameraX/Y/Zoom/Rotation)
	sf::Vector2f position;
	float zoom = 1.0f;
	float rotation = 0.0f;

	// Follow mode
	void FollowEntity(const std::string& l_entityName, float l_smoothing = 5.0f);
	void StopFollowing();
	bool IsFollowing() const { return m_following; }

private:
	sf::Vector2u m_windowSize;
	sf::View m_defaultView;
	sf::View m_savedView;
	bool m_following = false;
	std::string m_followTarget;
	float m_followSmoothing = 5.0f;
};
