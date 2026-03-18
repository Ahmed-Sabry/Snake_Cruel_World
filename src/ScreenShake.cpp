#include "ScreenShake.h"
#include <cmath>
#include <random>

static std::mt19937& GetRNG()
{
	static std::mt19937 rng(std::random_device{}());
	return rng;
}

ScreenShake::ScreenShake()
	: m_timer(0.0f),
	  m_intensity(0.0f)
{
}

void ScreenShake::Trigger(float l_duration, float l_intensity)
{
	if (l_duration > m_timer)
		m_timer = l_duration;
	if (l_intensity > m_intensity)
		m_intensity = l_intensity;
}

void ScreenShake::Update(float l_dt, Window& l_window)
{
	if (m_timer <= 0.0f)
		return;

	m_timer -= l_dt;

	std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
	float offsetX = dist(GetRNG()) * m_intensity;
	float offsetY = dist(GetRNG()) * m_intensity;

	sf::View view = l_window.GetDefaultView();
	view.move(offsetX, offsetY);
	l_window.SetView(view);

	// Frame-rate independent dampening
	m_intensity *= std::pow(0.9f, l_dt * 60.0f);

	if (m_timer <= 0.0f)
	{
		m_timer = 0.0f;
		m_intensity = 0.0f;
		l_window.SetView(l_window.GetDefaultView());
	}
}

void ScreenShake::Reset(Window& l_window)
{
	m_timer = 0.0f;
	m_intensity = 0.0f;
	l_window.SetView(l_window.GetDefaultView());
}

bool ScreenShake::IsActive() const
{
	return m_timer > 0.0f;
}
