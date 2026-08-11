#pragma once

#include "../EngineInfo.h"

//싱글톤으로 클래스를 구현한다.
//엔진의 모든 오브젝트들이 사용하기 때문에
//똑같지만 값이 다른 프로파일이 생성되는 것을 막기위해 싱글톤으로 생성한다.

class CCollisionInfoManager
{
	Singleton(CCollisionInfoManager);

	//실제로 사용될 콜리전의 채널과, 프로파일을 생성해서 저장한다.
private:

	//실제로 사용될 채널 맵
	std::unordered_map<std::string, std::shared_ptr<FCollisionChannel>> mChannelMap;
	//현재 만들어진 채널의 위치
	//채널의 갯수를 유한하게 증가시키기 위해 선언한다.
	ECollisionChannel::Type mCreateChannel = ECollisionChannel::Static;

	//실제로 사용될 프로파일 맵
	std::unordered_map<std::string, std::shared_ptr<FCollisionProfile>> mProfileMap;

public:
	bool Init();

	void CreateChannel(const std::string& Name);
	bool CreateProfile(const std::string& Name, ECollisionChannel::Type Channel, bool Enable,
		ECollisionInteraction::Type DefaultInteraction = ECollisionInteraction::Overlap);
	bool CreateProfile(const std::string& Name, const std::string& ChannelName, bool Enable,
		ECollisionInteraction::Type DefaultInteraction = ECollisionInteraction::Overlap);
	bool SetProfileInteraction(const std::string& Name, ECollisionChannel::Type Channel, ECollisionInteraction::Type Interaction);
	bool SetProfileInteraction(const std::string& Name, const std::string& ChannelName, ECollisionInteraction::Type Interaction);
	bool SetProfileEnable(const std::string& Name, bool Enable);

	std::shared_ptr<FCollisionProfile> FindProfile(const std::string& Name);

private:
	std::shared_ptr<FCollisionChannel> FindChannel(const std::string& Name);
};

