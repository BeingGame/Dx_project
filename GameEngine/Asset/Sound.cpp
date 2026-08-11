#include "Sound.h"
#include "PathManager.h"

CSound::CSound()
{
	mType = EAssetType::Sound;
}

CSound::~CSound()
{
	if (mSound)
	{
		mSound->release();
	}
}

bool CSound::LoadSound(FMOD::System * System, FMOD::ChannelGroup * Group, bool Loop, const char* FileName, const std::string & PathName, FMOD_MODE Mode)
{
	char FullPath[MAX_PATH] = {};

	const char* Path =CPathManager::FindPathMultiByte(PathName);
	
	if (Path)
	{
		strcpy_s(FullPath, Path);
	}

	strcat_s(FullPath, FileName);

	return LoadSoundFullPath(System,Group,Loop,FullPath,Mode);
}

bool CSound::LoadSoundFullPath(FMOD::System* System, FMOD::ChannelGroup* Group, bool Loop, const char* FullPath, FMOD_MODE Mode)
{
	mSystem = System;
	mGroup = Group;
	mLoop = Loop;

	//FMOD가 사운드를 생성할때 객체 생성을 위한 플래그
	//기본적으로는 default->2D 플래그를 사용하는데
	//공간감쇠가 필요한때 3D플래그를 사용한다.
	//반복일때 Mode플래그를 갱신한다.
	if (Loop)
	{
		Mode |= FMOD_LOOP_NORMAL;
	}
	else
	{
		Mode |= FMOD_LOOP_OFF;
	}

	if (mSystem->createSound(FullPath, Mode, nullptr, &mSound) != FMOD_OK)
	{
		return false;
	}

	return true;
}

void CSound::Play()
{
	//사운드를 실행한다음 사운드가 실행되는 채널을 받아온다.
	mSystem->playSound(mSound, mGroup, false, &mChannel);

	++mPlayCount;

	//UserData를 CSound 객체의 this포인터를 넣어준다.
	mChannel->setUserData(this);

	//사운드가 종료됬을때 콜백함수를 불러준다.
	mChannel->setCallback(SoundPlayEnd);

}

void CSound::Stop()
{
	if (mChannel)
	{
		--mPlayCount;

		bool IsPlaying = false;

		//현재 채널에서 사운드가 재생중인지 확인한다.
		mChannel->isPlaying(&IsPlaying);
	
		//실행중이라면 PlayCount가 0이아니라면 채널 미반납
		//실행중이 아니라면 채널을 즉시 반납해준다.
		if (IsPlaying)
		{
			mChannel->stop();

			if (mPlayCount == 0)
			{
				mChannel = nullptr;
			}
		}
		else
		{
			mChannel = nullptr;
		}

	}
}

void CSound::Pause()
{
	if (mChannel)
	{
		bool IsPlaying = false;

		//현재 채널에서 사운드가 재생중인지 확인한다.
		mChannel->isPlaying(&IsPlaying);

		//사운드가 정지상태이므로 채널은 유지시킨다.
		if (IsPlaying)
		{
			mChannel->setPaused(true);
		}
	}

}

void CSound::Resume()
{
	if (mChannel)
	{
		bool IsPaused = false;

		//현재 채널에서 사운드가 정지중인지 확인한다.
		mChannel->getPaused(&IsPaused);

		//사운드가 정지상태면 다시 사운드가 재생되도록 한다.
		if (IsPaused)
		{
			mChannel->setPaused(false);
		}
	}
}

void CSound::Set3DMinMaxDistance(float MinDist, float MaxDist)
{
	if (mSound)
	{
		mSound->set3DMinMaxDistance(MinDist, MaxDist);
	}
}
//현재 소리가 출력되고 있는 채널의 3D공간 및 속도
void CSound::Set3DAttributes(const FMOD_VECTOR & Pos, const FMOD_VECTOR & Vel)
{
	if (mChannel)
	{
		mChannel->set3DAttributes(&Pos, &Vel);
	}
}

bool CSound::IsPlaying() const
{
	if (!mChannel)
	{
		return false;
	}

	bool IsPlaying = false;

	mChannel->isPlaying(&IsPlaying);

	return IsPlaying;
}

FMOD_RESULT CSound::SoundPlayEnd(FMOD_CHANNELCONTROL* ChannelControl, FMOD_CHANNELCONTROL_TYPE ControlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE CallBackType, void* CommandData1, void* CommandData2)
{
	//종료시에 콜백함수를 호출하도록 만들어준다.
	if (CallBackType == FMOD_CHANNELCONTROL_CALLBACK_END)
	{
		FMOD::ChannelControl* Control = (FMOD::ChannelControl*)ChannelControl;

		//PlaySound시 넣어주었던 UserData(this포인터)를 이용해서 CSound 객체로 다시 형변환해준다.
		void* UserData = nullptr;
		Control->getUserData(&UserData);

		if (UserData)
		{
			auto Sound = static_cast<CSound*>(UserData);

			if (Sound->mSoundPlayEndFunc)
			{
				Sound->mSoundPlayEndFunc();
			}
		}

	}
	return FMOD_RESULT();
}
