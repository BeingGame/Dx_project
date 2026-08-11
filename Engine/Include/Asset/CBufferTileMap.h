#pragma once

#include "CBufferData.h"
#include "CBufferContainer.h"

class CCBufferTileMap :
	public CCBufferData
{

public:
	CCBufferTileMap();
	CCBufferTileMap(const CCBufferTileMap& src);
	CCBufferTileMap(CCBufferTileMap&& src) noexcept;
	virtual ~CCBufferTileMap();

protected:
	FCBufferTileMap mData;

public:
	void SetUV(const FVector2& LTUV, const FVector2& RBUV)
	{
		SetLTUV(LTUV);
		SetRBUV(RBUV);
	}

	void SetUV(float LTx, float LTy, float RBx, float RBy)
	{
		SetLTUV(LTx, LTy);
		SetRBUV(RBx, RBy);
	}

	void SetLTUV(const FVector2& LTUV)
	{
		mData.LTUV = LTUV;
	}

	void SetLTUV(float x, float y)
	{
		mData.LTUV = FVector2(x, y);
	}

	void SetRBUV(const FVector2& RBUV)
	{
		mData.RBUV = RBUV;
	}

	void SetRBUV(float x, float y)
	{
		mData.RBUV = FVector2(x, y);
	}

public:
	virtual bool Init();
	virtual void UpdateBuffer();
	virtual CCBufferTileMap* Clone();
};

