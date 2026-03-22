#pragma once

#include "CutsceneAction.h"
#include "Easing.h"
#include <string>

enum class TransitionType
{
	Fade,
	WipeLeft, WipeRight, WipeUp, WipeDown,
	IrisOpen, IrisClose,
	Dissolve
};

class TransitionAction : public CutsceneAction
{
public:
	TransitionAction(TransitionType l_type, float l_duration,
					 sf::Color l_color = sf::Color::Black,
					 EasingFunc l_easing = Easing::EaseInOutCubic);
	void Start(StateManager& l_sm) override;
	bool Update(float l_dt, StateManager& l_sm) override;
	void Render(sf::RenderTarget& l_target) override;
	bool IsPersistent() const override { return true; }
	void Skip() override;

private:
	TransitionType m_type;
	float m_duration;
	sf::Color m_color;
	EasingFunc m_easing;
	float m_elapsed = 0.f;
	float m_progress = 0.f;

	sf::RenderTexture m_snapshot;
	bool m_snapshotValid = false;

	sf::Shader m_wipeShader;
	sf::Shader m_irisShader;
	sf::Shader m_dissolveShader;
	bool m_wipeLoaded = false;
	bool m_irisLoaded = false;
	bool m_dissolveLoaded = false;

	void RenderFade(sf::RenderTarget& l_target);
	void RenderShader(sf::RenderTarget& l_target);
};

namespace Transition
{
	CutsceneActionPtr Create(TransitionType l_type, float l_duration,
							 sf::Color l_color = sf::Color::Black,
							 EasingFunc l_easing = Easing::EaseInOutCubic);

	TransitionType FromName(const std::string& l_name);
}
