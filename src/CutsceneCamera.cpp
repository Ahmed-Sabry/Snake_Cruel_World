#include "CutsceneCamera.h"
#include "CutsceneScene.h"
#include <cmath>

CutsceneCamera::CutsceneCamera()
{
}

void CutsceneCamera::Init(sf::Vector2u l_windowSize)
{
	m_windowSize = l_windowSize;
	position = sf::Vector2f(l_windowSize.x * 0.5f, l_windowSize.y * 0.5f);
	zoom = 1.0f;
	rotation = 0.0f;
	m_following = false;
	m_followTarget.clear();

	m_defaultView.setSize((float)l_windowSize.x, (float)l_windowSize.y);
	m_defaultView.setCenter(l_windowSize.x * 0.5f, l_windowSize.y * 0.5f);
}

void CutsceneCamera::Update(float l_dt, const CutsceneScene& l_scene)
{
	// Follow entity (smooth lerp toward target's world position)
	if (m_following)
	{
		const CutsceneEntity* target = l_scene.Get(m_followTarget);
		if (target)
		{
			// Use world position to account for parent transforms
			sf::Vector2f worldPos = target->position;
			if (!target->parent.empty())
			{
				sf::Transform wt = l_scene.GetWorldTransform(m_followTarget);
				worldPos = wt.transformPoint(target->position);
			}
			float factor = 1.f - std::exp(-m_followSmoothing * l_dt);
			position.x += (worldPos.x - position.x) * factor;
			position.y += (worldPos.y - position.y) * factor;
		}
	}
}

void CutsceneCamera::Apply(sf::RenderTarget& l_target)
{
	// Save current view so we can restore it later
	m_savedView = l_target.getView();

	sf::View cameraView;
	float safeZoom = std::max(zoom, 0.001f);
	cameraView.setSize((float)m_windowSize.x * safeZoom, (float)m_windowSize.y * safeZoom);
	cameraView.setCenter(position);
	cameraView.setRotation(rotation);

	l_target.setView(cameraView);
}

void CutsceneCamera::Reset(sf::RenderTarget& l_target)
{
	l_target.setView(m_savedView);
}

void CutsceneCamera::FollowEntity(const std::string& l_entityName, float l_smoothing)
{
	m_followTarget = l_entityName;
	m_followSmoothing = l_smoothing;
	m_following = true;
}

void CutsceneCamera::StopFollowing()
{
	m_following = false;
	m_followTarget.clear();
}
