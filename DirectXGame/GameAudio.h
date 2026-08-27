#pragma once

#include <array>
#include <cstddef>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

// Centralized music/SE playback for the MP3 assets in Resources/music.
// KamataEngine::Audio accepts WAV only, so this class uses Windows MCI for MP3.
class GameAudio
{
public:
	enum class Bgm : size_t
	{
		Title,
		Tutorial,
		GamePlay,
		Boss,
		GameOver,
		GameClear,
		Count,
		None = Count,
	};

	enum class Se : size_t
	{
		Danger,
		Hit,
		PlayerHit,
		PlayerShot,
		EnemyShot,
		LockOn,
		LockOff,
		BossRoar,
		MenuSelect,
		MenuConfirm,
		MenuCancel,
		PauseOpen,
		PauseClose,
		Count,
	};

	static GameAudio* GetInstance();

	void Initialize();
	void Finalize();
	void PlayBgm(Bgm bgm);
	void StopBgm();
	void PlaySe(Se se);
	void PlaySeLoop(Se se);
	void StopSe(Se se);
	void SetBgmVolume(float volume);
	void SetSeVolume(float volume);
	float GetBgmVolume() const { return bgmVolume_; }
	float GetSeVolume() const { return seVolume_; }

private:
	struct Track
	{
		std::string alias;
		int32_t baseVolume = 1000;
		bool loaded = false;
	};

	GameAudio() = default;
	bool OpenTrack(Track& track, const char* alias, const char* relativePath, int32_t volume);
	bool SendCommand(const std::string& command) const;
	void RunOnAudioThread(std::function<void()> command);
	void AudioThreadMain();
	void StopBgmOnAudioThread();

	std::array<Track, static_cast<size_t>(Bgm::Count)> bgmTracks_{};
	std::array<Track, static_cast<size_t>(Se::Count)> seTracks_{};
	Bgm currentBgm_ = Bgm::None;
	bool initialized_ = false;
	float bgmVolume_ = 1.0f;
	float seVolume_ = 1.0f;
	std::thread audioThread_;
	std::mutex commandMutex_;
	std::condition_variable commandCondition_;
	std::deque<std::function<void()>> commands_;
	bool stopThread_ = false;
};
