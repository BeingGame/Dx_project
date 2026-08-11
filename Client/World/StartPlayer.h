#pragma once

#include "World/Actor.h"

class CStartPlayer :
    public CActor
{

public:
	CStartPlayer();
	CStartPlayer(const CStartPlayer& src);
	CStartPlayer(CStartPlayer&& src) noexcept;
	virtual ~CStartPlayer();

public:
    virtual bool Init();

public:
	void ToMainWorld();
	void ToEditorWorld();

};

