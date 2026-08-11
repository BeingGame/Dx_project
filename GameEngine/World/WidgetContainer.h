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

