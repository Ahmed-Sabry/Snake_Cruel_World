#include "CutsceneTimeline.h"
#include "CutsceneActions.h"

void CutsceneTimeline::Add(CutsceneActionPtr l_action)
{
	m_steps.push_back(std::move(l_action));
}

void CutsceneTimeline::AddParallel(std::vector<CutsceneActionPtr> l_actions)
{
	m_steps.push_back(std::make_unique<ParallelAction>(std::move(l_actions)));
}

void CutsceneTimeline::Start(StateManager& l_sm)
{
	m_currentStep = 0;
	m_finished = false;
	if (!m_steps.empty())
		m_steps[0]->Start(l_sm);
	else
		m_finished = true;
}

bool CutsceneTimeline::Update(float l_dt, StateManager& l_sm)
{
	if (m_finished)
		return true;

	if (m_currentStep >= m_steps.size())
	{
		m_finished = true;
		return true;
	}

	bool stepDone = m_steps[m_currentStep]->Update(l_dt, l_sm);
	if (stepDone)
	{
		m_currentStep++;
		if (m_currentStep < m_steps.size())
			m_steps[m_currentStep]->Start(l_sm);
		else
			m_finished = true;
	}

	return m_finished;
}

void CutsceneTimeline::Render(sf::RenderTarget& l_target)
{
	// Render completed persistent actions (unless their persistence was cleared)
	for (size_t i = 0; i < m_currentStep && i < m_steps.size(); ++i)
	{
		if (m_steps[i]->IsPersistent() && !m_steps[i]->IsPersistenceCleared())
			m_steps[i]->Render(l_target);
	}

	// Render current action
	if (m_currentStep < m_steps.size())
		m_steps[m_currentStep]->Render(l_target);
}

void CutsceneTimeline::SkipCurrent()
{
	if (m_currentStep < m_steps.size())
		m_steps[m_currentStep]->Skip();
}

void CutsceneTimeline::ClearAllPersistent()
{
	for (size_t i = 0; i < m_currentStep && i < m_steps.size(); ++i)
		m_steps[i]->ClearPersistence();
}
