#pragma once
#include "Collider.h"
class CColliderSphere2D :
    public CCollider
{

public:
	CColliderSphere2D();
	CColliderSphere2D(const CColliderSphere2D& src);
	CColliderSphere2D(CColliderSphere2D&& src) noexcept;
	virtual ~CColliderSphere2D();

protected:
	FSphere2DInfo mInfo;

public:
	const FSphere2DInfo& GetInfo() const
	{
		return mInfo;
	}

	void SetRadius(float Radius)
	{
		mInfo.Radius = Radius;
	}

public:
	virtual void SetDebugDraw(bool DebugDraw);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Destroy();

	virtual void UpdateInfo();

public:
	virtual CColliderSphere2D* Clone() const;

public:
	virtual bool Collision(FVector3& HitPoint, FVector3& Normal, float& Depth, std::shared_ptr<CCollider>& Dest);
	virtual bool CollisionMouse(const FVector2& MousePos);

public:
	virtual std::string GetTypeName() const override { return "CColliderSphere2D"; }
	virtual void Save(std::ofstream& File) const override;
	virtual void Load(const std::unordered_map<std::string, std::string>& Props) override;
};

