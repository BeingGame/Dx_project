#pragma once

#include "CBufferData.h"
#include "CBufferContainer.h"

class CCBufferPalette :
	public CCBufferData
{
public:
	CCBufferPalette();
	CCBufferPalette(const CCBufferPalette& src);
	CCBufferPalette(CCBufferPalette&& src) noexcept;
	virtual ~CCBufferPalette();

protected:
	FCBufferPalette mData;

public:
	void SetColor(const std::vector<FVector4>& Color, size_t Size)
	{
		size_t Count = Size < 256 ? Size : 256;

		memcpy(mData.PaletteColor, &Color[0], sizeof(FVector4) * Size);
	}

	void SetUsePalette(bool Use)
	{
		mData.PaletteEnable = Use ? 1 : 0;
	}

public:
	virtual bool Init();
	virtual void UpdateBuffer();
	virtual CCBufferPalette* Clone();
};

