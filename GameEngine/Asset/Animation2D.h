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

	//초기화를 빼먹으면 쓰레기값이 들어가고, 셰이더가
	//cbAnimation2DTextureType != 0(SpriteSheet)으로 보고 UV 자르기를 건너뛴다.
	//그러면 프레임이 아니라 스프라이트 시트 전체가 그려진다.
	EAnimation2DTextureType mTextureType = EAnimation2DTextureType::SpriteSheet;

	std::vector<FTextureFrame> mFrameArray;

	//시퀀스에서 가장 큰 프레임의 크기(텍셀). CalculateFrameRatio에서 갱신된다.
	//정점의 1.0(쿼드 전체 폭)이 이 크기에 대응하므로,
	//픽셀 단위 값을 정규화 좌표로 바꿀 때 기준이 된다.
	FVector2 mMaxFrameSize = FVector2(0.f, 0.f);

	//프레임이 바뀌었으니 Ratio와 mMaxFrameSize를 다시 계산해야 하는지.
	//예전에는 CalculateFrameRatio를 부르는 쪽이 알아서 챙겨야 했는데,
	//애님 에디터와 .anim2d 로드 경로가 둘 다 빼먹고 있었다. 그래서
	//mMaxFrameSize가 0으로 남아 프레임 Offset이 화면에 전혀 반영되지 않았다.
	bool mFrameRatioDirty = false;

	//---- .anim2d에 저장된 재생 설정 ----
	//시퀀스(CAnimation2DSequence)가 들고 다니는 값이지만, 그 원본은 애니메이션 에셋이다.
	//여기 보관해두지 않으면 .anim2d에서 읽은 PlayRate/Loop/Reverse/Symmetry가
	//파일을 파싱하는 순간 버려지고, 월드에 적혀 있던 예전 값이 대신 쓰인다.
	//(애님 에디터에서 고쳐 저장해도 다시 켜면 옛날 값으로 돌아가 보이는 원인이었다)
	bool  mHasPlaySettings = false;
	float mPlayRate        = 1.f;
	bool  mLoop            = false;
	bool  mReverse         = false;
	bool  mSymmetry        = false;

public:
	EAnimation2DTextureType GetType() const
	{
		return mTextureType;
	}

	const FVector2& GetMaxFrameSize()
	{
		EnsureFrameRatio();
		return mMaxFrameSize;
	}

	const std::weak_ptr<class CTexture>& GetTexture() const
	{
		return mTexture;
	}

	const FTextureFrame& GetFrame(int Index)
	{
		EnsureFrameRatio();

		if (mFrameArray.empty())
		{
			static FTextureFrame dummy;
			return dummy;
		}

		//범위를 벗어나면 예전에는 크기 0짜리 더미를 돌려줬다.
		//그러면 UV 폭이 0이 되어 스프라이트가 통째로 투명해진다.
		//가장 가까운 실제 프레임으로 잘라준다.
		if (Index < 0)
		{
			Index = 0;
		}
		else if (Index >= (int)mFrameArray.size())
		{
			Index = (int)mFrameArray.size() - 1;
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

		//타입에 따라 최대 크기를 재는 방법이 달라진다.
		mFrameRatioDirty = true;
	}

	//프레임이 바뀐 뒤 처음 조회될 때 한 번만 다시 계산한다.
	//프레임을 추가할 때마다 계산하면 로드 중에 헛일을 반복하게 된다.
	void EnsureFrameRatio()
	{
		if (!mFrameRatioDirty)
		{
			return;
		}

		//먼저 꺼둔다. CalculateFrameRatio 안에서 다시 들어와도 되돌지 않도록.
		mFrameRatioDirty = false;

		CalculateFrameRatio();
	}

	//---- .anim2d에 저장된 재생 설정 ----
	//에셋에서 읽은 값이 있는지. 코드로 만든 애니메이션은 없다.
	bool  HasPlaySettings() const { return mHasPlaySettings; }
	float GetPlayRate() const     { return mPlayRate; }
	bool  GetLoop() const         { return mLoop; }
	bool  GetReverse() const      { return mReverse; }
	bool  GetSymmetry() const     { return mSymmetry; }

	void SetPlaySettings(float PlayRate, bool Loop, bool Reverse, bool Symmetry)
	{
		mHasPlaySettings = true;
		mPlayRate        = PlayRate;
		mLoop            = Loop;
		mReverse         = Reverse;
		mSymmetry        = Symmetry;
	}

public:
	//애니메이션에 텍스처를 등록하는 함수
	void SetTexture(const std::weak_ptr<class CTexture>& Texture);
	void SetTexture(const std::string& Name);
	void SetTexture(const std::string& Name, const TCHAR* FileName, const std::string& PathName = "Texture");
	void SetTextureFullPath(const std::string& Name, const TCHAR* FullPath);
	void SetTexture(const std::string& Name, std::vector<const TCHAR*> FileName, const std::string& PathName = "Texture");
	void SetTextureFullPath(const std::string& Name, std::vector<const TCHAR*> FullPath);


	//---- 프레임별 재생 시간 ----
	//시퀀스의 총 재생 시간 = 모든 프레임 Duration의 합
	float GetTotalDuration() const
	{
		float Total = 0.f;

		for (const auto& Frame : mFrameArray)
			Total += Frame.Duration;

		return Total;
	}

	void SetFrameDuration(int Index, float Duration)
	{
		if (Index < 0 || Index >= (int)mFrameArray.size())
			return;

		//0이면 프레임이 절대 안 넘어가므로 하한을 둔다.
		mFrameArray[Index].Duration = (Duration < 0.001f) ? 0.001f : Duration;
	}

	//총 재생 시간이 TotalTime이 되도록 모든 프레임을 같은 비율로 늘리고 줄인다.
	//프레임 사이의 완급(비율)은 그대로 유지된다.
	//프레임별 시간이 없던 시절의 SetPlayTime 호출부들이 이 경로로 들어온다.
	void ScaleTotalDuration(float TotalTime);

	//애니메이션에 프레임을 등록하는 함수
	void AddFrame(const FVector2& Start, const FVector2& Size, const FVector2& Offset = FVector2(0.f,0.f),
		float Duration = 0.1f);
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

