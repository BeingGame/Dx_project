#include "AnimationManager.h"

CAnimationManager::CAnimationManager()
{}

CAnimationManager::~CAnimationManager()
{}

bool CAnimationManager::Init()
{
	return true;
}

bool CAnimationManager::CreateAnimation(const std::string& Name)
{
	std::shared_ptr<CAnimation2D> Animation = FindAnimation(Name).lock();

	if (Animation)
	{
		//애니메이션을 초기화해서 새로운 애니메이션 동작으로 취급한다.
		Animation->ClearFrame();
		return true;
	}

	std::string AnimName = "Animation2D_" + Name;

	Animation = std::make_shared<CAnimation2D>();

	Animation->SetName(Name);

	mAnimationMap.insert(std::make_pair(AnimName, Animation));

	return true;
}

std::weak_ptr<CAnimation2D> CAnimationManager::FindAnimation(const std::string& Name)
{
	std::string AnimName = "Animation2D_" + Name;

	auto iter = mAnimationMap.find(AnimName);

	if (iter == mAnimationMap.end())
	{
		return std::weak_ptr<CAnimation2D>();
	}

	return iter->second;
}

bool CAnimationManager::SetAnimationTextureType(const std::string& Name, EAnimation2DTextureType Type)
{
	std::string AnimName = "Animation2D_" + Name;

	auto iter = mAnimationMap.find(AnimName);

	if (iter == mAnimationMap.end())
	{
		return false;
	}

	iter->second->SetAnimationTextureType(Type);

	return true;
}

bool CAnimationManager::SetTexture(const std::string& Name, const std::weak_ptr<class CTexture> Texture)
{
	std::string AnimName = "Animation2D_" + Name;

	auto iter = mAnimationMap.find(AnimName);

	if (iter == mAnimationMap.end())
	{
		return false;
	}

	iter->second->SetTexture(Texture);

	return true;
}

bool CAnimationManager::SetTexture(const std::string& Name, const std::string& TextureName)
{
	std::string AnimName = "Animation2D_" + Name;

	auto iter = mAnimationMap.find(AnimName);

	if (iter == mAnimationMap.end())
	{
		return false;
	}

	iter->second->SetTexture(TextureName);

	return true;
}

bool CAnimationManager::SetTexture(const std::string& Name, const std::string& TextureName, const TCHAR* FileName, const std::string& PathName)
{
	std::string AnimName = "Animation2D_" + Name;

	auto iter = mAnimationMap.find(AnimName);

	if (iter == mAnimationMap.end())
	{
		return false;
	}

	iter->second->SetTexture(TextureName, FileName, PathName);

	return true;
}

bool CAnimationManager::SetTextureFullPath(const std::string& Name, const std::string& TextureName, const TCHAR* FullPath)
{
	std::string AnimName = "Animation2D_" + Name;

	auto iter = mAnimationMap.find(AnimName);

	if (iter == mAnimationMap.end())
	{
		return false;
	}

	iter->second->SetTextureFullPath(TextureName, FullPath);

	return true;
}

bool CAnimationManager::SetTexture(const std::string& Name, const std::string& TextureName, std::vector<const TCHAR*> FileName, const std::string& PathName)
{
	std::string AnimName = "Animation2D_" + Name;

	auto iter = mAnimationMap.find(AnimName);

	if (iter == mAnimationMap.end())
	{
		return false;
	}

	iter->second->SetTexture(TextureName, FileName, PathName);

	return true;
}

bool CAnimationManager::SetTextureFullPath(const std::string& Name, const std::string& TextureName, std::vector<const TCHAR*> FullPath)
{
	std::string AnimName = "Animation2D_" + Name;

	auto iter = mAnimationMap.find(AnimName);

	if (iter == mAnimationMap.end())
	{
		return false;
	}

	iter->second->SetTextureFullPath(TextureName, FullPath);

	return true;
}

bool CAnimationManager::AddFrame(const std::string& Name, const FVector2& Start, const FVector2& Size)
{
	std::string AnimName = "Animation2D_" + Name;

	auto iter = mAnimationMap.find(AnimName);

	if (iter == mAnimationMap.end())
	{
		return false;
	}

	iter->second->AddFrame(Start, Size);

	return true;
}

bool CAnimationManager::AddFrame(const std::string& Name, float StartX, float StartY, float SizeX, float SizeY)
{
	std::string AnimName = "Animation2D_" + Name;

	auto iter = mAnimationMap.find(AnimName);

	if (iter == mAnimationMap.end())
	{
		return false;
	}

	iter->second->AddFrame(StartX, StartY, SizeX, SizeY);

	return true;
}

bool CAnimationManager::AddFrame(const std::string& Name, int Count, const FVector2& Start, const FVector2& Size)
{
	std::string AnimName = "Animation2D_" + Name;

	auto iter = mAnimationMap.find(AnimName);

	if (iter == mAnimationMap.end())
	{
		return false;
	}

	iter->second->AddFrame(Count, Start, Size);

	return true;
}

bool CAnimationManager::AddFrame(const std::string& Name, int Count, float StartX, float StartY, float SizeX, float SizeY)
{
	std::string AnimName = "Animation2D_" + Name;

	auto iter = mAnimationMap.find(AnimName);

	if (iter == mAnimationMap.end())
	{
		return false;
	}

	iter->second->AddFrame(Count, StartX, StartY, SizeX, SizeY);

	return true;
}
