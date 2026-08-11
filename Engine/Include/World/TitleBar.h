#pragma once

#include "Widget.h"

//드래그옵션을 가진 위젯
//업데이트할 위젯을 들고있고, 마우스 클릭시, 업데이트할 위젯이 있으면 그 위젯을 움직여준다.
//없으면 움직이지 않는다.

class CTitleBar :
    public CWidget
{

public:
	CTitleBar();
	CTitleBar(const CTitleBar& src);
	virtual ~CTitleBar();

public:
	//창의 막대기 이미지를 적용할수 있게 들고있는 브러쉬 변수
	FUIBrush mBrush;
	//창의 막대기에 제목이나 이미지가 적용될수 있도록 하는 자식 위젯
	std::shared_ptr<class CWidget> mChild;

	//위젯으로 이동을 업데이트해줄 위젯
	std::weak_ptr<CWidget> mUpdateWidget;
		
public:
	void SetUpdateWidget(const std::weak_ptr<CWidget>& Widget);
	void SetChild(const std::weak_ptr<CWidget>& Child);
	
	virtual void RemoveChild(std::shared_ptr<CWidget> Child);
	virtual void SetParentAll();
	virtual void SetOpacityAll(float Opacity);

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

	virtual bool CollisionMouse(std::weak_ptr<CWidget>& Result, const FVector2& MousePos);
	virtual void MouseHovered();
	virtual void MouseUnHovered();
	virtual void MouseClicked();
	virtual void MouseRelease();

	virtual CTitleBar* Clone();


};

