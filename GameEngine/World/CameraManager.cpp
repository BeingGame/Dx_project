#include "CameraManager.h"
#include "CameraComponent.h"

CCameraManager::CCameraManager()
{}

CCameraManager::~CCameraManager()
{}

const FMatrix& CCameraManager::GetViewMat() const
{
	// TODO: 여기에 return 문을 삽입합니다.
	
	auto MainCamera = mMainCamera.lock();

	if (MainCamera)
	{
		return MainCamera->GetViewMat();
	}

	return FMatrix::IdentityMatrix;

}

const FMatrix& CCameraManager::GetProjMat() const
{
	// TODO: 여기에 return 문을 삽입합니다.

	auto MainCamera = mMainCamera.lock();

	if (MainCamera)
	{
		return MainCamera->GetProjMat();
	}

	return FMatrix::IdentityMatrix;
}

const FVector3& CCameraManager::GetMainCameraPos() const
{
	// TODO: 여기에 return 문을 삽입합니다.

	auto MainCamera = mMainCamera.lock();

	if (MainCamera)
	{
		return MainCamera->GetWorldPos();
	}

	return FVector3::Zero;
}

bool CCameraManager::Init()
{
	return true;
}

void CCameraManager::Update(float DeltaTime)
{
	//유효하지않은 카메라를 제거한다.
	auto iter = mCameraMap.begin();
	auto iterEnd = mCameraMap.end();

	for (; iter != iterEnd;)
	{
		if (iter->second.expired())
		{
			iter = mCameraMap.erase(iter);
			continue;
		}

		++iter;
	}


	//메인카메라가 없으면 카메라 맵을 탐색해서 첫번째 카메라로 변경한다.
	if (mMainCamera.expired())
	{
		if (!mCameraMap.empty())
		{
			mMainCamera = mCameraMap.begin()->second;
		}
	}
}

void CCameraManager::AddCamera(std::string Name, const std::weak_ptr<class CCameraComponent>&Camera)
{
	if (mMainCamera.expired())
	{
		mMainCamera = Camera;
	}

	mCameraMap.insert(std::make_pair(Name, Camera));
}

void CCameraManager::ChangeMainCamera(const std::weak_ptr<class CCameraComponent>&Camera)
{
	if (Camera.expired())
	{
		return;
	}

	mMainCamera = Camera;
}

void CCameraManager::ChangeMainCamera(const std::string & Name)
{
	auto iter = mCameraMap.find(Name);

	if (iter == mCameraMap.end())
	{
		return;
	}

	mMainCamera = iter->second;
}