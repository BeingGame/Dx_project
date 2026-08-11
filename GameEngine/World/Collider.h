#pragma once

#include "SceneComponent.h"

//Collider 클래스를 추상클래스로 만들어
//함수 처리할때 하나의 클래스로만 할수 있게 만든다.


class CCollider :
	public CSceneComponent
{
public:
	CCollider();
	CCollider(const CCollider& src);
	CCollider(CCollider&& src) noexcept;
	virtual ~CCollider();

protected:
	EColliderType mColliderType;

	//렌더링 디버그기능
	bool mDebugDraw = false;

	//충돌상태 체크 변수
	bool mCollision = false;

	//콜리전의 크기
	FVector3 mMin;
	FVector3 mMax;

	std::shared_ptr<FCollisionProfile> mProfile = nullptr;

	//현재 충돌상태인 충돌체를 보관할 자료구조
	//충돌체가 소멸될때 endoverlap 호출하기 위해서
	std::unordered_map<CCollider*, std::weak_ptr<CCollider>> mColliderMap;

	//디버그 기능 렌더링
	std::weak_ptr<class CShader> mShader;
	std::weak_ptr<class CMesh> mMesh;

	std::shared_ptr<class CCBufferTransform> mCBufferTransform;

	//상수버퍼 충돌체의 색상을 변경하기위한 버퍼
	std::shared_ptr<class CCBufferCollider> mCBufferCollider;

	//충돌했을때 overlap(겹침)
	std::function<void(const FVector3&, const FVector3&, std::weak_ptr<CCollider>)> mBeginOverlapFunc;
	std::function<void(std::weak_ptr<CCollider>)> mEndOverlapFunc;

	//충돌했을때 Hit(충돌)
	std::function<void(const FVector3&, const FVector3&, std::weak_ptr<CCollider>)> mHitFunc;

public:
	const FVector3& GetMin() const
	{
		return mMin;
	}

	const FVector3& GetMax() const
	{
		return mMax;
	}

	EColliderType GetColliderType() const
	{
		return mColliderType;
	}

	bool GetDebugDraw() const
	{
		return mDebugDraw;
	}

	std::shared_ptr<FCollisionProfile> GetCollisionProfile() const
	{
		return mProfile;
	}

	virtual void SetDebugDraw(bool DebugDraw);
	void SetCollisionProfile(const std::string& Name);


	//현재 충돌중인 오브젝트를 판단해서 추가하고, 제거해주는 함수
	bool CheckCollisionObject(std::weak_ptr<CCollider> Collider);
	std::unordered_map<CCollider*, std::weak_ptr<CCollider>>::iterator EraseCollisionObject(std::weak_ptr<CCollider> Collider);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Render();
	virtual void Destroy();

	//충돌시 정보변경을 할 함수
	//postupdate이후에 호출해 정보를 갱신한다.
	virtual void UpdateInfo() = 0;

public:
	virtual CCollider* Clone() const = 0;

public:
	//충돌했을때 콜백함수 호출
	void CallCollisionBeginOverlap(const FVector3& HitPoint, const FVector3& Normal, std::weak_ptr<CCollider> Collider);
	void CallCollisionEndOverlap(std::weak_ptr<CCollider> Collider);
	void CallCollisionHit(const FVector3& HitPoint, const FVector3& Normal, std::weak_ptr<CCollider> Collider);

public:
	virtual bool Collision(FVector3& HitPoint, FVector3& Normal, float& Depth, std::shared_ptr<CCollider>& Dest) = 0;
	virtual bool CollisionMouse(const FVector2& MousePos) = 0;

public:
	//콜백함수 등록용 함수
	template<typename T>
	void SetBeginOverlapFunc(T* Obj, void (T::* Func)(const FVector3&, const FVector3&, std::weak_ptr<CCollider>))
	{
		mBeginOverlapFunc = std::bind(Func, Obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	}

	template<typename T>
	void SetEndOverlapFunc(T* Obj, void (T::* Func)(std::weak_ptr<CCollider>))
	{
		mEndOverlapFunc = std::bind(Func, Obj, std::placeholders::_1);
	}

	template<typename T>
	void SetHitFunc(T* Obj, void (T::* Func)(const FVector3&, const FVector3&, std::weak_ptr<CCollider>))
	{
		mHitFunc = std::bind(Func, Obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	}

};

