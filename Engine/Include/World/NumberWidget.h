#pragma once

#include "Widget.h"

//스프라이트 시트를 이용해서
//숫자를 정수와 소수부분을 렌더링할수있는 Widget 클래스

class CNumberWidget :
    public CWidget
{
public:
	CNumberWidget();
	CNumberWidget(const CNumberWidget& src);
	virtual ~CNumberWidget();

protected:
	FUIBrush mBrush;
	//정수랑 소수 둘다 출력하기위해서
	float mNumber = 0.f;

	//출력할 정수의 자릿수
	int mCount = 1;
	//출력할 소수점 자릿수
	int mDecimalCount = 2;

	std::vector<int> mNumberArray;
	std::vector<int> mDecimalNumberArray;

	FVector2 mNumberSize;
	FVector3 mNumberRenderPos;

public:
	void SetCount(int Count)
	{
		mCount = Count;
	}

	void SetDecimalCount(int Count)
	{
		mDecimalCount = Count;
	}

	void SetNumber(float Number)
	{
		mNumber = Number;
	}

	void AddNumber(float Number)
	{
		mNumber += Number;
	}
	
	void SetNumberSize(const FVector2& Size)
	{
		mNumberSize = Size;
	}

	void SetNumberSize(float x, float y)
	{
		mNumberSize.x = x;
		mNumberSize.y = y;
	}

public:
	bool SetTexture(const std::weak_ptr<class CTexture> Texture);
	bool SetTexture(const std::string& Name);
	bool SetTexture(const std::string& Name, const TCHAR* FileName, const std::string& PathName = "Texture");
	bool SetTexture(const std::string& Name, std::vector<const TCHAR*> FileName, const std::string& PathName = "Texture");

	void SetTint(const FVector4& Color);
	void SetTint(float r, float g, float b, float a);

	//애니메이션
	void SetBrushAnimation(bool AnimationEnable);
	void AddBrushFrame(const FVector2& Start, const FVector2& Size);
	void AddBrushFrame(float StartX, float StartY, float SizeX, float SizeY);
	void AddBrushFrame(int Count, const FVector2& Start, const FVector2& Size);
	void AddBrushFrame(int Count, float StartX, float StartY, float SizeX, float SizeY);
	void SetCurrentFrame(int Frame);
	void SetAnimationPlayTime(float PlayTime);
	void SetAnimationPlayRate(float PlayRate);
	void SetAnimationType(EAnimation2DTextureType Type);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void Render();

	//UI충돌
	virtual bool CollisionMouse(std::weak_ptr<CWidget>& Result, const FVector2& MousePos);
	virtual CNumberWidget* Clone();


public:
	void RenderNumber();

};

