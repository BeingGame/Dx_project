#pragma once

#include "../Object.h"

//컴포넌트의 최상위 클래스
//액터와는 다르게 추상클래스로 설계한다.

//컴포넌트
//컴포넌트는 설계를 바탕으로 조립되는 부품
//오브젝트(액터)마다 필요한 기능이 다를수 있기 때문에, 미리 만들어둔
//컴포넌트를 이용해서 다른 오브젝트(액터)에서도 재사용할수 있어 효율적이다.

//언리얼 엔진과 유사한 구조를 사용하기 때문에 두가지 컴포넌트를 사용한다.

//씬 컴포넌트
//트랜스폼 정보를 가진다.(트랜스폼 :크기 회전 정보를 가지는 구조체)
//해당 컴포넌트는 계층 구조를 가진다.
//자식 컴포넌트는 부모 컴포넌트의 트랜스폼에 영향을 받는다.

//액터 컴포넌트(오브젝트 컴포넌트)
//기능 구현을 중심으로 만들어진다.
//씬 컴포넌트 다르게 계층구조로 설계하지 않는다.

//컴포넌트를 조합해서 하나의 액터를 만들게 된다.
//월드의 배치는 씬컴포넌트를 이용, 기능은 액터 컴포넌트로 구현하게된다.

//설계 순서 컴포넌트 -> 액터 -> 월드 -> 월드매니저

enum class EComponentType
{
    Scene,
    Actor
};

class CComponent :
    public CObject
{
public:
	CComponent();
	CComponent(const CComponent& src);
	CComponent(CComponent&& src) noexcept;
	virtual ~CComponent() = 0;

protected:
	//소속된 월드
	std::weak_ptr<class CWorld> mWorld;

	//컴포넌트를 소유한 액터
	std::weak_ptr<class CActor> mOwner;

	//컴포넌트의 이름
	std::string mName;
	//컴포넌트의 타입
	EComponentType mType = EComponentType::Scene;

	//필요에 따라 컴포넌트를 렌더링할지 말지 선택한다.
	bool mRenderEnable = true;

public:
	std::weak_ptr<class CWorld> GetWorld() const
	{
		return mWorld;
	}

	std::weak_ptr<class CActor> GetOwner() const
	{
		return mOwner;
	}

	const std::string& GetName() const
	{
		return mName;
	}

	EComponentType GetCompType() const
	{
		return mType;
	}

	bool IsRenderEnable() const
	{
		return mRenderEnable;
	}

	void SetWorld(const std::weak_ptr<class CWorld>& World);
	void SetOwner(const std::weak_ptr<class CActor>& Owner);

	void SetName(const std::string& Name)
	{
		mName = Name;
	}

	void SetCompType(EComponentType Type)
	{
		mType = Type;
	}

	void SetRenderEnable(bool Enable)
	{
		mRenderEnable = Enable;
	}

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Render();
	virtual void PostRender();
	virtual void Destroy();

public:
	virtual CComponent* Clone() const = 0;

public:
	template<typename T>
	std::shared_ptr<T> GetThisPtr()
	{
		return std::dynamic_pointer_cast<T>(shared_from_this());
	}

};