#pragma once

#include "World/Actor.h"

class CWall :
    public CActor
{
public:
	CWall();
	CWall(const CWall& src);
	CWall(CWall&& src) noexcept;
	virtual ~CWall();

protected:
	std::weak_ptr<class CMeshComponent> mMeshComp;
	std::weak_ptr<class CColliderBox2D> mBox;

public:
	virtual bool Init();
};

