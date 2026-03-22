#include "ExpressionAction.h"
#include "CutsceneState.h"
#include "CutsceneCamera.h"
#include <algorithm>
#include <cmath>

// ── ExpressionAction ─────────────────────────────────────────────

ExpressionAction::ExpressionAction(const std::string& l_entityName, AnimProperty l_prop,
								   ExprFunc l_expression)
	: m_entityName(l_entityName), m_prop(l_prop), m_expression(std::move(l_expression))
{
}

void ExpressionAction::Start(StateManager& l_sm)
{
	(void)l_sm;
	if (!CutsceneState::s_active)
		return;

	CutsceneEntity* entity = CutsceneState::s_active->GetScene().Get(m_entityName);
	if (!entity)
		return;

	// Capture copies for the callback lambda
	ExprFunc expr = m_expression;
	AnimProperty prop = m_prop;
	std::string entityName = m_entityName;

	entity->updateCallbacks.push_back(
		[expr, prop, entityName](float /*dt*/, float totalTime)
		{
			if (!CutsceneState::s_active)
				return;
			CutsceneEntity* ent = CutsceneState::s_active->GetScene().Get(entityName);
			if (!ent)
				return;

			float value = expr(totalTime, ent);

			// Camera properties
			switch (prop)
			{
			case AnimProperty::CameraX:
				if (CutsceneState::s_active) CutsceneState::s_active->GetCamera().position.x = value;
				return;
			case AnimProperty::CameraY:
				if (CutsceneState::s_active) CutsceneState::s_active->GetCamera().position.y = value;
				return;
			case AnimProperty::CameraZoom:
				if (CutsceneState::s_active) CutsceneState::s_active->GetCamera().zoom = value;
				return;
			case AnimProperty::CameraRotation:
				if (CutsceneState::s_active) CutsceneState::s_active->GetCamera().rotation = value;
				return;
			default: break;
			}

			switch (prop)
			{
			case AnimProperty::PositionX: ent->position.x = value; break;
			case AnimProperty::PositionY: ent->position.y = value; break;
			case AnimProperty::ScaleX:    ent->scale.x = value; break;
			case AnimProperty::ScaleY:    ent->scale.y = value; break;
			case AnimProperty::Rotation:  ent->rotation = value; break;
			case AnimProperty::Alpha:     ent->alpha = value; break;
			case AnimProperty::ColorR:    ent->color.r = static_cast<sf::Uint8>(std::max(0.f, std::min(255.f, value))); break;
			case AnimProperty::ColorG:    ent->color.g = static_cast<sf::Uint8>(std::max(0.f, std::min(255.f, value))); break;
			case AnimProperty::ColorB:    ent->color.b = static_cast<sf::Uint8>(std::max(0.f, std::min(255.f, value))); break;
			case AnimProperty::Corruption: ent->corruption = value; break;
			default: break;
			}
		});
}

bool ExpressionAction::Update(float l_dt, StateManager& l_sm)
{
	(void)l_dt; (void)l_sm;
	// Fire-and-forget: callback registered, action completes immediately
	return true;
}

void ExpressionAction::Skip()
{
	// Nothing to skip — the callback is persistent on the entity
}

// ── Convenience Factory ──────────────────────────────────────────

CutsceneActionPtr ExpressionAnim::Create(const std::string& l_name, AnimProperty l_prop,
										 ExprFunc l_expression)
{
	return std::make_unique<ExpressionAction>(l_name, l_prop, std::move(l_expression));
}
