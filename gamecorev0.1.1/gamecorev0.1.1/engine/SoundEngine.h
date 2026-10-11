#ifndef GAMECORE_SOUNDENGINE_H
#define GAMECORE_SOUNDENGINE_H

#include <string>
#include <vector>

struct ma_engine;
struct ma_sound;

namespace gamecore
{
	class SoundEngine;

	class Sound
	{
	public:
		void setIsPaused(bool paused = true);
		bool getIsPaused() { return paused_; }
		void setVolume(float volume);
		float getVolume() { return volume_; }
		void stop();
		bool isFinished();
		bool drop() { released_ = true; return true; }

	private:
		friend class SoundEngine;
		Sound(const std::string& file, ma_sound* sound, bool tracked);
		~Sound();

		std::string file_;
		ma_sound* sound_;
		bool tracked_;
		bool released_;
		bool paused_;
		float volume_;
	};

	class SoundEngine
	{
	public:
		Sound* play2D(const char* file, bool looped = false, bool startPaused = false, bool track = false);
		bool isCurrentlyPlaying(const char* file);
		void removeSoundSource(const char* file);
		void stopAllSounds();
		void setSoundVolume(float volume);
		bool drop() { delete this; return true; }

	private:
		friend SoundEngine* createSoundEngine();
		SoundEngine();
		~SoundEngine();
		void removeFinished();

		ma_engine* engine_;
		std::vector<Sound*> sounds_;
		std::vector<std::string> missing_;
	};

	SoundEngine* createSoundEngine();
}

#endif
