#pragma once

#include "Window.h"

class ScreenShake
{
public:
	ScreenShake();

	void Trigger(float l_duration, float l_intensity);
	void Update(float l_dt, Window& l_window);
	void Reset(Window& l_window);
	bool IsActive() const;

private:
	float m_timer;
	float m_intensity;
};
