#pragma once
#include "Collider.h"
class CColliderBox2D :
    public CCollider
{
public:
	CColliderBox2D();
	CColliderBox2D(const CColliderBox2D& src);
	CColliderBox2D(CColliderBox2D&& src) noexcept;
	virtual ~CColliderBox2D();

protected:
	FBox2DInfo mInfo;

public:
	const FBox2DInfo& GetInfo() const
	{
		return mInfo;
	}

	void SetBoxSize(const FVector2& Size)
	{
		mInfo.Halfsize = Size / 2.f;
	}

	void SetBoxSize(float x, float y)
	{
		mInfo.Halfsize = FVector2(x / 2.f, y / 2.f);
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
	virtual CColliderBox2D* Clone() const;

public:
    virtual bool Collision(FVector3& HitPoint, FVector3& Normal, float& Depth, std::shared_ptr<CCollider>& Dest);
	virtual bool CollisionMouse(const FVector2& MousePos);

public:
	virtual std::string GetTypeName() const override { return "CColliderBox2D"; }
	virtual void Save(std::ofstream& File) const override;
	virtual void Load(const std::unordered_map<std::string, std::string>& Props) override;
};

