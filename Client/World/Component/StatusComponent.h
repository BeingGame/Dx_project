#pragma once

#include "World/ActorComponent.h"

class CStatusComponent :
    public CActorComponent
{
public:
	CStatusComponent();
	CStatusComponent(const CStatusComponent& src);
	CStatusComponent(CStatusComponent&& src) noexcept;
	virtual ~CStatusComponent();

protected:
	float mHP = 10.f;
	float mHPMax = 10.f;

	std::function<void()> mDeathFunc;

	//피격/사망을 애니메이션 상태로 넘겨줄 상대. 없으면 그냥 HP만 깎인다.
	std::weak_ptr<class CActionStateComponent> mActionState;

public:
	float GetHP() const
	{
		return mHP;
	}

	float GetHPMax() const
	{
		return mHPMax;
	}

	void SetHPMax(float HPMax)
	{
		mHP = HPMax;
		mHPMax = HPMax;
	}

	//인스펙터에서 현재 체력만 조정할 때 쓴다. (피격/사망 동작은 일으키지 않는다)
	void SetHP(float HP)
	{
		mHP = HP;

		if (mHP > mHPMax) mHP = mHPMax;
		if (mHP < 0.f)    mHP = 0.f;
	}

	//최대 체력만 바꾼다. 현재 체력은 넘치지 않게만 잘라준다.
	void SetHPMaxOnly(float HPMax)
	{
		mHPMax = (HPMax < 1.f) ? 1.f : HPMax;

		if (mHP > mHPMax) mHP = mHPMax;
	}

	void AddHP(float HP);

	//피해를 입었을 때 액션 상태(피격/사망)를 같이 걸어줄지 정한다.
	void SetUseActionState(bool Use) { mbUseActionState = Use; }


public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Destroy();

	virtual void Save(std::ofstream& File) const override;
	virtual void Load(const std::unordered_map<std::string, std::string>& Props) override;

public:
	virtual CStatusComponent* Clone() const
	{
		return new CStatusComponent(*this);
	}

	virtual std::string GetTypeName() const override { return "CStatusComponent"; }

protected:
	bool mbUseActionState = true;

	//소유 액터에서 액션 상태 컴포넌트를 찾아 물린다.
	std::shared_ptr<class CActionStateComponent> FindActionState();

public:
	template<typename T>
	void SetDeathFunction(T* Object, void (T::* Func)())
	{
		mDeathFunc = std::bind(Func, Object);
	}

};

