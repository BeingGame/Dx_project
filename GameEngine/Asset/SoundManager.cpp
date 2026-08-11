#include "SoundManager.h"

CSoundManager::CSoundManager()
{}

CSoundManager::~CSoundManager()
{
	mSoundMap.clear();

	auto iter = mChannelGroupMap.begin();
	auto iterEnd = mChannelGroupMap.end();

	for (; iter != iterEnd; ++iter)
	{
		iter->second->release();
	}

	if (mSystem)
	{
		mSystem->release();

		//release하면 close가 같이 호출되서 명시적으로 close를 호출해준것

		//드라이버나 장치와 연결을 끊는 함수
		mSystem->close();
	}

}

bool CSoundManager::Init()
{
	//먼저 시스템 생성및 초기화를 진행한다.

	FMOD_RESULT Result = FMOD::System_Create(&mSystem);

	if (Result != FMOD_OK)
	{
		return false;
	}

	Result = mSystem->init(256, FMOD_INIT_NORMAL, nullptr);

	if (Result != FMOD_OK)
	{
		return false;
	}

	Result = mSystem->getMasterChannelGroup(&mMasterGroup);

	if (Result != FMOD_OK)
	{
		return false;
	}

	mChannelGroupMap.insert(std::make_pair("Master", mMasterGroup));

	//마스터 그룹이 생성되면 하위 그룹을 생성해준다.
	CreateChannelGroup("BGM");
	CreateChannelGroup("Effect");
	CreateChannelGroup("UI");

	return true;
}

void CSoundManager::Update()
{
	mSystem->update();
}

std::weak_ptr<CSound> CSoundManager::FindSound(const std::string& Name)
{
	std::string SoundName = "Sound_" + Name;

	auto iter = mSoundMap.find(SoundName);

	if (iter == mSoundMap.end())
	{
		return std::weak_ptr<CSound>();
	}

	return iter->second;
}

bool CSoundManager::CreateChannelGroup(const std::string& Name)
{
	//동일한 이름의 그룹을 확인한다.
	FMOD::ChannelGroup* Group = FindChannelGroup(Name);

	if (Group)
	{
		return true;
	}

	FMOD_RESULT Result = mSystem->createChannelGroup(Name.c_str(), &Group);

	if (Result != FMOD_OK)
	{
		return false;
	}

	mMasterGroup->addGroup(Group, false);

	mChannelGroupMap.insert(std::make_pair(Name, Group));

	return true;
}

bool CSoundManager::LoadSound(const std::string& Name, const std::string& GroupName, bool Loop, const char* FileName, const std::string& PathName, FMOD_MODE Mode)
{
	if (!FindSound(Name).expired())
	{
		return true;
	}

	//그룹이 없으면 마스터 그룹에 사운드를 포함시킨다.
	FMOD::ChannelGroup* Group = FindChannelGroup(GroupName);

	if (!Group)
	{
		Group = mMasterGroup;
	}

	std::string SoundName = "Sound_" + Name;

	std::shared_ptr<CSound> Sound = std::make_shared<CSound>();

	Sound->SetName(SoundName);

	if (!Sound->LoadSound(mSystem, Group, Loop, FileName, PathName, Mode))
	{
		return false;
	}

	mSoundMap.insert(std::make_pair(SoundName, Sound));

	return true;
}

bool CSoundManager::LoadSoundFullPath(const std::string& Name, const std::string& GroupName, bool Loop, const char* FullPath, FMOD_MODE Mode)
{
	if (!FindSound(Name).expired())
	{
		return true;
	}

	//그룹이 없으면 마스터 그룹에 사운드를 포함시킨다.
	FMOD::ChannelGroup* Group = FindChannelGroup(GroupName);

	if (!Group)
	{
		Group = mMasterGroup;
	}

	std::string SoundName = "Sound_" + Name;

	std::shared_ptr<CSound> Sound = std::make_shared<CSound>();

	Sound->SetName(SoundName);

	if (!Sound->LoadSoundFullPath(mSystem, Group, Loop, FullPath, Mode))
	{
		return false;
	}

	mSoundMap.insert(std::make_pair(SoundName, Sound));

	return true;
}

void CSoundManager::Play(const std::string& Name)
{
	std::string SoundName = "Sound_" + Name;

	auto iter = mSoundMap.find(SoundName);

	if (iter == mSoundMap.end())
	{
		return;
	}

	iter->second->Play();
}

void CSoundManager::Stop(const std::string& Name)
{
	std::string SoundName = "Sound_" + Name;

	auto iter = mSoundMap.find(SoundName);

	if (iter == mSoundMap.end())
	{
		return;
	}

	iter->second->Stop();
}

void CSoundManager::Pause(const std::string& Name)
{
	std::string SoundName = "Sound_" + Name;

	auto iter = mSoundMap.find(SoundName);

	if (iter == mSoundMap.end())
	{
		return;
	}

	iter->second->Pause();
}

void CSoundManager::Resume(const std::string& Name)
{
	std::string SoundName = "Sound_" + Name;

	auto iter = mSoundMap.find(SoundName);

	if (iter == mSoundMap.end())
	{
		return;
	}

	iter->second->Resume();
}

void CSoundManager::SetMasterVolume(int Volume)
{
	if (Volume < 0)
	{
		Volume = 0;
	}

	else if (Volume > 100)
	{
		Volume = 100;
	}

	mMasterGroup->setVolume(Volume / 100.f);
}

void CSoundManager::SetGroupVolume(const std::string& GroupName, int Volume)
{
	FMOD::ChannelGroup* Group = FindChannelGroup(GroupName);

	if (!Group)
	{
		Group = mMasterGroup;
	}

	if (Volume < 0)
	{
		Volume = 0;
	}

	else if (Volume > 100)
	{
		Volume = 100;
	}

	Group->setVolume(Volume / 100.f);
}

void CSoundManager::SetListenerAttribute(const FVector3& Pos, const FVector3& Vel, const FVector3& Forward, const FVector3& Up)
{
	if (!mSystem)
	{
		return;
	}

	FMOD_VECTOR _Pos = { Pos.x,Pos.y,Pos.z };
	FMOD_VECTOR _Vel = { Vel.x,Vel.y,Vel.z };
	FMOD_VECTOR _Forward = { Forward.x,Forward.y,Forward.z };
	FMOD_VECTOR _Up = { Up.x,Up.y,Up.z };

	//0번 리스너에 위치, 속도, 전방, 위 벡터를 동기화해준다.
	mSystem->set3DListenerAttributes(0, &_Pos, &_Vel, &_Forward, &_Up);
}

FMOD::ChannelGroup* CSoundManager::FindChannelGroup(const std::string& Name)
{
	auto iter = mChannelGroupMap.find(Name);

	if (iter == mChannelGroupMap.end())
	{
		return nullptr;
	}

	return iter->second;
}
