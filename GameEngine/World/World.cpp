#include "World.h"
#include "CameraManager.h"
#include "Input.h"
#include "WorldCollision.h"
#include "UIManager.h"

CWorld::CWorld()
{}

CWorld::~CWorld()
{
	for (FTimerHandle& Handle : mTimerHandleList)
	{
		CTimeManager::ClearTimer(Handle);
	}

	mTimerHandleList.clear();

}

void CWorld::InputActive()
{
	mInput->DeviceAcquire();
}

void CWorld::InputDeActive()
{
	mInput->DeviceUnAcquire();
}

bool CWorld::Init()
{
	mCameraManager = std::make_shared<CCameraManager>();

	if (!mCameraManager->Init())
	{
		return false;
	}

	mInput = std::make_shared<CInput>();
	mInput->SetWorld(GetThisPtr());

	if (!mInput->Init())
	{
		return false;
	}

	mCollision = std::make_shared<CWorldCollision>();
	mCollision->SetWorld(GetThisPtr());

	if (!mCollision->Init())
	{
		return false;
	}

	mUIManager = std::make_shared<CUIManager>();
	mUIManager->SetWorld(GetThisPtr());

	if (!mUIManager->Init())
	{
		return false;
	}
	return true;
}

void CWorld::Update(float DeltaTime)
{
	mInput->Update(DeltaTime);

	auto iter = mActorList.begin();
	auto iterEnd = mActorList.end();

	for (; iter != iterEnd;)
	{
		if (!iter->second->IsAlive())
		{
			iter = mActorList.erase(iter);
			iterEnd = mActorList.end();
			continue;
		}
		else if (!iter->second->IsEnable())
		{
			++iter;
			continue;
		}

		iter->second->Update(DeltaTime);
		++iter;
	}

	//유효한 카메라들만 남기도록 업데이트
	mCameraManager->Update(DeltaTime);

	mUIManager->Update(DeltaTime);
}

void CWorld::PostUpdate(float DeltaTime)
{
	mCollision->UpdateInfo();

	auto iter = mActorList.begin();
	auto iterEnd = mActorList.end();

	for (; iter != iterEnd;)
	{
		if (!iter->second->IsAlive())
		{
			iter = mActorList.erase(iter);
			iterEnd = mActorList.end();
			continue;
		}
		else if (!iter->second->IsEnable())
		{
			++iter;
			continue;
		}

		iter->second->PostUpdate(DeltaTime);
		++iter;
	}

	mCollision->Update(DeltaTime);

	iter = mActorList.begin();
	iterEnd = mActorList.end();

	for (; iter != iterEnd;)
	{
		if (!iter->second->IsAlive())
		{
			iter = mActorList.erase(iter);
			iterEnd = mActorList.end();
			continue;
		}
		else if (!iter->second->IsEnable())
		{
			++iter;
			continue;
		}

		iter->second->UpdateTransform();
		++iter;
	}


}

void CWorld::Render()
{
	auto iter = mActorList.begin();
	auto iterEnd = mActorList.end();

	for (; iter != iterEnd;)
	{
		if (!iter->second->IsAlive())
		{
			iter = mActorList.erase(iter);
			iterEnd = mActorList.end();
			continue;
		}
		else if (!iter->second->IsEnable())
		{
			++iter;
			continue;
		}

		iter->second->Render();
		++iter;
	}

#ifdef _DEBUG

	mCollision->Render();

#endif

	//연산이 종료된 충돌체의 오브젝트를 풀에 반환한다.
	mCollision->ReturnNodePool();

}

void CWorld::PostRender()
{
	auto iter = mActorList.begin();
	auto iterEnd = mActorList.end();

	for (; iter != iterEnd;)
	{
		if (!iter->second->IsAlive())
		{
			iter = mActorList.erase(iter);
			iterEnd = mActorList.end();
			continue;
		}
		else if (!iter->second->IsEnable())
		{
			++iter;
			continue;
		}

		iter->second->PostRender();
		++iter;
	}

}

void CWorld::RenderUI()
{
	mUIManager->Render();
}