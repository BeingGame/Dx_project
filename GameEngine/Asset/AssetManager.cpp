#include "AssetManager.h"

#include "MeshManager.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "MaterialManager.h"
#include "AnimationManager.h"
#include "SoundManager.h"
#include "FontManager.h"

CAssetManager* CAssetManager::mInstance = nullptr;

CAssetManager::CAssetManager()
{

}

CAssetManager::~CAssetManager()
{

}

bool CAssetManager::Init()
{

	if (!AddSubManager<CShaderManager>(EAssetType::Shader))
	{
		return false;
	}

	if (!AddSubManager<CMaterialManager>(EAssetType::Material))
	{
		return false;
	}

	if (!AddSubManager<CMeshManager>(EAssetType::Mesh))
	{
		return false;
	}

	if (!AddSubManager<CTextureManager>(EAssetType::Texture))
	{
		return false;
	}

	if (!AddSubManager<CAnimationManager>(EAssetType::Animation2D))
	{
		return false;
	}

	if (!AddSubManager<CSoundManager>(EAssetType::Sound))
	{
		return false;
	}

	if (!AddSubManager<CFontManager>(EAssetType::Font))
	{
		return false;
	}

	return true;
}

void CAssetManager::Update()
{
	auto iter = mSubManagerMap.begin();
	auto iterEnd = mSubManagerMap.end();

	for (; iter != iterEnd; ++iter)
	{
		iter->second->Update();
	}
}
