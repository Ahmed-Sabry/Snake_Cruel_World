#pragma once

#include "Platform/Platform.hpp"
#include <map>
#include <string>
#include <vector>

class AudioManager
{
public:
	AudioManager();
	~AudioManager() = default;

	void LoadSound(const std::string& l_name, const std::string& l_filepath);
	void PlaySound(const std::string& l_name);

	void PlayMusic(const std::string& l_filepath, bool l_loop = true);
	void StopMusic();
	void PauseMusic();
	void ResumeMusic();

	void SetMasterVolume(float l_vol);
	void SetSFXVolume(float l_vol);
	void SetMusicVolume(float l_vol);

	float GetMasterVolume() const { return m_masterVolume; }
	float GetSFXVolume() const { return m_sfxVolume; }
	float GetMusicVolume() const { return m_musicVolume; }

	// Generate built-in synthesized sounds for any names not loaded from files
	void GenerateDefaultSounds();

private:
	void ApplyMusicVolume();
	void ApplySFXVolume();
	void StoreSamples(const std::string& l_name, const std::vector<sf::Int16>& l_samples,
					  unsigned l_sampleRate);

	std::map<std::string, sf::SoundBuffer> m_buffers;

	static constexpr int MAX_SOUNDS = 8;
	sf::Sound m_sounds[MAX_SOUNDS];
	int m_nextSound;

	sf::Music m_music;
	std::string m_currentMusicPath;

	float m_masterVolume;
	float m_sfxVolume;
	float m_musicVolume;
};
