#include "CBufferAnimation2D.h"

#include "CBuffer.h"

CCBufferAnimation2D::CCBufferAnimation2D()
{}

CCBufferAnimation2D::CCBufferAnimation2D(const CCBufferAnimation2D& src)
	:CCBufferData(src)
{
	mData = src.mData;
}

CCBufferAnimation2D::CCBufferAnimation2D(CCBufferAnimation2D&& src) noexcept
	:CCBufferData(std::move(src))
{
	mData = src.mData;
}

CCBufferAnimation2D::~CCBufferAnimation2D()
{}

bool CCBufferAnimation2D::Init()
{
	SetConstantBuffer("Animation2D");

	return mBuffer != nullptr;
}

void CCBufferAnimation2D::UpdateBuffer()
{
	if (mBuffer)
	{
		mBuffer->Update(&mData);
	}

}

CCBufferAnimation2D* CCBufferAnimation2D::Clone()
{
	//자기자신을 역참조해서 복사
	return new CCBufferAnimation2D(*this);
}
