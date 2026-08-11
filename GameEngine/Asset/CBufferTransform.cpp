#include "CBufferTransform.h"
#include "CBuffer.h"

CCBufferTransform::CCBufferTransform()
{}

CCBufferTransform::CCBufferTransform(const CCBufferTransform& src)
	:CCBufferData(src)
{
	mData = src.mData;
}

CCBufferTransform::CCBufferTransform(CCBufferTransform&& src) noexcept
	:CCBufferData(std::move(src))
{
	mData = src.mData;
}

CCBufferTransform::~CCBufferTransform()
{}

bool CCBufferTransform::Init()
{
	SetConstantBuffer("Transform");

	return mBuffer != nullptr;
}

void CCBufferTransform::UpdateBuffer()
{
	mData.WV = mData.World * mData.View;
	mData.WVP = mData.WV * mData.Proj;
	mData.VP = mData.View * mData.Proj;


	//DX의 행렬은 우리가 계산하는 행렬이랑 반대

	mData.World.Transpose();
	mData.View.Transpose();
	mData.Proj.Transpose();
	mData.WV.Transpose();
	mData.WVP.Transpose();
	mData.VP.Transpose();

	if (mBuffer)
	{
		mBuffer->Update(&mData);
	}

}

CCBufferTransform* CCBufferTransform::Clone()
{
	//자기자신을 역참조해서 복사
	return new CCBufferTransform(*this);
}
