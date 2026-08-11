#pragma once


#include "../Object.h"

//애셋
//리소스를 포함해서 ID와같은 특정 속성을 가진 객체를 위한 추상클래스
//애니메이션, 텍스처, 사운드등과 같은 리소스 데이터들을 ID를 가지게 설계해
//관리하기 쉽도록 한다.

class CAsset
	:public CObject
{
public:
	CAsset();
	CAsset(const CAsset& src);	
	CAsset(CAsset&& src) noexcept;
	virtual ~CAsset() = 0;

protected:
	//Asset의 필수 속성들
	std::string mName = "";
	size_t mID = 0;
	EAssetType mType = EAssetType::None;

public:
	const std::string& GetName() const
	{
		return mName;
	}

	size_t GetID() const
	{
		return mID;
	}

	EAssetType GetType() const
	{
		return mType;
	}

	void SetName(const std::string& Name)
	{
		mName = Name;
	}



};

