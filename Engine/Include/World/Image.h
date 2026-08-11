#pragma once

#include "Widget.h"

class CImage :
    public CWidget
{
public:
	CImage();
	CImage(const CImage& src);
	virtual ~CImage();

public:
	FUIBrush mBrush;

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

	virtual CImage* Clone();

};

