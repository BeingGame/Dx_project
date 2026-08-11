#include "WorldManager.h"

CWorldManager* CWorldManager::mInstance = nullptr;

CWorldManager::CWorldManager()
{

}

CWorldManager::~CWorldManager()
{

}

std::weak_ptr<CWorld> CWorldManager::GetWorld() const
{
    return mWorld;
}

bool CWorldManager::Init()
{

    return true;
}

bool CWorldManager::Update(float DeltaTime)
{
    if (mWorld)
    {
        mWorld->Update(DeltaTime);

        return ChangeWorld();
    }

    return false;
}

bool CWorldManager::PostUpdate(float DeltaTime)
{
    if (mWorld)
    {
        mWorld->PostUpdate(DeltaTime);

        return ChangeWorld();
    }

    return false;
}

void CWorldManager::Render()
{
    if (mWorld)
    {
        mWorld->Render();
    }

}

void CWorldManager::PostRender()
{
    if (mWorld)
    {
        mWorld->PostRender();
    }
}

void CWorldManager::RenderUI()
{
    if (mWorld)
    {
        mWorld->RenderUI();
    }
}

bool CWorldManager::ChangeWorld()
{
    if (mNextWorld)
    {
        mWorld = mNextWorld;

        mNextWorld.reset();

        return true;
    }

    return false;
}
