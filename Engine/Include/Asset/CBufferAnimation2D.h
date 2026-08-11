#pragma once

#include "CBufferData.h"
#include "CBufferContainer.h"

class CCBufferAnimation2D :
	public CCBufferData
{
public:
	CCBufferAnimation2D();
	CCBufferAnimation2D(const CCBufferAnimation2D& src);
	CCBufferAnimation2D(CCBufferAnimation2D&& src) noexcept;
	virtual ~CCBufferAnimation2D();

protected:
	FCBufferAnimation2D mData;

public:
	/*
	FVector2 LTUV;
	//우측 아래 UV는 특별한일이 없으면 무조건 1.f,1.f
	FVector2 RBUV = FVector2(1.f, 1.f);
	int Animation2DEnable = 0;
	int Animation2DTExtureType = -1;
	int TextureSymmetry = 0;
	int Animation2DFrame = 0;
	*/

	void SetLTUV(const FVector2& UV)
	{
		mData.LTUV = UV;
	}

	void SetLTUV(float U, float V)
	{
		mData.LTUV.x = U;
		mData.LTUV.y = V;
	}

	void SetRBUV(const FVector2& UV)
	{
		mData.RBUV = UV;
	}

	void SetRBUV(float U, float V)
	{
		mData.RBUV.x = U;
		mData.RBUV.y = V;
	}

	void SetAnimation2DEnable(bool Enable)
	{
		mData.Animation2DEnable = Enable ? 1 : 0;
	}

	void SetAnimation2DTextureType(EAnimation2DTextureType Type)
	{
		mData.Animation2DTextureType = (int)Type;
	}

	void SetTextureSymmetry(bool Symmetry)
	{
		mData.TextureSymmetry = Symmetry ? 1 : 0;
	}

	void SetAnimFrame(int Frame)
	{
		mData.Animation2DFrame = Frame;
	}

	void SetAnimRatio(const FVector2& Ratio)
	{
		mData.Ratio = Ratio;
	}

	void SetAnimRatio(float X, float Y)
	{
		mData.Ratio.x = X;
		mData.Ratio.y = Y;
	}

	void SetAnimOffset(const FVector2& Offset)
	{
		mData.Offset = Offset;
	}

	void SetAnimOffset(float X, float Y)
	{
		mData.Offset.x = X;
		mData.Offset.y = Y;
	}


public:
	virtual bool Init();
	virtual void UpdateBuffer();
	virtual CCBufferAnimation2D* Clone();
};

