#ifndef IRRKLANG_SILENT_STUB_H
#define IRRKLANG_SILENT_STUB_H

namespace irrklang
{
	typedef float ik_f32;

	class ISound
	{
	public:
		void setIsPaused(bool paused = true) { paused_ = paused; }
		bool getIsPaused() { return paused_; }
		void setVolume(ik_f32 volume) { volume_ = volume; }
		ik_f32 getVolume() { return volume_; }
		void stop() {}
		bool isFinished() { return true; }
		bool drop() { delete this; return true; }
	private:
		bool paused_ = false;
		ik_f32 volume_ = 1.0f;
	};

	class ISoundEngine
	{
	public:
		ISound* play2D(const char*, bool = false,
			bool startPaused = false, bool track = false)
		{
			if (!startPaused && !track)
				return 0;
			ISound* sound = new ISound();
			sound->setIsPaused(startPaused);
			return sound;
		}
		bool isCurrentlyPlaying(const char*) { return false; }
		void removeSoundSource(const char*) {}
		void stopAllSounds() {}
		void setSoundVolume(ik_f32) {}
		bool drop() { delete this; return true; }
	};

	inline ISoundEngine* createIrrKlangDevice() { return new ISoundEngine(); }
}

#endif
