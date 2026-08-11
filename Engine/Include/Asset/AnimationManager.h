#pragma once

#include "AssetSubManager.h"
#include "Animation2D.h"

class CAnimationManager :
	public CAssetSubManager
{
public:
	CAnimationManager();
	~CAnimationManager();

private:
	std::unordered_map<std::string, std::shared_ptr<CAnimation2D>> mAnimationMap;

public:
	virtual bool Init();

	bool CreateAnimation(const std::string& Name);
	std::weak_ptr<CAnimation2D> FindAnimation(const std::string& Name);
	bool SetAnimationTextureType(const std::string& Name, EAnimation2DTextureType Type);
	bool SetTexture(const std::string& Name, const std::weak_ptr<class CTexture> Texture);
	bool SetTexture(const std::string& Name, const std::string& TextureName);
	bool SetTexture(const std::string& Name, const std::string& TextureName, const TCHAR* FileName, const std::string& PathName = "Texture");
	bool SetTextureFullPath(const std::string& Name, const std::string& TextureName, const TCHAR* FullPath);
	bool SetTexture(const std::string& Name, const std::string& TextureName, std::vector<const TCHAR*> FileName, const std::string& PathName = "Texture");
	bool SetTextureFullPath(const std::string& Name, const std::string& TextureName, std::vector<const TCHAR*> FullPath);

	bool AddFrame(const std::string& Name, const FVector2& Start, const FVector2& Size);
	bool AddFrame(const std::string& Name, float StartX, float StartY, float SizeX, float SizeY);
	bool AddFrame(const std::string& Name, int Count, const FVector2& Start, const FVector2& Size);
	bool AddFrame(const std::string& Name, int Count, float StartX, float StartY, float SizeX, float SizeY);

};

