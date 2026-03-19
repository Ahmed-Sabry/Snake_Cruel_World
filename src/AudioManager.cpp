#include "AudioManager.h"
#include "RandomUtils.h"
#include <iostream>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static float RandomNoise()
{
	return RandomFloat(-1.0f, 1.0f);
}

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
	if (l_filepath == m_currentMusicPath &&
		m_music.getStatus() == sf::Music::Playing)
		return;

	if (!m_music.openFromFile(l_filepath))
	{
		std::cerr << "AudioManager: Could not load music from " << l_filepath << std::endl;
		m_currentMusicPath.clear();
		return;
	}

	m_currentMusicPath = l_filepath;
	m_music.setLoop(l_loop);
	m_music.setVolume(m_masterVolume * m_musicVolume / 100.0f);
	m_music.play();
}

void AudioManager::StopMusic()
{
	m_music.stop();
	m_currentMusicPath.clear();
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
	m_masterVolume = std::clamp(l_vol, 0.0f, 100.0f);
	ApplyMusicVolume();
	ApplySFXVolume();
}

void AudioManager::SetSFXVolume(float l_vol)
{
	m_sfxVolume = std::clamp(l_vol, 0.0f, 100.0f);
	ApplySFXVolume();
}

void AudioManager::SetMusicVolume(float l_vol)
{
	m_musicVolume = std::clamp(l_vol, 0.0f, 100.0f);
	ApplyMusicVolume();
}

void AudioManager::ApplyMusicVolume()
{
	m_music.setVolume(m_masterVolume * m_musicVolume / 100.0f);
}

void AudioManager::ApplySFXVolume()
{
	float vol = m_masterVolume * m_sfxVolume / 100.0f;
	for (int i = 0; i < MAX_SOUNDS; i++)
	{
		if (m_sounds[i].getStatus() == sf::Sound::Playing)
			m_sounds[i].setVolume(vol);
	}
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
			float noise = RandomNoise();
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
			float noise = RandomNoise();
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
			float grind = RandomNoise() * 0.3f;
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

	// --- blackout_on: low hum when lights go out (300ms) ---
	if (m_buffers.find("blackout_on") == m_buffers.end())
	{
		unsigned len = (unsigned)(RATE * 0.3f);
		std::vector<sf::Int16> samples(len);
		for (unsigned i = 0; i < len; i++)
		{
			float t = (float)i / RATE;
			float env = expDecay(t, 5.0f);
			float val = sine(t, 80.0f) + 0.3f * sine(t, 120.0f);
			samples[i] = (sf::Int16)(env * 10000.0f * val);
		}
		StoreSamples("blackout_on", samples, RATE);
	}

	// --- mirror_flip: descending shimmer sweep (200ms) ---
	if (m_buffers.find("mirror_flip") == m_buffers.end())
	{
		unsigned len = (unsigned)(RATE * 0.2f);
		std::vector<sf::Int16> samples(len);
		for (unsigned i = 0; i < len; i++)
		{
			float t = (float)i / RATE;
			float env = expDecay(t, 10.0f);
			float freq = 2000.0f - t * 8000.0f;
			if (freq < 200.0f) freq = 200.0f;
			samples[i] = (sf::Int16)(env * 10000.0f * sine(t, freq));
		}
		StoreSamples("mirror_flip", samples, RATE);
	}

	// --- earthquake: deep rumble with stone grinding (800ms) ---
	if (m_buffers.find("earthquake") == m_buffers.end())
	{
		unsigned len = (unsigned)(RATE * 0.8f);
		std::vector<sf::Int16> samples(len);
		for (unsigned i = 0; i < len; i++)
		{
			float t = (float)i / RATE;
			float env = (t < 0.1f) ? (t / 0.1f) : expDecay(t - 0.1f, 3.0f);
			float rumble = sine(t, 35.0f) + 0.7f * sine(t, 55.0f) + 0.3f * sine(t, 80.0f);
			float grind = RandomNoise() * 0.4f * expDecay(t, 4.0f);
			samples[i] = (sf::Int16)(env * 18000.0f * (rumble * 0.6f + grind));
		}
		StoreSamples("earthquake", samples, RATE);
	}

	// --- apple_miss: descending disappointment tone (200ms) ---
	if (m_buffers.find("apple_miss") == m_buffers.end())
	{
		unsigned len = (unsigned)(RATE * 0.2f);
		std::vector<sf::Int16> samples(len);
		for (unsigned i = 0; i < len; i++)
		{
			float t = (float)i / RATE;
			float env = expDecay(t, 8.0f);
			float freq = 400.0f - t * 1500.0f;
			if (freq < 100.0f) freq = 100.0f;
			samples[i] = (sf::Int16)(env * 14000.0f * sine(t, freq));
		}
		StoreSamples("apple_miss", samples, RATE);
	}
}
