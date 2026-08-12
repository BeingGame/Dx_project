#pragma once

#include "SceneComponent.h"
#include "../Asset/Sound.h"

class CSoundComponent :
    public CSceneComponent
{

public:
	CSoundComponent();
	CSoundComponent(const CSoundComponent& src);
	CSoundComponent(CSoundComponent&& src) noexcept;
	virtual ~CSoundComponent();

public:
	//사운드 에셋
	std::shared_ptr<CSound> mSound;



	//최소 감쇠
	//이 거리 미만의 소리는 감쇠되지 않고 100퍼센트의 볼륨
	float mMinDistance = 100.f;

	//최대 감쇠
	//이 거리 이상 멀어지면 볼륨을 비율대로 처리한다.
	float mMaxDistance = 1000.f;

public:
	bool SetSound(const std::string& Name, const std::string& GroupName, bool Loop, const char* FileName, ESoundDimension Dimension = ESoundDimension::Mode2D, const std::string& PathName = "Sound");

	//3D 사운드의 최대 최소 거리
	void SetMinMaxDistance(float MinDist, float MaxDist);

	void Play();
	void Stop();
	void Pause();
	void Resume();
	bool IsPlaying();

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);

public:
	virtual CSoundComponent* Clone() const;
	virtual std::string GetTypeName() const override { return "CSoundComponent"; }
};

