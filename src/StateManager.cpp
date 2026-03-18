#include "StateManager.h"

StateManager::StateManager(Window& l_window, AudioManager& l_audio)
	: m_window(l_window), m_audio(l_audio)
{
}

StateManager::~StateManager()
{
}

void StateManager::Update(float l_dt)
{
	if (m_stateStack.empty())
		return;

	m_isUpdating = true;
	m_stateStack.back().second->Update(l_dt);
	m_isUpdating = false;

	ProcessPendingTransitions();
}

void StateManager::HandleInput()
{
	if (m_stateStack.empty())
		return;

	m_isUpdating = true;
	m_stateStack.back().second->HandleInput();
	m_isUpdating = false;

	ProcessPendingTransitions();
}

void StateManager::Render()
{
	if (m_stateStack.empty())
		return;

	m_window.Clear();

	m_isUpdating = true;
	// Render all states in the stack (bottom to top) so overlays work
	for (auto& pair : m_stateStack)
	{
		pair.second->Render();
	}
	m_isUpdating = false;

	ProcessPendingTransitions();

	m_window.Display();
}

void StateManager::SwitchTo(StateType l_type)
{
	if (m_isUpdating)
	{
		// Defer: the current state is still executing, can't destroy it yet
		m_pendingTransitions.push_back({ TransitionType::Switch, l_type });
		return;
	}
	ExecuteSwitchTo(l_type);
}

void StateManager::PushState(StateType l_type)
{
	if (m_isUpdating)
	{
		m_pendingTransitions.push_back({ TransitionType::Push, l_type });
		return;
	}
	ExecutePushState(l_type);
}

void StateManager::PopState()
{
	if (m_isUpdating)
	{
		m_pendingTransitions.push_back({ TransitionType::Pop, StateType::MainMenu /*unused*/ });
		return;
	}
	ExecutePopState();
}

void StateManager::ProcessPendingTransitions()
{
	while (!m_pendingTransitions.empty())
	{
		auto transition = m_pendingTransitions.front();
		m_pendingTransitions.erase(m_pendingTransitions.begin());

		switch (transition.action)
		{
			case TransitionType::Switch:
				ExecuteSwitchTo(transition.targetState);
				break;
			case TransitionType::Push:
				ExecutePushState(transition.targetState);
				break;
			case TransitionType::Pop:
				ExecutePopState();
				break;
			default:
				break;
		}
	}
}

Window& StateManager::GetWindow()
{
	return m_window;
}

AudioManager& StateManager::GetAudio()
{
	return m_audio;
}

void StateManager::ExecuteSwitchTo(StateType l_type)
{
	// Create the new state first to avoid clearing the stack on failure
	auto state = CreateState(l_type);
	if (!state)
		return;

	m_isUpdating = true;
	while (!m_stateStack.empty())
	{
		m_stateStack.back().second->OnExit();
		m_stateStack.pop_back();
	}
	m_isUpdating = false;
	ProcessPendingTransitions();

	m_stateStack.push_back({ l_type, std::move(state) });
	m_isUpdating = true;
	m_stateStack.back().second->OnEnter();
	m_isUpdating = false;
	ProcessPendingTransitions();
}

void StateManager::ExecutePushState(StateType l_type)
{
	auto state = CreateState(l_type);
	if (state)
	{
		m_stateStack.push_back({ l_type, std::move(state) });
		m_isUpdating = true;
		m_stateStack.back().second->OnEnter();
		m_isUpdating = false;
		ProcessPendingTransitions();
	}
}

void StateManager::ExecutePopState()
{
	if (m_stateStack.empty())
		return;
	m_isUpdating = true;
	m_stateStack.back().second->OnExit();
	m_isUpdating = false;
	m_stateStack.pop_back();
	ProcessPendingTransitions();
}

std::unique_ptr<BaseState> StateManager::CreateState(StateType l_type)
{
	auto it = m_stateFactory.find(l_type);
	if (it != m_stateFactory.end())
	{
		return it->second();
	}
	return nullptr;
}
