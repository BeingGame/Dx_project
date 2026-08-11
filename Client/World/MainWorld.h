#pragma once

#include "World/World.h"

class CMainWorld :
	public CWorld
{
public:
	CMainWorld();
	virtual ~CMainWorld();

protected:
	std::vector<std::weak_ptr<class CCameraComponent>> mCameraList;
	int mCameraIndex = 0;

public:
	virtual bool Init();

public:
	void ChangeCamera();
	void AddAnimation();
};
