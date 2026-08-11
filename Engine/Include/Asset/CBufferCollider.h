#pragma once

#include "CBufferData.h"
#include "CBufferContainer.h"

class CCBufferCollider :
    public CCBufferData
{
public:
	CCBufferCollider();
	CCBufferCollider(const CCBufferCollider& src);
	CCBufferCollider(CCBufferCollider&& src) noexcept;
	virtual ~CCBufferCollider();

protected:
	FCBufferCollider mData;

public:
	void SetColor(const FVector4& Color)
	{
		mData.Color = Color;
	}

public:
	virtual bool Init();
	virtual void UpdateBuffer();
	virtual CCBufferCollider* Clone();
};

