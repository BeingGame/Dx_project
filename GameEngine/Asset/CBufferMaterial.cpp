#include "CBufferMaterial.h"
#include "CBuffer.h"

CCBufferMaterial::CCBufferMaterial()
{}

CCBufferMaterial::CCBufferMaterial(const CCBufferMaterial& src)
	:CCBufferData(src)
{
	mData = src.mData;
}

CCBufferMaterial::CCBufferMaterial(CCBufferMaterial&& src) noexcept
	:CCBufferData(std::move(src))
{
	mData = src.mData;
}

CCBufferMaterial::~CCBufferMaterial()
{}

bool CCBufferMaterial::Init()
{
	SetConstantBuffer("Material");

	return mBuffer != nullptr;
}

void CCBufferMaterial::UpdateBuffer()
{
	if (mBuffer)
	{
		mBuffer->Update(&mData);
	}

}

CCBufferMaterial* CCBufferMaterial::Clone()
{
	//자기자신을 역참조해서 복사
	return new CCBufferMaterial(*this);
}
