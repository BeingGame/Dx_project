#include "StatusComponent.h"
#include "ActionStateComponent.h"

#include "World/Actor.h"

#include <fstream>

CStatusComponent::CStatusComponent()
{}

CStatusComponent::CStatusComponent(const CStatusComponent& src)
	:CActorComponent(std::move(src))
{}

CStatusComponent::CStatusComponent(CStatusComponent&& src) noexcept
	:CActorComponent(std::move(src))
{}

CStatusComponent::~CStatusComponent()
{}

std::shared_ptr<CActionStateComponent> CStatusComponent::FindActionState()
{
	if (!mbUseActionState)
	{
		return nullptr;
	}

	if (auto Cached = mActionState.lock())
	{
		return Cached;
	}

	auto Owner = mOwner.lock();

	if (!Owner)
	{
		return nullptr;
	}

	for (const auto& Comp : Owner->GetActorCompList())
	{
		auto State = std::dynamic_pointer_cast<CActionStateComponent>(Comp);

		if (State)
		{
			mActionState = State;
			return State;
		}
	}

	return nullptr;
}

void CStatusComponent::AddHP(float HP)
{
	mHP += HP;

	if (mHP <= 0.f)
	{
		//사망 상태를 먼저 걸어둔다.
		//DeathFunc이 액터를 지워버리는 경우가 있어서 순서가 중요하다.
		if (auto State = FindActionState())
		{
			State->RequestDead();
		}

		if (mDeathFunc)
		{
			mDeathFunc();
		}
	}
	else
	{
		//회복이 아니라 피해를 입었을 때만 피격 동작을 낸다.
		if (HP < 0.f)
		{
			if (auto State = FindActionState())
			{
				State->RequestHit();
			}
		}

		if (mHP > mHPMax)
		{
			mHP = mHPMax;
		}
	}
}

bool CStatusComponent::Init()
{
	CActorComponent::Init();

	return true;
}

void CStatusComponent::Update(float DeltaTime)
{
	CActorComponent::Update(DeltaTime);
}

void CStatusComponent::PostUpdate(float DeltaTime)
{
	CActorComponent::PostUpdate(DeltaTime);
}

void CStatusComponent::Destroy()
{
	CActorComponent::Destroy();
}

void CStatusComponent::Save(std::ofstream& File) const
{
	CActorComponent::Save(File);

	File << "HPMax=" << mHPMax << "\n";
	File << "HP="    << mHP    << "\n";
}

void CStatusComponent::Load(const std::unordered_map<std::string, std::string>& Props)
{
	CActorComponent::Load(Props);

	auto GetF = [&Props](const char* Key, float& Out)
	{
		auto Found = Props.find(Key);
		if (Found == Props.end()) return;
		try { Out = std::stof(Found->second); }
		catch (...) {}
	};

	GetF("HPMax", mHPMax);
	GetF("HP",    mHP);

	if (mHPMax < 1.f)  mHPMax = 1.f;
	if (mHP > mHPMax)  mHP = mHPMax;
}
