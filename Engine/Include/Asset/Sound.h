#pragma once

#include "Asset.h"

#include "fmod.hpp"

//동적 라이브러리를 사용하기 위해
//함수의 위치를 알려주는 정적 라이브러리를 연결
//연결된 정적 라이브러리는 프로그램이 실행될때 메모리에 할당되는 dll(동적 라이브러리)와의 연결
//역할을 한다
#pragma comment(lib,"fmod_vc.lib")

class CSound :
	public CAsset
{
public:
	CSound();
	virtual ~CSound();

protected:
	//컴객체 처럼 FMOD 라이브러리에서 사용하는 포인터들을 release를 해줘야한다.
	//따라서 스마트포인터를 사용하지않고 직접 제거하기위한 원시포인터를 사용한다.

	//FMOD로 실행된 사운드의 본체
	//SoundManager가 연결해준다.
	//출력장치를 결정해준다.
	FMOD::System* mSystem = nullptr;

	//여러 채널을 묶어놓은 그룹 변수
	//한번에 볼륨 조절하고 사운드 이펙트를 변경할 수 있다.
	FMOD::ChannelGroup* mGroup = nullptr;

	//현재 출력되고 있는 사운드의 채널을 받아와서 직접적으로 제어할수 있게 하는 변수
	//FMOD::playSound 함수에서 Channel을 받아올수 있다.
	FMOD::Channel* mChannel = nullptr;

	//실제 사운드 파일
	//playSound로 실제로 실행하게될 파일 변수
	//system에서 sound 객체를 파일로 읽어와서 생성한다.
	FMOD::Sound* mSound = nullptr;

	//사운드가 몇번 실행됬는지 저장하는 변수
	int mPlayCount = 0;

	//반복재생
	bool mLoop = false;

	//사운드가 종료됬을때 콜백함수
	std::function<void()> mSoundPlayEndFunc;

public:
	//사운드를 파일에서 읽어서 저장할 함수
	bool LoadSound(FMOD::System* System, FMOD::ChannelGroup* Group, bool Loop, const char* FileName, const std::string& PathName = "Sound", FMOD_MODE Mode = FMOD_2D);
	bool LoadSoundFullPath(FMOD::System* System, FMOD::ChannelGroup* Group, bool Loop, const char* FullPath, FMOD_MODE Mode = FMOD_2D);

	//사운드 제어함수
	void Play();
	void Stop();
	void Pause();
	void Resume();

	//3D 감쇠 연산용 함수
	void Set3DMinMaxDistance(float MinDist, float MaxDist);
	void Set3DAttributes(const FMOD_VECTOR& Pos, const FMOD_VECTOR& Vel = { 0.f,0.f,0.f });
	bool IsPlaying() const;


public:
	static FMOD_RESULT SoundPlayEnd(FMOD_CHANNELCONTROL* ChannelControl, FMOD_CHANNELCONTROL_TYPE ControlType,
		FMOD_CHANNELCONTROL_CALLBACK_TYPE CallBackType, void* CommandData1, void* CommandData2);

public:
	template<typename T>
	void SetSoundEndFunc(T* Object, void (T::* Func)())
	{
		mSoundPlayEndFunc = std::bind(Func, Object);
	}

};