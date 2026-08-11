#include "SoundComponent.h"
#include "../Asset/SoundManager.h"
#include "../Asset/AssetManager.h"

CSoundComponent::CSoundComponent()
{}

CSoundComponent::CSoundComponent(const CSoundComponent& src)
	:CSceneComponent(src)
{}

CSoundComponent::CSoundComponent(CSoundComponent&& src) noexcept
	:CSceneComponent(std::move(src))
{}

CSoundComponent::~CSoundComponent()
{}

bool CSoundComponent::SetSound(const std::string& Name, const std::string& GroupName, bool Loop, const char* FileName, ESoundDimension Dimension, const std::string& PathName)
{
	auto SoundMgr = CAssetManager::GetInst()->GetSubManager<CSoundManager>(EAssetType::Sound);

	if (!SoundMgr)
	{
		return false;
	}

	FMOD_MODE Mode = FMOD_2D;

	switch (Dimension)
	{
	case ESoundDimension::Mode3D:
		//기본 3D 감쇠: maxDistance에서도 최저 볼륨 유지
		Mode = FMOD_3D;
		break;
	case ESoundDimension::Mode3D_Linear:
		//선형 3D 감쇠 : maxDistance에서 무음 처리가 됨
		Mode = FMOD_3D | FMOD_3D_LINEARROLLOFF;
		break;
	default:
		break;
	}

	if (!SoundMgr->LoadSound(Name, GroupName, Loop, FileName, PathName, Mode))
	{
		return false;
	}

	mSound = SoundMgr->FindSound(Name).lock();

	if (mSound)
	{
		mSound->Set3DMinMaxDistance(mMinDistance, mMaxDistance);
	}

	return true;
}

void CSoundComponent::SetMinMaxDistance(float MinDist, float MaxDist)
{
	mMinDistance = MinDist;
	mMaxDistance = MaxDist;

	if (mSound)
	{
		mSound->Set3DMinMaxDistance(MinDist, MaxDist);
	}
}

void CSoundComponent::Play()
{
	if (mSound)
	{
		mSound->Play();

		//재생 시 시작위치 동기화
		FVector3 WorldPos = GetWorldPos();
		FMOD_VECTOR Pos = { WorldPos.x, WorldPos.y, WorldPos.z };
		FMOD_VECTOR Vel = { 0.f,0.f,0.f };

		mSound->Set3DAttributes(Pos, Vel);
	}
}

void CSoundComponent::Stop()
{
	if (mSound)
	{
		mSound->Stop();
	}
}

void CSoundComponent::Pause()
{
	if (mSound)
	{
		mSound->Pause();
	}
}

void CSoundComponent::Resume()
{
	if (mSound)
	{
		mSound->Resume();
	}
}

bool CSoundComponent::IsPlaying()
{
	if (mSound)
	{
		return mSound->IsPlaying();
	}

	return false;
}

bool CSoundComponent::Init()
{
	CSceneComponent::Init();

	return true;
}

void CSoundComponent::Update(float DeltaTime)
{
	CSceneComponent::Update(DeltaTime);
}

void CSoundComponent::PostUpdate(float DeltaTime)
{
	CSceneComponent::PostUpdate(DeltaTime);

	//액터가 움직일때 매 프레임 소리 발생 지점을 FMOD 채널에 갱신

	if (mSound && mSound->IsPlaying())
	{
		//재생 시 시작위치 동기화
		FVector3 WorldPos = GetWorldPos();
		FMOD_VECTOR Pos = { WorldPos.x, WorldPos.y, WorldPos.z };
		FMOD_VECTOR Vel = { 0.f,0.f,0.f };

		mSound->Set3DAttributes(Pos, Vel);
	}

}

CSoundComponent* CSoundComponent::Clone() const
{
	return new CSoundComponent(*this);
}
