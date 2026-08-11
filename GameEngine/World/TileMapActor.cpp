#include "TileMapActor.h"

#include "TileMapComponent.h"
#include "TileMapRender.h"

CTileMapActor::CTileMapActor()
{}

CTileMapActor::CTileMapActor(const CTileMapActor & src)
{}

CTileMapActor::CTileMapActor(CTileMapActor && src) noexcept
{}

CTileMapActor::~CTileMapActor()
{}

bool CTileMapActor::Init()
{
	mRender = CreateComponent<CTileMapRender>("TileMapRender");
	mTileMapComp = CreateComponent<CTileMapComponent>("TileMapComp");

	if (mRender.expired() || mTileMapComp.expired())
	{
		return false;
	}

	auto Render = mRender.lock();

	if (Render)
	{
		Render->SetTileMapComponent(mTileMapComp);
	}

	auto Comp = mTileMapComp.lock();

	if (Comp)
	{
		Comp->SetTileMapRender(mRender);
	}


	return true;
}

void CTileMapActor::Update(float DeltaTime)
{
	CActor::Update(DeltaTime);
}

void CTileMapActor::PostUpdate(float DeltaTime)
{
	CActor::PostUpdate(DeltaTime);
}

void CTileMapActor::Render()
{
	CActor::Render();
}

void CTileMapActor::PostRender()
{
	CActor::PostRender();
}

void CTileMapActor::Destroy()
{
	CActor::Destroy();
}
