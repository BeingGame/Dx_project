#include "CollisionInfoManager.h"

CCollisionInfoManager* CCollisionInfoManager::mInstance = nullptr;

CCollisionInfoManager::CCollisionInfoManager()
{

}

CCollisionInfoManager::~CCollisionInfoManager()
{

}

bool CCollisionInfoManager::Init()
{
	//엔진의 기본적인 콜리전 프로파일을 만들어준다.
	CreateChannel("Static");
	CreateChannel("Player");
	CreateChannel("Monster");

	//채널을 만든뒤 프로파일을 생성한다.
	CreateProfile("Static", "Static", true);
	CreateProfile("Player", "Player", true);
	CreateProfile("Monster", "Monster", true);

	/*SetProfileInteraction("Player", "Monster", ECollisionInteraction::Block);
	SetProfileInteraction("Monster", "Player", ECollisionInteraction::Block);*/

	return true;
}

void CCollisionInfoManager::CreateChannel(const std::string& Name)
{
	//mCreateChannel을 사용해서 현재 만들어진 채널을 생성하고
	//범위를 제한해서 총 n개의 채널만 만들어질수 있게 한다.

	//범위를 제한한다.
	if (mCreateChannel == ECollisionChannel::End)
	{
		return;
	}

	std::shared_ptr<FCollisionChannel> Channel = FindChannel(Name);

	//중복된 이름의 채널이 있는지 확인한다.
	if (Channel)
	{
		return;
	}

	//없으면 새로운 채널을 추가한다.
	Channel = std::make_shared<FCollisionChannel>();

	Channel->Name = Name;

	Channel->Channel = mCreateChannel;

	mCreateChannel = (ECollisionChannel::Type)(mCreateChannel + 1);

	mChannelMap.insert(std::make_pair(Name, Channel));
}

bool CCollisionInfoManager::CreateProfile(const std::string& Name, ECollisionChannel::Type Channel, bool Enable, ECollisionInteraction::Type DefaultInteraction)
{
	std::shared_ptr<FCollisionProfile> Profile = FindProfile(Name);

	if (Profile)
	{
		return true;
	}

	//ChannelMap에 Channel이 존재하는지 확인한다.

	std::shared_ptr<FCollisionChannel> ChannelObject = nullptr;

	auto iter = mChannelMap.begin();
	auto iterEnd = mChannelMap.end();

	for (; iter != iterEnd; ++iter)
	{
		if (iter->second->Channel == Channel)
		{
			ChannelObject = iter->second;
			break;
		}
	}

	if (!ChannelObject)
	{
		return false;
	}

	Profile = std::make_shared<FCollisionProfile>();

	Profile->Name = Name;
	Profile->Channel = ChannelObject;
	Profile->Enable = Enable;

	//기본적인 상호작용을 모든 채널에 넣어준다.
	for (int i = 0; i < ECollisionChannel::End; ++i)
	{
		Profile->Interaction[i] = DefaultInteraction;
 	}

	mProfileMap.insert(std::make_pair(Name, Profile));

	return true;
}

bool CCollisionInfoManager::CreateProfile(const std::string& Name, const std::string& ChannelName, bool Enable, ECollisionInteraction::Type DefaultInteraction)
{
	
	std::shared_ptr<FCollisionProfile> Profile = FindProfile(Name);

	if (Profile)
	{
		return true;
	}

	//ChannelMap에 Channel이 존재하는지 확인한다.

	std::shared_ptr<FCollisionChannel> Channel = FindChannel(ChannelName);

	if (!Channel)
	{
		return false;
	}

	Profile = std::make_shared<FCollisionProfile>();

	Profile->Name = Name;
	Profile->Channel = Channel;
	Profile->Enable = Enable;

	//기본적인 상호작용을 모든 채널에 넣어준다.
	for (int i = 0; i < ECollisionChannel::End; ++i)
	{
		Profile->Interaction[i] = DefaultInteraction;
	}

	mProfileMap.insert(std::make_pair(Name, Profile));

	return true;
}

bool CCollisionInfoManager::SetProfileInteraction(const std::string& Name, ECollisionChannel::Type Channel, ECollisionInteraction::Type Interaction)
{
	std::shared_ptr<FCollisionProfile> Profile = FindProfile(Name);

	if (!Profile)
	{
		return false;
	}

	Profile->Interaction[Channel] = Interaction;

	return true;
}

bool CCollisionInfoManager::SetProfileInteraction(const std::string& Name, const std::string& ChannelName, ECollisionInteraction::Type Interaction)
{
	std::shared_ptr<FCollisionProfile> Profile = FindProfile(Name);

	if (!Profile)
	{
		return false;
	}

	std::shared_ptr<FCollisionChannel> Channel = FindChannel(ChannelName);

	if (!Channel)
	{
		return false;
	}

	Profile->Interaction[Channel->Channel] = Interaction;

	return true;
}

bool CCollisionInfoManager::SetProfileEnable(const std::string& Name, bool Enable)
{
	std::shared_ptr<FCollisionProfile> Profile = FindProfile(Name);

	if (!Profile)
	{
		return false;
	}

	Profile->Enable = Enable;

	return true;
}

std::shared_ptr<FCollisionProfile> CCollisionInfoManager::FindProfile(const std::string& Name)
{
	auto iter = mProfileMap.find(Name);

	if (iter == mProfileMap.end())
	{
		return std::shared_ptr<FCollisionProfile>();
	}

	return iter->second;
}

std::shared_ptr<FCollisionChannel> CCollisionInfoManager::FindChannel(const std::string& Name)
{
	auto iter = mChannelMap.find(Name);

	if (iter == mChannelMap.end())
	{
		return std::shared_ptr<FCollisionChannel>();
	}

	return iter->second;
}
