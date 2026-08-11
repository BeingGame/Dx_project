#pragma once

#include "CBufferData.h"
#include "CBufferContainer.h"

//트랜스폼
//크기랑 회전 이동 모두를 가지고 있는 데이터

class CCBufferTransform :
    public CCBufferData
{
public:
	CCBufferTransform();
	CCBufferTransform(const CCBufferTransform& src);
	CCBufferTransform(CCBufferTransform&& src) noexcept;
	virtual ~CCBufferTransform();

protected:
	FCBufferTransform mData;

public:
	void SetWorldMatrix(const FMatrix& World)
	{
		mData.World = World;
	}

	void SetViewMatrix(const FMatrix& View)
	{
		mData.View = View;
	}

	void SetProjMatrix(const FMatrix& Proj)
	{
		mData.Proj = Proj;
	}

	void SetPivotSize(const FVector3& Pivot)
	{
		mData.PivotSize = Pivot;
	}

public:
	virtual bool Init();
	virtual void UpdateBuffer();
	virtual CCBufferTransform* Clone();

};

