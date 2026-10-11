#include "SoundEngine.h"
#include <miniaudio.h>
#include <algorithm>
#include <cstdio>

namespace gamecore
{
	Sound::Sound(const std::string& file, ma_sound* sound, bool tracked)
		: file_(file), sound_(sound), tracked_(tracked), released_(false), paused_(false), volume_(1.0f)
	{
	}

	Sound::~Sound()
	{
		if (sound_)
		{
			ma_sound_uninit(sound_);
			delete sound_;
		}
	}

	void Sound::setIsPaused(bool paused)
	{
		paused_ = paused;
		if (!sound_)
			return;
		if (paused)
			ma_sound_stop(sound_);
		else
			ma_sound_start(sound_);
	}

	void Sound::setVolume(float volume)
	{
		volume_ = volume;
		if (sound_)
			ma_sound_set_volume(sound_, volume);
	}

	void Sound::stop()
	{
		paused_ = true;
		if (!sound_)
			return;
		ma_sound_stop(sound_);
		ma_sound_seek_to_pcm_frame(sound_, 0);
	}

	bool Sound::isFinished()
	{
		return !sound_ || (!paused_ && !ma_sound_is_playing(sound_));
	}

	SoundEngine::SoundEngine() : engine_(new ma_engine)
	{
		if (ma_engine_init(NULL, engine_) != MA_SUCCESS)
		{
			delete engine_;
			engine_ = 0;
			fprintf(stderr, "Sound is off: no audio device could be opened\n");
		}
	}

	SoundEngine::~SoundEngine()
	{
		for (size_t i = 0; i < sounds_.size(); ++i)
			delete sounds_[i];
		if (engine_)
		{
			ma_engine_uninit(engine_);
			delete engine_;
		}
	}

	Sound* SoundEngine::play2D(const char* file, bool looped, bool startPaused, bool track)
	{
		removeFinished();

		ma_sound* sound = 0;
		bool missing = std::find(missing_.begin(), missing_.end(), file) != missing_.end();
		if (engine_ && !missing)
		{
			sound = new ma_sound;
			if (ma_sound_init_from_file(engine_, file, 0, NULL, NULL, sound) != MA_SUCCESS)
			{
				fprintf(stderr, "Could not load sound %s\n", file);
				missing_.push_back(file);
				delete sound;
				sound = 0;
			}
		}

		bool tracked = startPaused || track;
		if (!sound && !tracked)
			return 0;

		Sound* result = new Sound(file, sound, tracked);
		sounds_.push_back(result);
		if (sound)
			ma_sound_set_looping(sound, looped);
		result->setIsPaused(startPaused);
		return tracked ? result : 0;
	}

	bool SoundEngine::isCurrentlyPlaying(const char* file)
	{
		for (size_t i = 0; i < sounds_.size(); ++i)
			if (sounds_[i]->file_ == file && sounds_[i]->sound_ && ma_sound_is_playing(sounds_[i]->sound_))
				return true;
		return false;
	}

	void SoundEngine::removeSoundSource(const char* file)
	{
		for (size_t i = 0; i < sounds_.size(); ++i)
			if (sounds_[i]->file_ == file)
				sounds_[i]->stop();
		removeFinished();
	}

	void SoundEngine::stopAllSounds()
	{
		for (size_t i = 0; i < sounds_.size(); ++i)
			sounds_[i]->stop();
		removeFinished();
	}

	void SoundEngine::setSoundVolume(float volume)
	{
		if (engine_)
			ma_engine_set_volume(engine_, volume);
	}

	void SoundEngine::removeFinished()
	{
		size_t kept = 0;
		for (size_t i = 0; i < sounds_.size(); ++i)
		{
			Sound* sound = sounds_[i];
			bool owned = !sound->tracked_ || sound->released_;
			bool idle = !sound->sound_ || !ma_sound_is_playing(sound->sound_);
			if (owned && idle)
				delete sound;
			else
				sounds_[kept++] = sound;
		}
		sounds_.resize(kept);
	}

	SoundEngine* createSoundEngine()
	{
		return new SoundEngine();
	}
}
