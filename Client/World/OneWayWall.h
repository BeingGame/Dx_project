#pragma once

#include "World/Actor.h"

class COneWayWall :
    public CActor
{
public:
	COneWayWall();
	COneWayWall(const COneWayWall& src);
	COneWayWall(COneWayWall&& src) noexcept;
	virtual ~COneWayWall();
	virtual std::string GetTypeName() const override { return "COneWayWall"; }

protected:
	std::weak_ptr<class CMeshComponent> mMeshComp;
	std::weak_ptr<class CColliderBox2D> mBox;

public:
	virtual bool Init();

public:
	void OnBeginOverlap(const FVector3& HitPoint, const FVector3& Normal, std::weak_ptr<class CCollider> Collider);
};

