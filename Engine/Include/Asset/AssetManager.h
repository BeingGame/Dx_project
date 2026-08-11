#pragma once

#include "../EngineInfo.h"

class CAssetManager
{
	Singleton(CAssetManager);

private:
	std::map<EAssetType, std::shared_ptr<class CAssetSubManager>> mSubManagerMap;

public:
	bool Init();
	void Update();

public:
	template<typename T>
	bool AddSubManager(EAssetType Type)
	{
		std::shared_ptr<T> NewManager = std::make_shared<T>();

		if (!NewManager->Init())
		{
			return false;
		}

		mSubManagerMap.insert(std::make_pair(Type, std::dynamic_pointer_cast<CAssetSubManager>(NewManager)));

		return true;
	}

	template<typename T>
	std::shared_ptr<T> GetSubManager(EAssetType Type)
	{
		auto iter = mSubManagerMap.find(Type);

		if (iter == mSubManagerMap.end())
		{
			return std::shared_ptr<T>();
		}

		return std::dynamic_pointer_cast<T>(iter->second);
	}

};

