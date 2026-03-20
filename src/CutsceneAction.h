#pragma once

#include "Platform/Platform.hpp"
#include <memory>

class StateManager;

class CutsceneAction
{
public:
	virtual ~CutsceneAction() = default;

	virtual void Start(StateManager& l_sm) { (void)l_sm; }
	virtual bool Update(float l_dt, StateManager& l_sm) = 0;
	virtual void Render(sf::RenderTarget& l_target) { (void)l_target; }
	virtual bool IsPersistent() const { return false; }
	virtual void Skip() {}
};

using CutsceneActionPtr = std::unique_ptr<CutsceneAction>;
