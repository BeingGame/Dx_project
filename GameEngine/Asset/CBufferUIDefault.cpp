#include "CBufferUIDefault.h"
#include "CBuffer.h"

CCBufferUIDefault::CCBufferUIDefault()
{}

CCBufferUIDefault::CCBufferUIDefault(const CCBufferUIDefault& src)
	:CCBufferData(src)
{
	mData = src.mData;
}

CCBufferUIDefault::CCBufferUIDefault(CCBufferUIDefault&& src) noexcept
	:CCBufferData(std::move(src))
{
	mData = src.mData;
}

CCBufferUIDefault::~CCBufferUIDefault()
{}

bool CCBufferUIDefault::Init()
{
	SetConstantBuffer("UIDefault");

	return mBuffer != nullptr;
}

void CCBufferUIDefault::UpdateBuffer()
{
	if (mBuffer)
	{
		mBuffer->Update(&mData);
	}

}

CCBufferUIDefault* CCBufferUIDefault::Clone()
{
	//자기자신을 역참조해서 복사
	return new CCBufferUIDefault(*this);
}
