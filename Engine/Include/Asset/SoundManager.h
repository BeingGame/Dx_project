#pragma once

#include "AssetSubManager.h"
#include "Sound.h"

class CSoundManager :
	public CAssetSubManager
{
public:
	CSoundManager();
	~CSoundManager();

private:
	std::unordered_map<std::string, std::shared_ptr<CSound>> mSoundMap;
	std::unordered_map<std::string, FMOD::ChannelGroup*> mChannelGroupMap;

	FMOD::System* mSystem = nullptr;
	FMOD::ChannelGroup* mMasterGroup = nullptr;

public:
	bool Init();
	void Update();
	std::weak_ptr<CSound> FindSound(const std::string& Name);


	bool CreateChannelGroup(const std::string& Name);

	//사운드를 파일에서 읽어서 저장할 함수
	bool LoadSound(const std::string& Name, const std::string& GroupName, bool Loop, const char* FileName, const std::string& PathName = "Sound", FMOD_MODE Mode = FMOD_2D);
	bool LoadSoundFullPath(const std::string& Name, const std::string& GroupName, bool Loop, const char* FullPath, FMOD_MODE Mode = FMOD_2D);

	//사운드 제어함수
	void Play(const std::string& Name);
	void Stop(const std::string& Name);
	void Pause(const std::string& Name);
	void Resume(const std::string& Name);

	void SetMasterVolume(int Volume);
	void SetGroupVolume(const std::string& GroupName, int Volume);
	void SetListenerAttribute(const FVector3& Pos, const FVector3& Vel = FVector3::Zero, const FVector3& Forward = FVector3(0.f, 0.f, 1.f), const FVector3& Up = FVector3(0.f, 1.f, 0.f));

private:
	FMOD::ChannelGroup* FindChannelGroup(const std::string& Name);

public:
	template<typename T>
	void SetSoundEndFunc(const std::string& Name, T* Object, void (T::* Func)())
	{
		auto iter = mSoundMap.find(Name);

		if (iter == mSoundMap.end())
		{
			return;
		}

		iter->second->SetSoundEndFunc(Object, Func);
	}


};

