#pragma once

#include "World/World.h"

class CStartWorld :
    public CWorld
{
public:
	CStartWorld();
	virtual ~CStartWorld();

public:
	virtual bool Init();
};

