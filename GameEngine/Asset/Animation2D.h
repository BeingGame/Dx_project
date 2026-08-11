#pragma once

#include "Asset.h"

//2D환경에서 애니메이션은 프레임과 스프라이트시트 두가지 텍스처를 이용해서
//시간에맞추서 텍스처를 흐리게 만드는 방식
//ex)공책 구석에 그림 그려놓고 당기면 애니메이션처럼 만들어지는것

//프레임은 텍스처 여러장을 사용
//스프라이트 시트는 텍스처 한장에 모든 애니메이션을 몰아넣어서 사용

//드로우콜
//머티리얼을 교체할때마다 발생한다.
//드로우콜이 발생할때마다 셰이더가 변경되고 상수버퍼 교체
//발생하면 비용문제로 인해 성능이 내려갈수 있다.

//스프라이트 시트는 드로우콜을 줄이기 위해 UV데이터를 받아서 애니메이션을 출력한다.

class CAnimation2D :
	public CAsset
{
public:
	CAnimation2D();
	virtual ~CAnimation2D();

protected:
	std::weak_ptr<class CTexture> mTexture;

	EAnimation2DTextureType mTextureType;

	std::vector<FTextureFrame> mFrameArray;

public:
	EAnimation2DTextureType GetType() const
	{
		return mTextureType;
	}

	const std::weak_ptr<class CTexture>& GetTexture() const
	{
		return mTexture;
	}

	const FTextureFrame& GetFrame(int Index)
	{
		if (Index < 0 || Index >= mFrameArray.size())
		{
			static FTextureFrame dummy;
			return dummy;
		}

		return mFrameArray[Index];
	}

	//애니메이션이 가진 프레임의 갯수를 반환한다.
	int GetFrameCount()
	{
		return (int)mFrameArray.size();
	}

	void SetAnimationTextureType(EAnimation2DTextureType Type)
	{
		mTextureType = Type;
	}

public:
	//애니메이션에 텍스처를 등록하는 함수
	void SetTexture(const std::weak_ptr<class CTexture>& Texture);
	void SetTexture(const std::string& Name);
	void SetTexture(const std::string& Name, const TCHAR* FileName, const std::string& PathName = "Texture");
	void SetTextureFullPath(const std::string& Name, const TCHAR* FullPath);
	void SetTexture(const std::string& Name, std::vector<const TCHAR*> FileName, const std::string& PathName = "Texture");
	void SetTextureFullPath(const std::string& Name, std::vector<const TCHAR*> FullPath);


	//애니메이션에 프레임을 등록하는 함수
	void AddFrame(const FVector2& Start, const FVector2& Size, const FVector2& Offset = FVector2(0.f,0.f));
	void AddFrame(float StartX, float StartY, float SizeX, float SizeY, float OffsetX = 0.f, float OffsetY = 0.f);
	//한번에 여러개의 프레임을 등록하는 함수
	void AddFrame(int Count,const FVector2& Start, const FVector2& Size, const FVector2& Offset = FVector2(0.f, 0.f));
	void AddFrame(int Count,float StartX, float StartY, float SizeX, float SizeY, float OffsetX = 0.f, float OffsetY = 0.f);

	//애니메이션의 프레임을 전부 제거하는 함수
	void ClearFrame();

	//가장 크기가 큰 텍스처를 기준으로 모든 프레임의 Ratio를 계산한다.
	void CalculateFrameRatio();

	//애니메이션을 통해 팔레트컬러를 만들어준다.
	void MakePaletteColor(const FVector2& OriginPos, const FVector2& TargetPos, const FVector2& FrameSize);

};

