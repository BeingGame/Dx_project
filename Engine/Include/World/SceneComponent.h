#pragma once

#include "Component.h"

class CSceneComponent :
	public CComponent
{
public:
	CSceneComponent();
	CSceneComponent(const CSceneComponent& src);
	CSceneComponent(CSceneComponent&& src) noexcept;
	virtual ~CSceneComponent();

protected:
	//계층구조를 위한 자료구조를 정의해준다.
	std::weak_ptr<CSceneComponent> mParent;
	std::vector<std::weak_ptr<CSceneComponent>> mChildList;

	int mRenderLayerOrder = 0;

public:
	std::weak_ptr<CSceneComponent> GetParent() const
	{
		return mParent;
	}

	int GetLayerOrder() const
	{
		return mRenderLayerOrder;
	}

	void SetRenderLayer(int LayerOrder);
	void SetRenderLayer(const std::string& Name);

public:
	void AddChild(std::weak_ptr<CSceneComponent> Child);
	void UpdateTransform();

	void AttachToActor(const std::weak_ptr<CActor> Actor);
	void DetachFromParent();

protected:
	void NormalizeRot(FVector3& Rotation);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Render();
	virtual void PostRender();
	virtual void Destroy();

public:
	virtual std::string GetTypeName() const override { return "CSceneComponent"; }
	virtual void Save(std::ofstream& File) const override;
	virtual void Load(const std::unordered_map<std::string, std::string>& Props) override;

public:
	virtual CSceneComponent* Clone() const;

protected:
	//부모의 크기나 회전을 경우에 따라 받지 않게끔 처리하는 불변수
	bool mInheritScale = true;
	bool mInheritRot = true;

	FVector3 mVelocity;

	//씬컴포넌트의 트랜스폼
	//월드 트랜스폼
	FVector3 mWorldScale = FVector3::One;
	FVector3 mWorldRot;
	FVector3 mWorldPos;

	//상대 트랜스폼
	//부모가 없을땐 월드랑 상대 트랜스폼은 동일하다.
	FVector3 mRelativeScale = FVector3::One;
	FVector3 mRelativeRot;
	FVector3 mRelativePos;

	//축 벡터
	//각 축의 방향벡터, 미리 만들어놓으면 좋다.
	FVector3 mWorldAxis[EAxis::End]
	{
		FVector3::Axis[EAxis::X],
		FVector3::Axis[EAxis::Y],
		FVector3::Axis[EAxis::Z]
	};


	FMatrix mScaleMat;
	FMatrix mRotMat;
	FMatrix mTranslateMat;
	FMatrix mWorldMat;

public:
	const FVector3& GetVelocity() const
	{
		return mVelocity;
	}

	const FVector3& GetWorldScale() const
	{
		return mWorldScale;
	}

	const FVector3& GetWorldRot() const
	{
		return mWorldRot;
	}

	const FVector3& GetWorldPos() const
	{
		return mWorldPos;
	}

	const FVector3& GetRelativeScale() const
	{
		return mRelativeScale;
	}

	const FVector3& GetRelativeRot() const
	{
		return mRelativeRot;
	}

	const FVector3& GetRelativePos() const
	{
		return mRelativePos;
	}

	const FVector3& GetAxis(EAxis::Type Axis) const
	{
		return mWorldAxis[Axis];
	}

	void SetInheritScale(bool Scale);
	void SetInheritRot(bool Rot);

	void SetRelativeScale(const FVector3& Scale);
	void SetRelativeScale(const FVector2& Scale);
	void SetRelativeScale(float x, float y, float z);
	void SetRelativeScale(float x, float y);

	void AddRelativeScale(const FVector3& Scale);
	void AddRelativeScale(const FVector2& Scale);
	void AddRelativeScale(float x, float y, float z);
	void AddRelativeScale(float x, float y);

	void SetRelativeRotation(const FVector3& Rot);
	void SetRelativeRotation(const FVector2& Rot);
	void SetRelativeRotation(float x, float y, float z);
	void SetRelativeRotation(float x, float y);
	void SetRelativeRotationX(float x);
	void SetRelativeRotationY(float y);
	void SetRelativeRotationZ(float z);
	void AddRelativeRotation(const FVector3& Rot);
	void AddRelativeRotation(const FVector2& Rot);
	void AddRelativeRotation(float x, float y, float z);
	void AddRelativeRotation(float x, float y);
	void AddRelativeRotationX(float x);
	void AddRelativeRotationY(float y);
	void AddRelativeRotationZ(float z);

	void SetRelativePos(const FVector3& Pos);
	void SetRelativePos(const FVector2& Pos);
	void SetRelativePos(float x, float y, float z);
	void SetRelativePos(float x, float y);

	void AddRelativePos(const FVector3& Pos);
	void AddRelativePos(const FVector2& Pos);
	void AddRelativePos(float x, float y, float z);
	void AddRelativePos(float x, float y);

	void InheritScale();
	void InheritRotation();
	void InheritPos();

	//월드 트랜스폼
	void SetWorldScale(const FVector3& Scale);
	void SetWorldScale(const FVector2& Scale);
	void SetWorldScale(float x, float y, float z);
	void SetWorldScale(float x, float y);
			
	void AddWorldScale(const FVector3& Scale);
	void AddWorldScale(const FVector2& Scale);
	void AddWorldScale(float x, float y, float z);
	void AddWorldScale(float x, float y);
			
	void SetWorldRotation(const FVector3& Rot);
	void SetWorldRotation(const FVector2& Rot);
	void SetWorldRotation(float x, float y, float z);
	void SetWorldRotation(float x, float y);
	void SetWorldRotationX(float x);
	void SetWorldRotationY(float y);
	void SetWorldRotationZ(float z);
			
	void AddWorldRotation(const FVector3& Rot);
	void AddWorldRotation(const FVector2& Rot);
	void AddWorldRotation(float x, float y, float z);
	void AddWorldRotation(float x, float y);
	void AddWorldRotationX(float x);
	void AddWorldRotationY(float y);
	void AddWorldRotationZ(float z);
			
	void SetWorldPos(const FVector3& Pos);
	void SetWorldPos(const FVector2& Pos);
	void SetWorldPos(float x, float y, float z);
	void SetWorldPos(float x, float y);
			
	void AddWorldPos(const FVector3& Pos);
	void AddWorldPos(const FVector2& Pos);
	void AddWorldPos(float x, float y, float z);
	void AddWorldPos(float x, float y);

	void InheritWorldScale();
	void InheritWorldRotation();
	void InheritWorldPos();

	void UpdateChildWorldPos(const FVector3& Dist);

};

