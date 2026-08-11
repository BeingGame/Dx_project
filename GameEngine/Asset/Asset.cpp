#include "Asset.h"

CAsset::CAsset()
{}

CAsset::CAsset(const CAsset & src)
	:CObject(src)
{
	mName = src.mName;
	mID = src.mID;
	mType = src.mType;
}

CAsset::CAsset(CAsset&& src) noexcept
	:CObject(std::move(src))
{
	mName = src.mName;
	mID = src.mID;
	mType = src.mType;
}

CAsset::~CAsset()
{}
