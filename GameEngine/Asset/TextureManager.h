#pragma once

#include "AssetSubManager.h"
#include "Texture.h"

class CTextureManager :
	public CAssetSubManager
{
public:
	CTextureManager();
	virtual ~CTextureManager();

private:
	std::unordered_map<std::string, std::shared_ptr<CTexture>> mTextureMap;

public:
	bool Init();

	//관리자가 텍스처 로드함수를 불러올수 있게 만든다.
	bool LoadTexture(const std::string& Name, const TCHAR* FileName, const std::string& PathName);
	bool LoadTexture(const std::string& Name, const TCHAR* FullPath);
	bool LoadTexture(const std::string& Name, const std::vector<const TCHAR*>& FileName, const std::string& PathName);
	bool LoadTexture(const std::string& Name, const std::vector<const TCHAR*>& FullPath);

public:
	std::weak_ptr<CTexture> FindTexture(const std::string& Name);


};

