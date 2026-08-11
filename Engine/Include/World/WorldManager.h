#pragma once

#include "World.h"

class CWorldManager
{
	//월드매니저는 여러개일 필요가 없으므로 싱글톤
	Singleton(CWorldManager);

private:
	std::shared_ptr<CWorld> mWorld;
	std::shared_ptr<CWorld> mNextWorld;

public:
	std::weak_ptr<CWorld> GetWorld() const;

public:
	bool Init();

	bool Update(float DeltaTime);
	bool PostUpdate(float DeltaTime);
	void Render();
	void PostRender();
	void RenderUI();

private:
	bool ChangeWorld();


public:
	template<typename T>
	std::weak_ptr<T> CreateWorld(bool Next = true)
	{
		if (Next)
		{
			mNextWorld = std::make_shared<T>();

			if (!mNextWorld->Init())
			{
				return std::weak_ptr<T>();
			}

			return std::dynamic_pointer_cast<T>(mNextWorld);
		}
		//월드 생성전 직전 월드를 메모리에서 해제한다.
		mWorld.reset();

		mWorld = std::make_shared<T>();

		if (!mWorld->Init())
		{
			return std::weak_ptr<T>();
		}

		return std::dynamic_pointer_cast<T>(mWorld);

	}
		
};

