#pragma once
#include "SceneComponent.h"

enum class ECameraProjectionType
{
	Perspective,
	Ortho
};

class CCameraComponent :
    public CSceneComponent
{
public:
	CCameraComponent();
	CCameraComponent(const CCameraComponent& src);
	CCameraComponent(CCameraComponent&& src) noexcept;
	virtual ~CCameraComponent();

protected:
	//카메라컴포넌트에서 중요한건 카메라의 위치는 씬컴포넌트에서 상속받는다.
	//투영행렬을 구성하기 위한 거리와 해상도와 거리도 필요하다.

	FMatrix mViewMat;
	FMatrix mProjMat;

	ECameraProjectionType mProjType = ECameraProjectionType::Ortho;
	float mViewAngle = 90.f;
	float mWidth = 1280.f;
	float mHeight = 720.f;
	float mViewDistance = 1000.f;

public:
	const FMatrix& GetViewMat() const
	{
		return mViewMat;
	}

	const FMatrix& GetProjMat() const
	{
		return mProjMat;
	}

	//투영행렬을 생성하는 함수
	void SetProjection(ECameraProjectionType Type, float ViewAngle, float Width, float Height, float ViewDistance);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);

public:
	virtual CCameraComponent* Clone() const;
};

