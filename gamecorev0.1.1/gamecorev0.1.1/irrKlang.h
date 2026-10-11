#ifndef IRRKLANG_MINIAUDIO_H
#define IRRKLANG_MINIAUDIO_H

#include <string>
#include <vector>

struct ma_engine;
struct ma_sound;

namespace irrklang
{
	typedef float ik_f32;

	class ISoundEngine;

	class ISound
	{
	public:
		void setIsPaused(bool paused = true);
		bool getIsPaused() { return paused_; }
		void setVolume(ik_f32 volume);
		ik_f32 getVolume() { return volume_; }
		void stop();
		bool isFinished();
		bool drop() { released_ = true; return true; }

	private:
		friend class ISoundEngine;
		ISound(const std::string& file, ma_sound* sound, bool tracked);
		~ISound();

		std::string file_;
		ma_sound* sound_;
		bool tracked_;
		bool released_;
		bool paused_;
		ik_f32 volume_;
	};

	class ISoundEngine
	{
	public:
		ISound* play2D(const char* file, bool looped = false, bool startPaused = false, bool track = false);
		bool isCurrentlyPlaying(const char* file);
		void removeSoundSource(const char* file);
		void stopAllSounds();
		void setSoundVolume(ik_f32 volume);
		bool drop() { delete this; return true; }

	private:
		friend ISoundEngine* createIrrKlangDevice();
		ISoundEngine();
		~ISoundEngine();
		void removeFinished();

		ma_engine* engine_;
		std::vector<ISound*> sounds_;
		std::vector<std::string> missing_;
	};

	ISoundEngine* createIrrKlangDevice();
}

#endif
