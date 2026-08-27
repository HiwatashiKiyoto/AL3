#include "GameAudio.h"

#include <Windows.h>
#include <algorithm>
#include <filesystem>
#include <future>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

namespace
{
std::filesystem::path ResolveAudioPath(const char* relativePath)
{
	const std::filesystem::path projectRelative = std::filesystem::path("Resources") / relativePath;
	if (std::filesystem::exists(projectRelative))
	{
		return std::filesystem::absolute(projectRelative);
	}

	const std::filesystem::path workspaceRelative = std::filesystem::path("DirectXGame/Resources") / relativePath;
	if (std::filesystem::exists(workspaceRelative))
	{
		return std::filesystem::absolute(workspaceRelative);
	}

	return {};
}
}

GameAudio* GameAudio::GetInstance()
{
	static GameAudio instance;
	return &instance;
}

void GameAudio::Initialize()
{
	if (initialized_)
	{
		return;
	}
	initialized_ = true;
	stopThread_ = false;
	audioThread_ = std::thread(&GameAudio::AudioThreadMain, this);

	// MCI's MP3 driver fails when opened from KamataEngine's MTA main thread.
	// Open and control every track from one dedicated STA thread instead.
	RunOnAudioThread([this]() {
		OpenTrack(bgmTracks_[static_cast<size_t>(Bgm::Title)], "game_bgm_title", "music/title.mp3", 420);
		OpenTrack(bgmTracks_[static_cast<size_t>(Bgm::Tutorial)], "game_bgm_tutorial", "music/tutorial.mp3", 360);
		OpenTrack(bgmTracks_[static_cast<size_t>(Bgm::GamePlay)], "game_bgm_play", "music/gamePlay.mp3", 390);
		OpenTrack(bgmTracks_[static_cast<size_t>(Bgm::Boss)], "game_bgm_boss", "music/boss.mp3", 440);
		OpenTrack(bgmTracks_[static_cast<size_t>(Bgm::GameOver)], "game_bgm_over", "music/gameOver.mp3", 420);
		OpenTrack(bgmTracks_[static_cast<size_t>(Bgm::GameClear)], "game_bgm_clear", "music/gameClear.mp3", 420);

		OpenTrack(seTracks_[static_cast<size_t>(Se::Danger)], "game_se_danger", "music/danger.mp3", 850);
		OpenTrack(seTracks_[static_cast<size_t>(Se::Hit)], "game_se_hit", "music/hit.mp3", 720);
		OpenTrack(seTracks_[static_cast<size_t>(Se::PlayerHit)], "game_se_player_hit", "music/playerHit.mp3", 850);
		OpenTrack(seTracks_[static_cast<size_t>(Se::PlayerShot)], "game_se_player_shot", "music/shot.mp3", 780);
		OpenTrack(seTracks_[static_cast<size_t>(Se::EnemyShot)], "game_se_enemy_shot", "music/enemyShot.mp3", 670);
		OpenTrack(seTracks_[static_cast<size_t>(Se::LockOn)], "game_se_lock_on", "music/rockOn.mp3", 720);
		OpenTrack(seTracks_[static_cast<size_t>(Se::LockOff)], "game_se_lock_off", "music/rockOff.mp3", 620);
		OpenTrack(seTracks_[static_cast<size_t>(Se::BossRoar)], "game_se_boss_roar", "music/roar.mp3", 950);
		OpenTrack(seTracks_[static_cast<size_t>(Se::MenuSelect)], "game_se_menu_select", "music/select.mp3", 630);
		OpenTrack(seTracks_[static_cast<size_t>(Se::MenuConfirm)], "game_se_menu_confirm", "music/deside.mp3", 760);
		OpenTrack(seTracks_[static_cast<size_t>(Se::MenuCancel)], "game_se_menu_cancel", "music/cancel.mp3", 650);
		OpenTrack(seTracks_[static_cast<size_t>(Se::PauseOpen)], "game_se_pause_open", "music/poseStart.mp3", 760);
		OpenTrack(seTracks_[static_cast<size_t>(Se::PauseClose)], "game_se_pause_close", "music/poseCancel.mp3", 700);
	});
}

void GameAudio::Finalize()
{
	if (!initialized_)
	{
		return;
	}

	RunOnAudioThread([this]() {
		StopBgmOnAudioThread();
		for (Track& track : bgmTracks_)
		{
			if (track.loaded)
			{
				SendCommand("close " + track.alias);
				track.loaded = false;
			}
		}
		for (Track& track : seTracks_)
		{
			if (track.loaded)
			{
				SendCommand("stop " + track.alias);
				SendCommand("close " + track.alias);
				track.loaded = false;
			}
		}
	});

	{
		std::lock_guard<std::mutex> lock(commandMutex_);
		stopThread_ = true;
	}
	commandCondition_.notify_one();
	if (audioThread_.joinable())
	{
		audioThread_.join();
	}
	initialized_ = false;
}

void GameAudio::PlayBgm(Bgm bgm)
{
	if (!initialized_ || bgm == Bgm::None)
	{
		return;
	}
	RunOnAudioThread([this, bgm]() {
		if (bgm == currentBgm_)
		{
			return;
		}
		StopBgmOnAudioThread();
		Track& track = bgmTracks_[static_cast<size_t>(bgm)];
		if (!track.loaded)
		{
			return;
		}
		SendCommand("seek " + track.alias + " to start");
		if (SendCommand("play " + track.alias + " repeat"))
		{
			currentBgm_ = bgm;
		}
	});
}

void GameAudio::StopBgm()
{
	if (!initialized_)
	{
		return;
	}
	RunOnAudioThread([this]() { StopBgmOnAudioThread(); });
}

void GameAudio::StopBgmOnAudioThread()
{
	if (currentBgm_ == Bgm::None)
	{
		return;
	}
	Track& track = bgmTracks_[static_cast<size_t>(currentBgm_)];
	if (track.loaded)
	{
		SendCommand("stop " + track.alias);
	}
	currentBgm_ = Bgm::None;
}

void GameAudio::PlaySe(Se se)
{
	if (!initialized_)
	{
		return;
	}
	RunOnAudioThread([this, se]() {
		Track& track = seTracks_[static_cast<size_t>(se)];
		if (!track.loaded)
		{
			return;
		}
		// Restarting keeps rapid hits crisp and avoids accumulating stale voices.
		SendCommand("stop " + track.alias);
		SendCommand("seek " + track.alias + " to start");
		SendCommand("play " + track.alias);
	});
}

void GameAudio::PlaySeLoop(Se se)
{
	if (!initialized_)
	{
		return;
	}
	RunOnAudioThread([this, se]() {
		Track& track = seTracks_[static_cast<size_t>(se)];
		if (!track.loaded)
		{
			return;
		}
		SendCommand("stop " + track.alias);
		SendCommand("seek " + track.alias + " to start");
		SendCommand("play " + track.alias + " repeat");
	});
}

void GameAudio::StopSe(Se se)
{
	if (!initialized_)
	{
		return;
	}
	RunOnAudioThread([this, se]() {
		Track& track = seTracks_[static_cast<size_t>(se)];
		if (track.loaded)
		{
			SendCommand("stop " + track.alias);
		}
	});
}

void GameAudio::SetBgmVolume(float volume)
{
	bgmVolume_ = std::clamp(volume, 0.0f, 1.0f);
	if (!initialized_)
	{
		return;
	}
	RunOnAudioThread([this]() {
		for (const Track& track : bgmTracks_)
		{
			if (track.loaded)
			{
				const int32_t volume = static_cast<int32_t>(static_cast<float>(track.baseVolume) * bgmVolume_);
				SendCommand("setaudio " + track.alias + " volume to " + std::to_string(volume));
			}
		}
	});
}

void GameAudio::SetSeVolume(float volume)
{
	seVolume_ = std::clamp(volume, 0.0f, 1.0f);
	if (!initialized_)
	{
		return;
	}
	RunOnAudioThread([this]() {
		for (const Track& track : seTracks_)
		{
			if (track.loaded)
			{
				const int32_t volume = static_cast<int32_t>(static_cast<float>(track.baseVolume) * seVolume_);
				SendCommand("setaudio " + track.alias + " volume to " + std::to_string(volume));
			}
		}
	});
}

bool GameAudio::OpenTrack(Track& track, const char* alias, const char* relativePath, int32_t volume)
{
	const std::filesystem::path path = ResolveAudioPath(relativePath);
	if (path.empty())
	{
		OutputDebugStringA((std::string("Audio file not found: ") + relativePath + "\n").c_str());
		return false;
	}

	track.alias = alias;
	track.baseVolume = volume;
	const std::string openCommand = "open \"" + path.string() + "\" type mpegvideo alias " + track.alias;
	track.loaded = SendCommand(openCommand);
	if (track.loaded)
	{
		const bool isBgm = alias[5] == 'b';
		const float masterVolume = isBgm ? bgmVolume_ : seVolume_;
		const int32_t appliedVolume = static_cast<int32_t>(static_cast<float>(volume) * masterVolume);
		SendCommand("setaudio " + track.alias + " volume to " + std::to_string(appliedVolume));
	}
	return track.loaded;
}

bool GameAudio::SendCommand(const std::string& command) const
{
	const MCIERROR result = mciSendStringA(command.c_str(), nullptr, 0, nullptr);
	if (result == 0)
	{
		return true;
	}

	char errorText[256]{};
	mciGetErrorStringA(result, errorText, static_cast<UINT>(sizeof(errorText)));
	OutputDebugStringA(("MCI audio error: " + command + " : " + errorText + "\n").c_str());
	return false;
}

void GameAudio::RunOnAudioThread(std::function<void()> command)
{
	auto completion = std::make_shared<std::promise<void>>();
	std::future<void> completed = completion->get_future();
	{
		std::lock_guard<std::mutex> lock(commandMutex_);
		commands_.push_back([command = std::move(command), completion]() {
			try
			{
				command();
				completion->set_value();
			}
			catch (...)
			{
				completion->set_exception(std::current_exception());
			}
		});
	}
	commandCondition_.notify_one();
	completed.get();
}

void GameAudio::AudioThreadMain()
{
	const HRESULT comResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	while (true)
	{
		std::function<void()> command;
		{
			std::unique_lock<std::mutex> lock(commandMutex_);
			commandCondition_.wait(lock, [this]() { return stopThread_ || !commands_.empty(); });
			if (stopThread_ && commands_.empty())
			{
				break;
			}
			command = std::move(commands_.front());
			commands_.pop_front();
		}
		command();
	}
	if (SUCCEEDED(comResult))
	{
		CoUninitialize();
	}
}
