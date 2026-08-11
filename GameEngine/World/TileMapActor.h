#pragma once

#include "Actor.h"

class CTileMapActor :
    public CActor
{
public:
	CTileMapActor();
	CTileMapActor(const CTileMapActor& src);
	CTileMapActor(CTileMapActor&& src) noexcept;
	virtual ~CTileMapActor();

protected:
	std::weak_ptr<class CTileMapRender> mRender;
	std::weak_ptr<class CTileMapComponent> mTileMapComp;

public:
	std::weak_ptr<class CTileMapRender> GetRender() const
	{
		return mRender;
	}

	std::weak_ptr<class CTileMapComponent> GetTileMapComp() const
	{
		return mTileMapComp;
	}

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Render();
	virtual void PostRender();
	virtual void Destroy();

};

