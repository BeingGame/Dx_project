#include "CBufferTileMap.h"
#include "CBuffer.h"

CCBufferTileMap::CCBufferTileMap()
{}

CCBufferTileMap::CCBufferTileMap(const CCBufferTileMap& src)
	:CCBufferData(src)
{
	mData = src.mData;
}

CCBufferTileMap::CCBufferTileMap(CCBufferTileMap&& src) noexcept
	:CCBufferData(std::move(src))
{
	mData = src.mData;
}

CCBufferTileMap::~CCBufferTileMap()
{}

bool CCBufferTileMap::Init()
{
	SetConstantBuffer("TileMap");

	return mBuffer != nullptr;
}

void CCBufferTileMap::UpdateBuffer()
{
	if (mBuffer)
	{
		mBuffer->Update(&mData);
	}

}

CCBufferTileMap* CCBufferTileMap::Clone()
{
	//자기자신을 역참조해서 복사
	return new CCBufferTileMap(*this);
}
