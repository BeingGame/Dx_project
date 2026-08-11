#include "BlackBoard.h"

CBlackBoard::CBlackBoard()
{}

CBlackBoard::~CBlackBoard()
{}

void CBlackBoard::SetInt(const std::string& Name, int Value)
{
	mIntMap[Name] = Value;
}

int CBlackBoard::GetInt(const std::string & Name)
{
	auto iter = mIntMap.find(Name);

	if (iter == mIntMap.end())
	{
		return 0;
	}

	return iter->second;
}

void CBlackBoard::SetFloat(const std::string& Name, float Value)
{
	mFloatMap[Name] = Value;
}

float CBlackBoard::GetFloat(const std::string & Name)
{
	auto iter = mFloatMap.find(Name);

	if (iter == mFloatMap.end())
	{
		return 0.f;
	}

	return iter->second;
}

void CBlackBoard::SetString(const std::string& Name, const std::string& Value)
{
	mStringMap[Name] = Value;
}

std::string CBlackBoard::GetString(const std::string & Name)
{
	auto iter = mStringMap.find(Name);

	if (iter == mStringMap.end())
	{
		return std::string();
	}

	return iter->second;
}

void CBlackBoard::SetVector2(const std::string& Name, const FVector2& Value)
{
	mVector2Map[Name] = Value;
}

FVector2 CBlackBoard::GetVector2(const std::string & Name)
{
	auto iter = mVector2Map.find(Name);

	if (iter == mVector2Map.end())
	{
		return	FVector2();
	}

	return iter->second;
}

void CBlackBoard::SetVector3(const std::string& Name, const FVector3& Value)
{
	mVector3Map[Name] = Value;
}

FVector3 CBlackBoard::GetVector3(const std::string & Name)
{
	auto iter = mVector3Map.find(Name);

	if (iter == mVector3Map.end())
	{
		return	FVector3::Zero;
	}

	return iter->second;
}

void CBlackBoard::SetCObject(const std::string& Name, std::weak_ptr<CObject> Value)
{
	mObjectMap[Name] = Value;
}
