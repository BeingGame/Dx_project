#pragma once

#include "../EngineInfo.h"
#include "Actor.h"
#include "../TimeManager.h"
#include <fstream>
#include <functional>

//월드
//하나의 레벨혹은 스테이지
//레벨단위로 사용되기 때문에 배치된 액터는 월드가 변경되면 사라진다.

class CWorld : public std::enable_shared_from_this<CWorld>
{
public:
	using ActorFactory = std::function<std::shared_ptr<CActor>()>;
	using CompFactory  = std::function<std::shared_ptr<CComponent>()>;

	static std::unordered_map<std::string, ActorFactory> sActorFactoryMap;
	static std::unordered_map<std::string, CompFactory>  sCompFactoryMap;

	template<typename T>
	static void RegisterActorType(const std::string& TypeName)
	{
		sActorFactoryMap[TypeName] = []() { return std::make_shared<T>(); };
	}

	template<typename T>
	static void RegisterComponentType(const std::string& TypeName)
	{
		sCompFactoryMap[TypeName] = []() { return std::make_shared<T>(); };
	}

public:
	CWorld();
	virtual ~CWorld();

protected:
	//월드에서는 실제적으로 액터가 생성되어서 배치된다.
	//따라서 shared_ptr로 액터를 감쌀 자료구조를 선언한다.

	//연결리스트를 기반으로 만드는게 유리하다.
	//멀티맵을 사용한 이유는 같은 이름을 간지 액터가 여러개 올수 있기 때문
	//삭제시에도 연결리스트와 유사하게 동작한다.
	//배열은 삽입 삭제가 빈번하게 일어나는 게임에선 유리하지않기 때문에 사용하지 않는다.
	std::unordered_multimap<std::string, std::shared_ptr<CActor>> mActorList;

	std::shared_ptr<class CCameraManager> mCameraManager;
	std::shared_ptr<class CInput> mInput;
	std::shared_ptr<class CWorldCollision> mCollision;
	std::shared_ptr<class CUIManager> mUIManager;

	//월드에 등록된 타이머를 제거하기위한 변수
	std::vector<FTimerHandle> mTimerHandleList;

public:
	std::weak_ptr<class CCameraManager> GetCameraManager() const
	{
		return mCameraManager;
	}

	std::weak_ptr<class CInput> GetInput() const
	{
		return mInput;
	}

	std::weak_ptr<class CWorldCollision> GetWorldCollision() const
	{
		return mCollision;
	}

	std::weak_ptr<class CUIManager> GetUIManager() const
	{
		return mUIManager;
	}

	const std::unordered_multimap<std::string, std::shared_ptr<CActor>>& GetActorList() const
	{
		return mActorList;
	}

public:
	void InputActive();
	void InputDeActive();

	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);
	virtual void Render();
	virtual void PostRender();
	virtual void RenderUI();

	void SaveScene(const std::string& FilePath) const;
	bool LoadScene(const std::string& FilePath);

public:
	std::shared_ptr<CWorld> GetThisPtr()
	{
		return std::dynamic_pointer_cast<CWorld>(shared_from_this());
	}

public:
	template<typename T>
	std::weak_ptr<T> CreateActor(const std::string& Name)
	{
		std::shared_ptr<T> Actor = std::make_shared<T>();

		Actor->SetName(Name);
		Actor->SetWorld(GetThisPtr());

		if (!Actor->Init())
		{
			return std::weak_ptr<T>();
		}

		mActorList.insert(std::make_pair(Name, Actor));

		return std::dynamic_pointer_cast<T>(Actor);
	}

	//액터 검색 기능
	template<typename T>
	std::weak_ptr<T> FindActor(const std::string& Name)
	{
		auto iter = mActorList.find(Name);

		if (iter == mActorList.end())
		{
			return std::weak_ptr<T>();
		}

		return std::dynamic_pointer_cast<T>(iter->second);
	}

	//액터 이름을 바꾸고 맵의 키도 같이 옮긴다.
	//mActorList는 이름을 키로 쓰는 multimap이라, 액터 이름만 바꾸면 키가 옛 이름으로
	//남아 FindActor/저장이 어긋난다. 포인터가 일치하는 기존 항목을 지우고 새 키로 넣는다.
	//성공하면 true. (이름이 비었거나 같으면 false)
	bool RenameActor(const std::shared_ptr<CActor>& Actor, const std::string& NewName)
	{
		if (!Actor || NewName.empty())
		{
			return false;
		}

		if (Actor->GetName() == NewName)
		{
			return false;
		}

		//옛 이름 버킷에서 포인터가 일치하는 항목을 찾아 제거한다.
		auto Range = mActorList.equal_range(Actor->GetName());

		for (auto iter = Range.first; iter != Range.second; ++iter)
		{
			if (iter->second == Actor)
			{
				mActorList.erase(iter);
				break;
			}
		}

		Actor->SetName(NewName);
		mActorList.insert(std::make_pair(NewName, Actor));

		return true;
	}

	//액터 검색을 했을때 여러개를 받을수 있는 함수
	template<typename T>
	bool FindObjectList(const std::string& Name, std::list<std::weak_ptr<T>>& ActorList)
	{
		//범위를 받아오는 함수는 equal_range
		//정상적으로 범위를 받으면 시작이랑 끝 반복자가 다르다.
		//동일하면 범위를 찾지 못한것이다.
		std::pair<std::unordered_multimap<std::string, std::shared_ptr<CActor>>::iterator,
			std::unordered_multimap<std::string, std::shared_ptr<CActor>>::iterator> Range = mActorList.equal_range(Name);

		if (Range.first == Range.second)
		{
			return false;
		}

		std::unordered_multimap<std::string, std::shared_ptr<CActor>>::iterator iter = Range.first;

		for (; iter != Range.second; ++iter)
		{
			ActorList.push_back(std::dynamic_pointer_cast<T>(iter->second));
		}

		return true;

	}


	//클래스별로 해당하는 액터 리스트를 가져온다.
	//상속을 이용한다.
	template<typename T>
	bool FindObjectList(std::list<std::weak_ptr<T>>& ActorList)
	{

		auto iter = mActorList.begin();
		auto iterEnd = mActorList.end();

		for (; iter != iterEnd; ++iter)
		{
			auto Actor = std::dynamic_pointer_cast<T>(iter->second);

			if (Actor)
			{
				ActorList.push_back(Actor);
			}
		}

		return true;

	}


public:
	template<typename T>
	FTimerHandle SetTimer(float Time, bool Loop, T* Object, void (T::* Func)())
	{
		FTimerHandle Handle = CTimeManager::SetTimer(Time, Loop, Object, Func);
		mTimerHandleList.push_back(Handle);
		return Handle;
	}

	void ClearTimer(FTimerHandle& Handle)
	{
		CTimeManager::ClearTimer(Handle);
	}

};

