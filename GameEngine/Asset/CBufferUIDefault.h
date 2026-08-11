#pragma once

#include "CBufferData.h"
#include "CBufferContainer.h"

class CCBufferUIDefault :
    public CCBufferData
{
public:
	CCBufferUIDefault();
	CCBufferUIDefault(const CCBufferUIDefault& src);
	CCBufferUIDefault(CCBufferUIDefault&& src) noexcept;
	virtual ~CCBufferUIDefault();

protected:
	FCBufferUIDefault mData;

public:
	void SetBrushTint(const FVector4& Tint)
	{
		mData.BrushTint = Tint;
	}

	void SetWidgetColor(const FVector4& Color)
	{
		mData.WidgetColor = Color;
	}

	void SetLTUV(const FVector2& UV)
	{
		mData.BrushLTUV = UV;
	}

	void SetRBUV(const FVector2& UV)
	{
		mData.BrushRBUV = UV;
	}
	
	void SetAnimationEnable(bool Enable)
	{
		mData.BrushAnimationEnable = Enable ? 1 : 0;
	}
	
	void SetTextureEnable(bool Enable)
	{
		mData.BrushTextureEnable = Enable ? 1 : 0;
	}
	
	void SetBrushPercentEnable(bool Enable)
	{
		mData.BrushPercentEnable = Enable ? 1 : 0;
	}

	void SetBrushPercent(float Percent)
	{
		mData.BrushPercent = Percent;
	}

	void SetBrushBarDir(int Dir)
	{
		mData.BrushBarDir = Dir;
	}

public:
	virtual bool Init();
	virtual void UpdateBuffer();
	virtual CCBufferUIDefault* Clone();
};

