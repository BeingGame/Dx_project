#include "Tile.h"
#include <fstream>

CTile::CTile()
{}

CTile::CTile(const CTile& src)
{}

CTile::CTile(CTile&& src) noexcept
{}

CTile::~CTile()
{}

void CTile::Save(std::ofstream& File)
{
	File.write((char*)(&mType), sizeof(ETileType));
	File.write((char*)(&mPos), sizeof(FVector2));
	File.write((char*)(&mSize), sizeof(FVector2));
	File.write((char*)(&mCenter), sizeof(FVector2));

	File.write((char*)(&mTextureFrame), sizeof(int));
	File.write((char*)(&mIndexX), sizeof(int));
	File.write((char*)(&mIndexY), sizeof(int));
	File.write((char*)(&mIndex), sizeof(int));

	File.write((char*)(&mFrameStart), sizeof(FVector2));
	File.write((char*)(&mFrameEnd), sizeof(FVector2));

	File.write((char*)(&mRender), sizeof(bool));
	File.write((char*)(&mOutLineRender), sizeof(bool));
	File.write((char*)(&mOutLineColor), sizeof(FVector4));
}

void CTile::Load(std::ifstream& File)
{
	File.read((char*)(&mType), sizeof(ETileType));
	File.read((char*)(&mPos), sizeof(FVector2));
	File.read((char*)(&mSize), sizeof(FVector2));
	File.read((char*)(&mCenter), sizeof(FVector2));

	File.read((char*)(&mTextureFrame), sizeof(int));
	File.read((char*)(&mIndexX), sizeof(int));
	File.read((char*)(&mIndexY), sizeof(int));
	File.read((char*)(&mIndex), sizeof(int));

	File.read((char*)(&mFrameStart), sizeof(FVector2));
	File.read((char*)(&mFrameEnd), sizeof(FVector2));

	File.read((char*)(&mRender), sizeof(bool));
	File.read((char*)(&mOutLineRender), sizeof(bool));
	File.read((char*)(&mOutLineColor), sizeof(FVector4));
}
