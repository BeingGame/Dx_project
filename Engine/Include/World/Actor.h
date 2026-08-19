#pragma once

#include "../Object.h"

#include "SceneComponent.h"
#include "ActorComponent.h"

//액터 클래스
//컴포넌트를 받을수 있는 실제로 월드에 배치되는 클래스
//추상클래스로 설계할 필요는 없다.
//다만 컴포넌트 구조는 부모계층으로 되어있기 때문에 컴포넌트에서 작업을 할 필요성이 있다.

class CActor :
	public CObject
{
public:
	CActor();
	CActor(const CActor& src);
	CActor(CActor&& src) noexcept;
	virtual ~CActor();

protected:
	//소속된 월드
	std::weak_ptr<class CWorld> mWorld;

	//액터의 이름
	std::string mName;

	bool mRenderEnable = true;

	//루트 컴포넌트
	//최상위 씬 컴포넌트
	//weak_ptr인 이유는 실질적으로 컴포넌트는 배열로 저장되기 때문
	std::weak_ptr<CSceneComponent> mRoot;
	//배열로 사용하는 이유는
	//일괄적으로 처리하기 편함, 또한 인덱스로 바로 접근이 가능
	//캐시 적중률, 즉 효율이 좋아진다.
	std::vector<std::shared_ptr<CSceneComponent>> mSceneCompList;
	std::vector<std::shared_ptr<CActorComponent>> mActorCompList;

public:
	std::weak_ptr<class CWorld>GetWorld() const
	{
		return mWorld;
	}

	const std::string& GetName() const
	{
		return mName;
	}

	bool IsRenderEnable() const
	{
		return mRenderEnable;
	}

	const std::weak_ptr<class CSceneComponent> GetRootComponent() const
	{
		return mRoot;
	}


	void SetWorld(const std::weak_ptr<class CWorld>& World);

	void SetName(const std::string& Name)
	{
		mName = Name;
	}

	void SetRenderEnable(bool Enable)
	{
		mRenderEnable = Enable;
	}

public:
	//루트가 메시 같은 실체 있는 컴포넌트면, 그 위에 빈 씬 컴포넌트를 끼워 넣고
	//원래 루트를 자식으로 내린다. 이미 빈 루트면 아무것도 하지 않는다.
	//
	//필요한 이유: 이 판은 Y가 깊이 축이고 렌더러가 Y로 정렬한다.
	//점프/피격으로 몸을 띄울 때 액터 자체의 Y를 올리면 뒤로 물러난 것으로
	//취급되어 정렬과 판정이 어긋난다. 그래서 지면 위치(루트)와
	//화면에 보이는 높이(자식 메시)를 갈라놓아야 한다.
	std::weak_ptr<class CSceneComponent> InsertBareRoot(const std::string& Name = "Root");

	void AttachActorToActor(const std::weak_ptr<CActor> OtherActor);
	void DetachActorFromActor();

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Render();
	virtual void PostRender();
	virtual void Destroy();

	virtual float TakeDamage(float fDamage, const std::string& Instigator, const std::weak_ptr<CActor>& CauseActor);

public:
	void UpdateTransform();

public:
	FVector3 GetVelocity() const
	{
		auto Root = mRoot.lock();
		return Root ? Root->GetVelocity() : FVector3::Zero;
	}

	FVector3 GetWorldScale() const
	{
		auto Root = mRoot.lock();
		return Root ? Root->GetWorldScale() : FVector3::One;
	}

	FVector3 GetWorldRot() const
	{
		auto Root = mRoot.lock();
		return Root ? Root->GetWorldRot() : FVector3::Zero;
	}

	FVector3 GetWorldPos() const
	{
		auto Root = mRoot.lock();
		return Root ? Root->GetWorldPos() : FVector3::Zero;
	}

	FVector3 GetRelativeScale() const
	{
		auto Root = mRoot.lock();
		return Root ? Root->GetRelativeScale() : FVector3::One;
	}

	FVector3 GetRelativeRot() const
	{
		auto Root = mRoot.lock();
		return Root ? Root->GetRelativeRot() : FVector3::Zero;
	}

	FVector3 GetRelativePos() const
	{
		auto Root = mRoot.lock();
		return Root ? Root->GetRelativePos() : FVector3::Zero;
	}

	FVector3 GetAxis(EAxis::Type Axis) const
	{
		auto Root = mRoot.lock();
		return Root ? Root->GetAxis(Axis) : FVector3::Axis[Axis];
	}

	void SetRelativeScale(const FVector3& Scale);
	void SetRelativeScale(const FVector2& Scale);
	void SetRelativeScale(float x, float y, float z);
	void SetRelativeScale(float x, float y);

	void AddRelativeScale(const FVector3& Scale);
	void AddRelativeScale(const FVector2& Scale);
	void AddRelativeScale(float x, float y, float z);
	void AddRelativeScale(float x, float y);

	void SetRelativeRotation(const FVector3& Rot);
	void SetRelativeRotation(const FVector2& Rot);
	void SetRelativeRotation(float x, float y, float z);
	void SetRelativeRotation(float x, float y);
	void SetRelativeRotationX(float x);
	void SetRelativeRotationY(float y);
	void SetRelativeRotationZ(float z);
	void AddRelativeRotation(const FVector3& Rot);
	void AddRelativeRotation(const FVector2& Rot);
	void AddRelativeRotation(float x, float y, float z);
	void AddRelativeRotation(float x, float y);
	void AddRelativeRotationX(float x);
	void AddRelativeRotationY(float y);
	void AddRelativeRotationZ(float z);

	void SetRelativePos(const FVector3& Pos);
	void SetRelativePos(const FVector2& Pos);
	void SetRelativePos(float x, float y, float z);
	void SetRelativePos(float x, float y);

	void AddRelativePos(const FVector3& Pos);
	void AddRelativePos(const FVector2& Pos);
	void AddRelativePos(float x, float y, float z);
	void AddRelativePos(float x, float y);

	//월드 트랜스폼
	void SetWorldScale(const FVector3& Scale);
	void SetWorldScale(const FVector2& Scale);
	void SetWorldScale(float x, float y, float z);
	void SetWorldScale(float x, float y);

	void AddWorldScale(const FVector3& Scale);
	void AddWorldScale(const FVector2& Scale);
	void AddWorldScale(float x, float y, float z);
	void AddWorldScale(float x, float y);

	void SetWorldRotation(const FVector2& Rot);
	void SetWorldRotation(const FVector3& Rot);
	void SetWorldRotation(float x, float y, float z);
	void SetWorldRotation(float x, float y);
	void SetWorldRotationX(float x);
	void SetWorldRotationY(float y);
	void SetWorldRotationZ(float z);

	void AddWorldRotation(const FVector2& Rot);
	void AddWorldRotation(const FVector3& Rot);
	void AddWorldRotation(float x, float y, float z);
	void AddWorldRotation(float x, float y);
	void AddWorldRotationX(float x);
	void AddWorldRotationY(float y);
	void AddWorldRotationZ(float z);

	void SetWorldPos(const FVector3& Pos);
	void SetWorldPos(const FVector2& Pos);
	void SetWorldPos(float x, float y, float z);
	void SetWorldPos(float x, float y);

	void AddWorldPos(const FVector3& Pos);
	void AddWorldPos(const FVector2& Pos);
	void AddWorldPos(float x, float y, float z);
	void AddWorldPos(float x, float y);

protected:
	virtual CActor* Clone()
	{
		return new CActor(*this);
	}

public:
	template<typename T>
	std::weak_ptr<T> CreateComponent(const std::string& Name, const std::string& ParentName = "Root")
	{
		//처음에 생성한 컴포넌트는 반드시 Root가된다.
		//단 씬컴포넌트만

		std::shared_ptr<CComponent> Component = std::make_shared<T>();

		Component->SetName(Name);
		Component->SetWorld(mWorld);
		Component->SetOwner(GetThisPtr<CActor>());

		if (!Component->Init())
		{
			return std::weak_ptr<T>();
		}

		if (Component->GetCompType() == EComponentType::Scene)
		{
			//루트가 없으면 루트로 변경
			if (mSceneCompList.empty())
			{
				mRoot = std::dynamic_pointer_cast<CSceneComponent>(Component);
			}
			else
			{
				//부모의 이름을 확인한뒤 addchild로 부착시켜준다.

				if (ParentName == "Root")
				{
					auto Root = mRoot.lock();

					if (Root)
					{
						Root->AddChild(std::dynamic_pointer_cast<CSceneComponent>(Component));
					}
				}
				else
				{
					size_t Size = mSceneCompList.size();

					std::shared_ptr<CSceneComponent> Parent;

					for (size_t i = 0; i < Size; ++i)
					{
						if (mSceneCompList[i]->GetName() == ParentName)
						{
							Parent = mSceneCompList[i];
							break;
						}
					}


					if (!Parent)
					{
						auto Root = mRoot.lock();

						if (Root)
						{
							Root->AddChild(std::dynamic_pointer_cast<CSceneComponent>(Component));
						}
					}
					else
					{
						Parent->AddChild(std::dynamic_pointer_cast<CSceneComponent>(Component));
					}
				}
			}

			//부착이 완료되면 리스트에 추가해준다.
			mSceneCompList.push_back(std::dynamic_pointer_cast<CSceneComponent>(Component));

		}
		else
		{
			mActorCompList.push_back(std::dynamic_pointer_cast<CActorComponent>(Component));
		}


		return std::dynamic_pointer_cast<T>(Component);
	}

public:
	//---- 컴포넌트 제거 ----
	//씬 컴포넌트를 제거한다.
	//제거 대상의 자식들은 대상의 부모(루트가 제거되는 경우엔 승격된 새 루트)로 재부착된다.
	//루트를 제거하면 첫 번째 자식이 새 루트로 승격되고, 자식이 없으면 루트는 비워진다.
	//리스트에서 erase한 뒤에도 대상이 유효해야 하므로 값으로 받는다.
	bool RemoveSceneComponent(std::shared_ptr<CSceneComponent> Comp);

	//액터 컴포넌트를 제거한다.
	bool RemoveActorComponent(std::shared_ptr<CActorComponent> Comp);

	//타입에 상관없이 제거한다. (씬/액터 자동 판별)
	bool RemoveComponent(std::shared_ptr<CComponent> Comp);

	//이름으로 제거한다. 씬 컴포넌트를 먼저 탐색한다.
	bool RemoveComponent(const std::string& Name);

	//임시코드
protected:
	//원충돌을 위한 radius
	//float mRadius = 0.f;

	//액터 태그를 만들때 enum 혹은 string 형식으로 만든다.
	//enum 확장성이 매우 구리다
	//다만 쓰기 편함
	//string 필요하다면 액터마다 다르게 사용할수 있다.

	std::string mActorTag = "Actor";

public:
	const std::string& GetActorTag() const
	{
		return mActorTag;
	}

	void SetActorTag(const std::string& Tag)
	{
		mActorTag = Tag;
	}

	virtual std::string GetTypeName() const { return "CActor"; }

	const std::vector<std::shared_ptr<CSceneComponent>>& GetSceneCompList() const
	{
		return mSceneCompList;
	}

	const std::vector<std::shared_ptr<CActorComponent>>& GetActorCompList() const
	{
		return mActorCompList;
	}

	std::string GetParentName(const std::shared_ptr<CSceneComponent>& Comp) const;

	void SaveScene(std::ofstream& File, int ActorIdx) const;
	void LoadSceneComp(std::shared_ptr<CSceneComponent> Comp, const std::string& Parent);
	void LoadActorComp(std::shared_ptr<CActorComponent> Comp);



public:
	//더이상 사용하지 않는다.
	//void Collision();
	virtual void OnCollision(const std::weak_ptr<CActor>& HitActor);

};

