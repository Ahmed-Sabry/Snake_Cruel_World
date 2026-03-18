#include "AudioManager.h"
#include <iostream>
#include <cmath>
#include <cstdlib>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

AudioManager::AudioManager()
	: m_nextSound(0),
	  m_masterVolume(100.0f),
	  m_sfxVolume(100.0f),
	  m_musicVolume(70.0f)
{
}

void AudioManager::LoadSound(const std::string& l_name, const std::string& l_filepath)
{
	sf::SoundBuffer buffer;
	if (!buffer.loadFromFile(l_filepath))
	{
		// Silently skip — GenerateDefaultSounds() will fill in missing ones
		return;
	}
	m_buffers[l_name] = buffer;
}

void AudioManager::PlaySound(const std::string& l_name)
{
	auto it = m_buffers.find(l_name);
	if (it == m_buffers.end())
		return;

	sf::Sound& sound = m_sounds[m_nextSound];
	sound.setBuffer(it->second);
	sound.setVolume(m_masterVolume * m_sfxVolume / 100.0f);
	sound.play();

	m_nextSound = (m_nextSound + 1) % MAX_SOUNDS;
}

void AudioManager::PlayMusic(const std::string& l_filepath, bool l_loop)
{
	if (!m_music.openFromFile(l_filepath))
	{
		std::cerr << "AudioManager: Could not load music from " << l_filepath << std::endl;
		return;
	}

	m_music.setLoop(l_loop);
	m_music.setVolume(m_masterVolume * m_musicVolume / 100.0f);
	m_music.play();
}

void AudioManager::StopMusic()
{
	m_music.stop();
}

void AudioManager::PauseMusic()
{
	m_music.pause();
}

void AudioManager::ResumeMusic()
{
	if (m_music.getStatus() == sf::Music::Paused)
		m_music.play();
}

void AudioManager::SetMasterVolume(float l_vol)
{
	m_masterVolume = l_vol;
	ApplyVolumes();
}

void AudioManager::SetSFXVolume(float l_vol)
{
	m_sfxVolume = l_vol;
	ApplyVolumes();
}

void AudioManager::SetMusicVolume(float l_vol)
{
	m_musicVolume = l_vol;
	ApplyVolumes();
}

void AudioManager::ApplyVolumes()
{
	m_music.setVolume(m_masterVolume * m_musicVolume / 100.0f);
}

void AudioManager::StoreSamples(const std::string& l_name, const std::vector<sf::Int16>& l_samples,
								unsigned l_sampleRate)
{
	sf::SoundBuffer buffer;
	if (buffer.loadFromSamples(l_samples.data(), l_samples.size(), 1, l_sampleRate))
		m_buffers[l_name] = buffer;
}

void AudioManager::GenerateDefaultSounds()
{
	const unsigned RATE = 44100;

	// Helper: sine wave sample
	auto sine = [](float t, float freq) -> float {
		return std::sin(2.0f * (float)M_PI * freq * t);
	};

	// Helper: linear envelope (1.0 → 0.0 over duration)
	auto decay = [](float t, float duration) -> float {
		return 1.0f - t / duration;
	};

	// Helper: exponential decay
	auto expDecay = [](float t, float rate) -> float {
		return std::exp(-rate * t);
	};

	// --- apple_eat: rising chirp (80ms) ---
	if (m_buffers.find("apple_eat") == m_buffers.end())
	{
		unsigned len = (unsigned)(RATE * 0.08f);
		std::vector<sf::Int16> samples(len);
		for (unsigned i = 0; i < len; i++)
		{
			float t = (float)i / RATE;
			float env = decay(t, 0.08f);
			float freq = 700.0f + t * 6000.0f;
			samples[i] = (sf::Int16)(env * env * 18000.0f * sine(t, freq));
		}
		StoreSamples("apple_eat", samples, RATE);
	}

	// --- self_collide: buzzy crunch (120ms) ---
	if (m_buffers.find("self_collide") == m_buffers.end())
	{
		unsigned len = (unsigned)(RATE * 0.12f);
		std::vector<sf::Int16> samples(len);
		for (unsigned i = 0; i < len; i++)
		{
			float t = (float)i / RATE;
			float env = expDecay(t, 25.0f);
			float noise = ((rand() % 20000) - 10000) / 10000.0f;
			float tone = sine(t, 150.0f) + 0.5f * sine(t, 230.0f);
			samples[i] = (sf::Int16)(env * 14000.0f * (0.5f * tone + 0.5f * noise));
		}
		StoreSamples("self_collide", samples, RATE);
	}

	// --- wall_death: heavy thud with crumble (300ms) ---
	if (m_buffers.find("wall_death") == m_buffers.end())
	{
		unsigned len = (unsigned)(RATE * 0.3f);
		std::vector<sf::Int16> samples(len);
		for (unsigned i = 0; i < len; i++)
		{
			float t = (float)i / RATE;
			float env = expDecay(t, 8.0f);
			float thud = sine(t, 60.0f + 40.0f * expDecay(t, 15.0f));
			float noise = ((rand() % 20000) - 10000) / 10000.0f;
			float crackle = noise * expDecay(t, 12.0f);
			samples[i] = (sf::Int16)(20000.0f * (env * thud * 0.7f + crackle * 0.3f));
		}
		StoreSamples("wall_death", samples, RATE);
	}

	// --- world_shrink: low rumble with grind (400ms) ---
	if (m_buffers.find("world_shrink") == m_buffers.end())
	{
		unsigned len = (unsigned)(RATE * 0.4f);
		std::vector<sf::Int16> samples(len);
		for (unsigned i = 0; i < len; i++)
		{
			float t = (float)i / RATE;
			float env = (t < 0.05f) ? (t / 0.05f) : decay(t - 0.05f, 0.35f);
			float rumble = sine(t, 45.0f) + 0.6f * sine(t, 70.0f);
			float grind = ((rand() % 20000) - 10000) / 10000.0f * 0.3f;
			samples[i] = (sf::Int16)(env * 16000.0f * (rumble * 0.7f + grind));
		}
		StoreSamples("world_shrink", samples, RATE);
	}

	// --- combo_3x: quick ascending three-note arpeggio (250ms) ---
	if (m_buffers.find("combo_3x") == m_buffers.end())
	{
		unsigned len = (unsigned)(RATE * 0.25f);
		std::vector<sf::Int16> samples(len);
		float notes[] = { 523.25f, 659.25f, 783.99f }; // C5, E5, G5
		float noteDur = 0.08f;
		for (unsigned i = 0; i < len; i++)
		{
			float t = (float)i / RATE;
			int noteIdx = std::min((int)(t / noteDur), 2);
			float noteT = t - noteIdx * noteDur;
			float env = expDecay(noteT, 8.0f);
			samples[i] = (sf::Int16)(env * 14000.0f * sine(t, notes[noteIdx]));
		}
		StoreSamples("combo_3x", samples, RATE);
	}

	// --- level_complete: ascending fanfare (600ms) ---
	if (m_buffers.find("level_complete") == m_buffers.end())
	{
		unsigned len = (unsigned)(RATE * 0.6f);
		std::vector<sf::Int16> samples(len);
		float notes[] = { 523.25f, 587.33f, 659.25f, 783.99f }; // C5, D5, E5, G5
		float noteDur = 0.14f;
		for (unsigned i = 0; i < len; i++)
		{
			float t = (float)i / RATE;
			int noteIdx = std::min((int)(t / noteDur), 3);
			float noteT = t - noteIdx * noteDur;
			float env = expDecay(noteT, 4.0f);
			float val = sine(t, notes[noteIdx]) + 0.3f * sine(t, notes[noteIdx] * 2.0f);
			samples[i] = (sf::Int16)(env * 13000.0f * val);
		}
		StoreSamples("level_complete", samples, RATE);
	}

	// --- menu_navigate: soft tick (30ms) ---
	if (m_buffers.find("menu_navigate") == m_buffers.end())
	{
		unsigned len = (unsigned)(RATE * 0.03f);
		std::vector<sf::Int16> samples(len);
		for (unsigned i = 0; i < len; i++)
		{
			float t = (float)i / RATE;
			float env = expDecay(t, 80.0f);
			samples[i] = (sf::Int16)(env * 12000.0f * sine(t, 1200.0f));
		}
		StoreSamples("menu_navigate", samples, RATE);
	}

	// --- menu_select: confirm blip (60ms) ---
	if (m_buffers.find("menu_select") == m_buffers.end())
	{
		unsigned len = (unsigned)(RATE * 0.06f);
		std::vector<sf::Int16> samples(len);
		for (unsigned i = 0; i < len; i++)
		{
			float t = (float)i / RATE;
			float env = expDecay(t, 30.0f);
			float val = sine(t, 800.0f) + 0.5f * sine(t, 1200.0f);
			samples[i] = (sf::Int16)(env * 12000.0f * val);
		}
		StoreSamples("menu_select", samples, RATE);
	}
}
