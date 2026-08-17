#pragma once

#include "Widget.h"

//위젯 클래스를 상속받아 만드는 컨테이너 클래스
//자식 위젯을 필요한만큼 받을수 있게 설계한다.
//하나의 도화지 역할을 해서 자식위젯을 도화지에 그려질수 있게 만들어준다.
//실질적인 역할은 위젯의 보관함용도

//트리구조
//월드 -> UIManager -> WidgetContainer.List -> Widget.List
//ZOrder를 트리구조로 구성하려 하면, 매번 ZOrder가 변경될때 모든 하위 위젯이 변경되어야한다.
//따라서 성능 낭비를 피하기 위해서 ZOrder 정렬이 아닌 트리 구조로 렌더링을 실시한다.(어디까지나 부모, 자식관계에서)

class CWidgetContainer :
    public CWidget
{
public:
	CWidgetContainer();
	CWidgetContainer(const CWidgetContainer& src);
	virtual ~CWidgetContainer();

protected:
	std::vector<std::shared_ptr<CWidget>> mChildList;

	// ── 세로 스크롤 ──────────────────────────────────────────────────────────
	//내용이 컨테이너보다 길어졌을 때 마우스 휠로 밀어볼 수 있게 해준다.
	//SetScrollTarget(true)로 표시된 자식만 함께 움직이고,
	//스크롤 영역([mScrollTop, mScrollBottom]) 밖으로 벗어난 자식은
	//렌더링과 충돌에서 제외된다.
	//(가위(Scissor) 클리핑이 없어서 걸친 위젯을 잘라낼 수 없으므로
	// 영역을 벗어난 위젯은 통째로 감춘다)
	bool  mScrollEnable   = false;
	float mScrollY        = 0.f;   //현재 스크롤 양 (0 = 맨 위)
	float mScrollTop      = 0.f;   //스크롤 영역 시작 (컨테이너 로컬 y)
	float mScrollBottom   = 0.f;   //스크롤 영역 끝 (컨테이너 로컬 y)
	float mScrollContentEnd = 0.f; //스크롤 대상 콘텐츠가 끝나는 로컬 y
	float mScrollStep     = 32.f;  //휠 한 칸당 이동량
	float mScrollBarW     = 6.f;   //스크롤바 두께

public:
	void EnableScroll(bool Enable)
	{
		mScrollEnable = Enable;

		if (!Enable)
		{
			mScrollY = 0.f;
		}
	}

	bool IsScrollEnable() const
	{
		return mScrollEnable;
	}

	//스크롤이 적용될 세로 구간을 컨테이너 로컬 좌표로 지정한다.
	//(보통 타이틀바 아래 ~ 패널 하단)
	void SetScrollArea(float Top, float Bottom)
	{
		mScrollTop = Top;
		mScrollBottom = Bottom;
	}

	//스크롤 대상 콘텐츠가 끝나는 로컬 y좌표.
	//레이아웃을 만들면서 쌓아온 Y 커서 값을 그대로 넘기면 된다.
	void SetScrollContentEnd(float LocalY)
	{
		mScrollContentEnd = LocalY;
	}

	void SetScrollStep(float Step)
	{
		mScrollStep = Step;
	}

	float GetScrollY() const
	{
		return mScrollY;
	}

	//더 이상 내려갈 수 없는 최대 스크롤 양
	float GetScrollMax() const
	{
		float Over = mScrollContentEnd - mScrollBottom;

		return Over > 0.f ? Over : 0.f;
	}

	void SetScrollY(float Y);

	void AddScrollY(float Delta)
	{
		SetScrollY(mScrollY + Delta);
	}

	//해당 자식이 스크롤 영역 밖으로 벗어났는지
	bool IsScrolledOut(const std::shared_ptr<CWidget>& Child) const;

protected:
	//휠 입력을 읽어 스크롤을 갱신하고 자식들에게 이동량을 넣어준다.
	void UpdateScroll();

	//스크롤바(트랙 + 썸)를 그린다. 스크롤이 필요 없으면 아무것도 그리지 않는다.
	void RenderScrollBar();

public:
	void AddWidget(const std::shared_ptr<CWidget>& Widget)
	{
		Widget->SetParent(GetThisPtr<CWidget>());
		Widget->SetUIManager(mUIManager);

		mChildList.push_back(Widget);
	}

	virtual void SetParentAll();
	virtual void SetOpacityAll(float Opacity);

	virtual void RemoveChild(std::shared_ptr<CWidget> Child);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void Render();

	//UI충돌
	virtual bool CollisionMouse(std::weak_ptr<CWidget>& Result, const FVector2& MousePos);
	virtual void MouseHovered();
	virtual void MouseUnHovered();
	virtual void MouseClicked();
	virtual void MouseRelease();
	virtual CWidgetContainer* Clone();

public:
	//위젯컨테이너에서 부품으로 사용될 위젯을 생성하는 함수
	template<typename T>
	std::weak_ptr<T> CreateWidget(const std::string& Name, int ZOrder = 0)
	{
		std::shared_ptr<T> NewWidget = std::make_shared<T>();

		NewWidget->SetWorld(mWorld);
		NewWidget->SetName(Name);
		NewWidget->SetUIManager(mUIManager);
		NewWidget->SetZOrder(ZOrder);
		NewWidget->SetParent(GetThisPtr<CWidget>());

		if (!NewWidget->Init())
		{
			return std::weak_ptr<T>();
		}

		mChildList.push_back(NewWidget);

		return NewWidget;
	}

	template<typename T>
	std::weak_ptr<T> FindWidget(const std::string& Name)
	{
		for (auto& It : mChildList)
		{
			if (It->GetName() == Name)
			{
				return std::dynamic_pointer_cast<T>(It);
			}
		}

		return std::weak_ptr<T>();
	}
};

