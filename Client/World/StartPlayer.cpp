#include "StartPlayer.h"

#include "MainWorld.h"
#include "EditorWorld.h"
#include "World/WorldManager.h"
#include "World/Input.h"

CStartPlayer::CStartPlayer()
{}

CStartPlayer::CStartPlayer(const CStartPlayer & src)
{}

CStartPlayer::CStartPlayer(CStartPlayer && src) noexcept
{}

CStartPlayer::~CStartPlayer()
{}

bool CStartPlayer::Init()
{
	auto World = mWorld.lock();

	auto Input = World->GetInput().lock();

	if (Input)
	{
		Input->AddBindKey("ToMainWorld", VK_SPACE);
		Input->SetBindFunction("ToMainWorld", EInputType::Press, this, &CStartPlayer::ToMainWorld);

		Input->AddBindKey("ToEditorWorld", VK_RETURN);
		Input->SetBindFunction("ToEditorWorld", EInputType::Press, this, &CStartPlayer::ToEditorWorld);
	}


	return true;
}

void CStartPlayer::ToMainWorld()
{
	CWorldManager::GetInst()->CreateWorld<CMainWorld>(true);
}

void CStartPlayer::ToEditorWorld()
{
	CWorldManager::GetInst()->CreateWorld<CEditorWorld>(true);
}
