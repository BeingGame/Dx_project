#pragma once

#include "../EngineInfo.h"
#include "../Object.h"

//상태끼리 데이터를 공유하는 데이터 클래스
//각 데이터의 타입마다 규격이 다르다.

//void* -> 동적할당된 객체의 경우 직접 해제줘야한다.
//shared_ptr<void>로 만든다면?
//Actor나 컴포넌트같은 경우의 참조 카운트가 증가해서 IsAlive로 리스트에서 제거 하더라도
//블랙보드에는 남아있게 된다.

//따라서 최대한 단순하게 만들기 위해서 각 데이터 타입별로 map을 만들어준다.

class CBlackBoard
{
public:
	CBlackBoard();
	virtual ~CBlackBoard();

private:
	std::unordered_map<std::string, int> mIntMap;
	std::unordered_map<std::string, float> mFloatMap;
	std::unordered_map<std::string, std::string> mStringMap;
	std::unordered_map<std::string, FVector2> mVector2Map;
	std::unordered_map<std::string, FVector3> mVector3Map;
	std::unordered_map<std::string, std::weak_ptr<CObject>> mObjectMap;

public:
	void SetInt(const std::string& Name, int Value);
	int GetInt(const std::string& Name);

	void SetFloat(const std::string& Name, float Value);
	float GetFloat(const std::string& Name);

	void SetString(const std::string& Name, const std::string& Value);
	std::string GetString(const std::string& Name);

	void SetVector2(const std::string& Name, const FVector2& Value);
	FVector2 GetVector2(const std::string& Name);

	void SetVector3(const std::string& Name, const FVector3& Value);
	FVector3 GetVector3(const std::string& Name);

	void SetCObject(const std::string& Name, std::weak_ptr<CObject> Value);

	template<typename T>
	std::weak_ptr<T> GetCObject(const std::string& Name)
	{
		auto iter = mObjectMap.find(Name);

		if (iter == mObjectMap.end() || iter->second.expired())
		{
			if (iter->second.expired())
			{
				mObjectMap.erase(iter);
			}

			return std::weak_ptr<T>();
		}

		return std::dynamic_pointer_cast<T>(iter->second.lock());
	}

};

