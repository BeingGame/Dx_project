#include "StartWorld.h"

#include "StartPlayer.h"

#include "World/UIManager.h"
#include "StartWidget.h"

CStartWorld::CStartWorld()
{}

CStartWorld::~CStartWorld()
{}

bool CStartWorld::Init()
{
	CWorld::Init();

	auto Player = CreateActor<CStartPlayer>("Player").lock();

	auto UIMgr = GetUIManager().lock();

	if (UIMgr)
	{
		auto MainUI = UIMgr->CreateWidgetContainer<CStartWidget>("StartWidget", 0).lock();

		if (MainUI)
		{
			MainUI->SetPos(0.f, 0.f);
		}
	}

	return true;
}
