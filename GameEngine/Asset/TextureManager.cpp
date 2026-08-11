#include "TextureManager.h"

CTextureManager::CTextureManager()
{}

CTextureManager::~CTextureManager()
{}

bool CTextureManager::Init()
{
	return true;
}

bool CTextureManager::LoadTexture(const std::string& Name, const TCHAR* FileName, const std::string& PathName)
{
	//텍스처를 로드하기전 먼저 텍스처맵에 존재하는지 확인한다.
	auto Check = FindTexture(Name);

	if (!Check.expired())
	{
		return true;
	}

	//없으면 새로운 텍스처를 생성하고 맵에 넣어준다.
	std::string TextureName = "Texture_" + Name;

	std::shared_ptr<CTexture> Texture = std::make_shared<CTexture>();

	Texture->SetName(TextureName);

	if (!Texture->LoadTexture(FileName, PathName))
	{
		return false;
	}

	mTextureMap.insert(std::make_pair(TextureName, Texture));

	return true;
}

bool CTextureManager::LoadTexture(const std::string& Name, const TCHAR* FullPath)
{
	//텍스처를 로드하기전 먼저 텍스처맵에 존재하는지 확인한다.
	auto Check = FindTexture(Name);

	if (!Check.expired())
	{
		return true;
	}

	//없으면 새로운 텍스처를 생성하고 맵에 넣어준다.
	std::string TextureName = "Texture_" + Name;

	std::shared_ptr<CTexture> Texture = std::make_shared<CTexture>();

	Texture->SetName(TextureName);

	if (!Texture->LoadTexture(FullPath))
	{
		return false;
	}

	mTextureMap.insert(std::make_pair(TextureName, Texture));

	return true;
}

bool CTextureManager::LoadTexture(const std::string& Name, const std::vector<const TCHAR*>& FileName, const std::string& PathName)
{
	//텍스처를 로드하기전 먼저 텍스처맵에 존재하는지 확인한다.
	auto Check = FindTexture(Name);

	if (!Check.expired())
	{
		return true;
	}

	//없으면 새로운 텍스처를 생성하고 맵에 넣어준다.
	std::string TextureName = "Texture_" + Name;

	std::shared_ptr<CTexture> Texture = std::make_shared<CTexture>();

	Texture->SetName(TextureName);

	if (!Texture->LoadTexture(FileName, PathName))
	{
		return false;
	}

	mTextureMap.insert(std::make_pair(TextureName, Texture));

	return true;
}

bool CTextureManager::LoadTexture(const std::string& Name, const std::vector<const TCHAR*>& FullPath)
{
	//텍스처를 로드하기전 먼저 텍스처맵에 존재하는지 확인한다.
	auto Check = FindTexture(Name);

	if (!Check.expired())
	{
		return true;
	}

	//없으면 새로운 텍스처를 생성하고 맵에 넣어준다.
	std::string TextureName = "Texture_" + Name;

	std::shared_ptr<CTexture> Texture = std::make_shared<CTexture>();

	Texture->SetName(TextureName);

	if (!Texture->LoadTexture(FullPath))
	{
		return false;
	}

	mTextureMap.insert(std::make_pair(TextureName, Texture));

	return true;
}

std::weak_ptr<CTexture> CTextureManager::FindTexture(const std::string& Name)
{
	std::string TextureName = "Texture_" + Name;

	auto iter = mTextureMap.find(TextureName);

	if (iter == mTextureMap.end())
	{
		return std::weak_ptr<CTexture>();
	}

	return iter->second;
}
