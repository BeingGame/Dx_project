#include "MaterialManager.h"

CMaterialManager::CMaterialManager()
{}

CMaterialManager::~CMaterialManager()
{}

bool CMaterialManager::Init()
{
	CreateMaterial("Default", "Material", "Linear");

	return true;
}

bool CMaterialManager::CreateMaterial(const std::string& Name, const std::string& PixelShader, const std::string& SamplerName)
{
	if (!FindMaterial(Name).expired())
	{
		return false;
	}

	std::string MatName = "Material_" + Name;

	std::shared_ptr<CMaterial> Material = std::make_shared<CMaterial>();

	Material->SetName(MatName);
	Material->Init(PixelShader, SamplerName);

	mMaterialMap.insert(std::make_pair(MatName, Material));

	return true;
}

std::shared_ptr<CMaterial> CMaterialManager::CreateMaterialInstance(const std::string& Name)
{
	//현재 존재하는 머티리얼의 복사본을 반환한다.

	auto Material = FindMaterial(Name);

	if (Material.expired())
	{
		return std::shared_ptr<CMaterial>();
	}

	auto _Material = Material.lock();

	return std::shared_ptr<CMaterial>(_Material->Clone());
}

std::weak_ptr<CMaterial> CMaterialManager::FindMaterial(const std::string& Name)
{
	std::string MatName = "Material_" + Name;

	auto iter = mMaterialMap.find(MatName);

	if (iter == mMaterialMap.end())
	{
		return std::weak_ptr<CMaterial>();
	}

	return iter->second;
}

