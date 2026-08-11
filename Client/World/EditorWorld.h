#pragma once

#include "World/World.h"

class CEditorWorld :
    public CWorld
{
public:
	CEditorWorld();
	virtual ~CEditorWorld();

public:
	virtual bool Init();
};

