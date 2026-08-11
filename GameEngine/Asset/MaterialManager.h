#pragma once

#include "AssetSubManager.h"
#include "Material.h"

//머티리얼 매니저의 역할은 머티리얼 공유하고, 사용시 머티리얼을 복사해서 사용하도록 한다.
//복사해서 각각이 고유의 속성을 가진 머티리얼을 사용하게 한다.

class CMaterialManager :
    public CAssetSubManager
{
public:
    CMaterialManager();
    virtual ~CMaterialManager();

private:
    std::unordered_map<std::string, std::shared_ptr<CMaterial>> mMaterialMap;

public:
    virtual bool Init();
    bool CreateMaterial(const std::string& Name, const std::string& PixelShader, const std::string& SamplerName);
    std::shared_ptr<CMaterial> CreateMaterialInstance(const std::string& Name);
    std::weak_ptr<CMaterial> FindMaterial(const std::string& Name);
};

