#include "CameraComponent.h"

#include "../Device.h"
#include "World.h"
#include "CameraManager.h"
#include "../Asset/SoundManager.h"
#include "../Asset/AssetManager.h"

CCameraComponent::CCameraComponent()
{}

CCameraComponent::CCameraComponent(const CCameraComponent& src)
	:CSceneComponent(src)
{}

CCameraComponent::CCameraComponent(CCameraComponent&& src) noexcept
	:CSceneComponent(std::move(src))
{}

CCameraComponent::~CCameraComponent()
{}

void CCameraComponent::SetProjection(ECameraProjectionType Type, float ViewAngle, float Width, float Height, float ViewDistance)
{
	mProjType = Type;
	mViewAngle = ViewAngle;
	mWidth = Width;
	mHeight = Height;
	mViewDistance = ViewDistance;

	//해상도의 절반만큼의 -를 좌로, 절반만큼을 우로 둔다.
	//2D환경인 직교투영으로 만들어준다.
	switch (mProjType)
	{
	case ECameraProjectionType::Perspective:
		mProjMat = DirectX::XMMatrixPerspectiveFovLH(mViewAngle, mWidth / mHeight, 0.f, mViewDistance);
		break;
	case ECameraProjectionType::Ortho:
		mProjMat = DirectX::XMMatrixOrthographicOffCenterLH(mWidth / -2.f, mWidth / 2.f, mHeight / -2.f, mHeight / 2.f, 0.f, mViewDistance);
		break;
	}
}

bool CCameraComponent::Init()
{
	FResolution RS = CDevice::GetInst()->GetResolution();

	SetProjection(ECameraProjectionType::Ortho, 0.f, (float)RS.Width, (float)RS.Height, 1000.f);

	auto World = mWorld.lock();

	if (World)
	{
		auto CameraManager = World->GetCameraManager().lock();

		if (CameraManager)
		{
			CameraManager->AddCamera(mName, GetThisPtr<CCameraComponent>());
		}
	}
	

	return true;
}

void CCameraComponent::Update(float DeltaTime)
{
	CSceneComponent::Update(DeltaTime);
}

void CCameraComponent::PostUpdate(float DeltaTime)
{
	//Direct를 이용한 뷰행렬 만들기
	//카메라의 정면, 카메라의 위쪽, 카메라의 위치를 이용해서
	//다이렉트 함수를 사용해 간단히 만들수 있다.

	//뷰행렬은 카메라를 원점으로 돌리기위한 행렬
	//뷰 변환이 이뤄지는 이유는 카메라가 원점에서 정면을 바라보는 기준으로 만들면
	//계산이 쉬워지기 때문

	//카메라의 트랜스폼을 이용해 카메라의 회전과 이동을 상쇄시켜
	//카메라의 위치를 원점(0,0,0)으로 이동시키고 카메라가 정면을 바라보게(0,0,1)을 바라보도록 만들어준다.

	//카메라의 월드공간에서 축값에 뷰행렬을 곱하면 다음과 같아진다.
	/*
	WXx		WXy		WXz * 뷰행렬	= 1 0 0
	WYx		WYy		WYz	* 뷰행렬	= 0 1 0
	WZx		WZy		WZz	* 뷰행렬	= 0 0 1

	행렬 * 역행렬 = 항등행렬
	
	직교행렬의 역행렬은 직교행렬을 전치한것과 같다.
	따라서 회전행렬을 전치해서 카메라가 원점을 바라보게 만든다.

	WXx		WYx		WZx	0
	WXy		WYy		WZy	0
	WXz		WYz		WZz	0
	-x		-y		-z	1
	이동의 경우는 이동한 값만큼 빼주면 원점으로 이동한다.
	*/

	//뷰행렬을 생성한다.
	mViewMat.Identity();

	for (int i = 0; i < EAxis::End; ++i)
	{
		//카메라의 축을 행렬에 복사한다.
		memcpy(&mViewMat[i], &mWorldAxis[i], sizeof(FVector3));
	}

	/*
	WXx		WXy		WXz
	WYx		WYy		WYz
	WZx		WZy		WZz
	*/

	//전치시켜 회전행렬의 역행렬을 만들어준다.
	mViewMat.Transpose();

	for (int i = 0; i < EAxis::End; ++i)
	{
		//위치값을 넣어줄때 축과 현재 위치값을 내적해서 넣어준다.
		mViewMat[3][i] = -mWorldPos.DotProduct(mWorldAxis[i]);
	}

	auto World = mWorld.lock();

	if (World)
	{
		auto CameraMgr = World->GetCameraManager().lock();

		if (CameraMgr && CameraMgr->GetMainCamera().lock().get() == this)
		{
			//감쇠 시 듣는이(리스너)를 설정해줘서 
			//메인 카메라 위치를 리스너로 등록해준다.
			//리스너: 현재 사운드를 들을 액터의 위치를 포함한 속성

			auto SoundMgr = CAssetManager::GetInst()->GetSubManager<CSoundManager>(EAssetType::Sound);

			if (SoundMgr)
			{
				SoundMgr->SetListenerAttribute(mWorldPos, mVelocity, mWorldAxis[EAxis::Z], mWorldAxis[EAxis::Y]);
			}

		}
	}


	CSceneComponent::PostUpdate(DeltaTime);
}

CCameraComponent* CCameraComponent::Clone() const
{
	return new CCameraComponent(*this);
}
