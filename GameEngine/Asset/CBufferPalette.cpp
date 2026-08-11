#include "CBufferPalette.h"
#include "CBuffer.h"

CCBufferPalette::CCBufferPalette()
{}

CCBufferPalette::CCBufferPalette(const CCBufferPalette& src)
	:CCBufferData(src)
{
	mData = src.mData;
}

CCBufferPalette::CCBufferPalette(CCBufferPalette&& src) noexcept
	:CCBufferData(std::move(src))
{
	mData = src.mData;
}

CCBufferPalette::~CCBufferPalette()
{}

bool CCBufferPalette::Init()
{
	SetConstantBuffer("Palette");

	return mBuffer != nullptr;
}

void CCBufferPalette::UpdateBuffer()
{
	if (mBuffer)
	{
		mBuffer->Update(&mData);
	}

}

CCBufferPalette* CCBufferPalette::Clone()
{
	//자기자신을 역참조해서 복사
	return new CCBufferPalette(*this);
}
