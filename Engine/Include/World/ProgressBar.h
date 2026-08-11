#pragma once

#include "Widget.h"

//프로그레스 바(진행도 막대기)
//바형식으로된 이미지와 백그라운드 이미지를 이용해서 진행도를 나태내도록 설정한다.
//진행방향을 2가지로 만들어준다.
//왼쪽에서 오른쪽, 위에서 아래

namespace EProgressBarImageType
{
	enum Type
	{
		Fill,
		Back,
		End
	};
}

enum class EProgressBarDir
{
	RightToLeft,
	TopToBottom
};

class CProgressBar :
	public CWidget
{
public:
	CProgressBar();
	CProgressBar(const CProgressBar& src);
	virtual ~CProgressBar();

protected:
	//브러쉬 타입 두가지를 정의한다.
	FUIBrush mBrush[EProgressBarImageType::End];
	EProgressBarDir mBarDir = EProgressBarDir::RightToLeft;

	//정규화된 데이터
	float mPercent = 1.f;
	float mMinValue = 0.f;
	float mMaxValue = 1.f;

	std::shared_ptr<class CWidget> mChild;

public:
	float GetPercent() const
	{
		return mPercent;
	}

	void SetPercent(float Percent);
	void SetMinValue(float Min);
	void SetMaxValue(float Max);
	void SetDir(EProgressBarDir Dir);
	void SetValue(float Value);

	void SetChild(const std::weak_ptr<CWidget>& Child);
	virtual void RemoveChild(std::shared_ptr<CWidget> Child);
	virtual void SetParentAll();
	virtual void SetOpacityAll(float Opacity);

public:
	bool SetTexture(EProgressBarImageType::Type Type,const std::weak_ptr<class CTexture> Texture);
	bool SetTexture(EProgressBarImageType::Type Type, const std::string& Name);
	bool SetTexture(EProgressBarImageType::Type Type, const std::string& Name, const TCHAR* FileName, const std::string& PathName = "Texture");
	bool SetTexture(EProgressBarImageType::Type Type, const std::string& Name, std::vector<const TCHAR*> FileName, const std::string& PathName = "Texture");
	void SetTint(EProgressBarImageType::Type Type, const FVector4& Color);
	void SetTint(EProgressBarImageType::Type Type, float r, float g, float b, float a);
	void SetBrushAnimation(EProgressBarImageType::Type Type, bool AnimationEnable);
	void AddBrushFrame(EProgressBarImageType::Type Type, const FVector2& Start, const FVector2& Size);
	void AddBrushFrame(EProgressBarImageType::Type Type, float StartX, float StartY, float SizeX, float SizeY);
	void AddBrushFrame(EProgressBarImageType::Type Type, int Count, const FVector2& Start, const FVector2& Size);
	void AddBrushFrame(EProgressBarImageType::Type Type, int Count, float StartX, float StartY, float SizeX, float SizeY);
	void SetCurrentFrame(EProgressBarImageType::Type Type, int Frame);
	void SetAnimationPlayTime(EProgressBarImageType::Type Type, float PlayTime);
	void SetAnimationPlayRate(EProgressBarImageType::Type Type, float PlayRate);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void Render();

	//UI충돌
	virtual bool CollisionMouse(std::weak_ptr<CWidget>& Result, const FVector2& MousePos);
	virtual CProgressBar* Clone();

};

