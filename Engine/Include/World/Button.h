#pragma once

#include "Widget.h"

//버튼
//마우스를 눌러서 버튼을 클릭하고, 뗄수있는 클래스
//버튼의 상태에 따라서 콜백함수를 처리해준다.

class CButton :
	public CWidget
{
public:
	CButton();
	CButton(const CButton& src);
	virtual ~CButton();

protected:
	//버튼은 어떤 위젯이든 자식 하나를 가질수 있게 한다.
	std::shared_ptr<CWidget> mChild;

	//버튼의 상태별로 UIBrush를 정의한다.
	FUIBrush mBrush[EWidgetState::End];

	std::weak_ptr<class CSound> mSound[EWidgetEventState::End];

public:
	void SetChild(const std::weak_ptr<CWidget>& Child);

	virtual void RemoveChild(std::shared_ptr<CWidget> Child);

	virtual void SetOpacityAll(float Opacity);

public:
	//버튼을 브러쉬마다 변경할수 있도록 set함수를 구성해준다.
	bool SetTexture(EWidgetState::Type State, const std::weak_ptr<class CTexture> Texture);
	bool SetTexture(EWidgetState::Type State, const std::string& Name);
	bool SetTexture(EWidgetState::Type State, const std::string& Name, const TCHAR* FileName, const std::string& PathName = "Texture");

	void SetTint(EWidgetState::Type State, const FVector4& Color);
	void SetTint(EWidgetState::Type State, float r, float g, float b, float a);

	//애니메이션
	void SetBrushAnimation(EWidgetState::Type State, bool AnimationEnable);
	void AddBrushFrame(EWidgetState::Type State, const FVector2& Start, const FVector2& Size);
	void AddBrushFrame(EWidgetState::Type State, float StartX, float StartY, float SizeX, float SizeY);
	void SetCurrentFrame(EWidgetState::Type State, int Frame);
	void SetAnimationPlayTime(EWidgetState::Type State, float PlayTime);
	void SetAnimationPlayRate(EWidgetState::Type State, float PlayRate);

	void SetSound(EWidgetEventState::Type State, const std::string& Name);
	void SetSound(EWidgetEventState::Type State, const std::string& Name, const char* FileName, const std::string& PathName = "Sound");

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void Render();
	virtual bool CollisionMouse(std::weak_ptr<CWidget>& Result, const FVector2& MousePos);

	virtual void MouseHovered();
	virtual void MouseUnHovered();
	virtual void MouseClicked();
	virtual void MouseRelease();

	virtual CButton* Clone();


};

