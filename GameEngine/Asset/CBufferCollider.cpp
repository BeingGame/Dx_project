#include "CBufferCollider.h"
#include "CBuffer.h"

CCBufferCollider::CCBufferCollider()
{}

CCBufferCollider::CCBufferCollider(const CCBufferCollider& src)
	:CCBufferData(src)
{
	mData = src.mData;
}

CCBufferCollider::CCBufferCollider(CCBufferCollider&& src) noexcept
	:CCBufferData(std::move(src))
{
	mData = src.mData;
}

CCBufferCollider::~CCBufferCollider()
{}

bool CCBufferCollider::Init()
{
	SetConstantBuffer("Collider");

	return mBuffer != nullptr;
}

void CCBufferCollider::UpdateBuffer()
{
	if (mBuffer)
	{
		mBuffer->Update(&mData);
	}
}

CCBufferCollider* CCBufferCollider::Clone()
{
	//자기자신을 역참조해서 복사
	return new CCBufferCollider(*this);
}
