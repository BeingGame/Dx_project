#pragma once

#include "../EngineInfo.h"
#include "WidgetContainer.h"

//UI매니저
//UI의 생성, 렌더링 순서, UI의 충돌까지 모두 매니저에서 관리한다.

//UI는 위젯을 여러개 가지는 하나의 컨테이너 역할을 하는 것
//위젯은 하나의 부품이자 요소

//UI는 항상 최상단에 렌더링되야 되기 때문에
//RenderManager와 다른 렌더링 시스템을 가지며, 액터보다 후순위로 렌더링된다.

//렌더링 순서같은 경우는 UI는 ZOrder라는 속성을 통해 UI의 렌더링 순서를 정렬해준다.

//UI충돌
//마우스를 이용한 충돌이기 때문에, 마우스의 상태에 따라 위젯의 충돌 상태를 변경한다.
//3가지 상태 Hovered(마우스 올라감), Clicked(마우스 눌림), Release(마우스 떼짐)
//UI도 회전이 가능, 다만 사각형으로 만들기 때문에 AABB와 OBB 둘중 하나를 선택해서 사용한다.

class CUIManager : public std::enable_shared_from_this<CUIManager>
{
public:
	CUIManager();
	~CUIManager();

private:
	std::weak_ptr<class CWorld> mWorld;

	//스크린에 렌더링될 위젯 목록
	std::vector<std::shared_ptr<CWidgetContainer>> mWidgetList;

	//월드에 렌더링될 위젯 목록
	//월드 위젯 리스트를 CWidget으로 하는 이유
	//간단한 위젯을 위젯 컴포넌트에 붙여서 사용할수 있게 설계하기 때문
	std::vector<std::weak_ptr<CWidget>> mWorldWidgetList;

	std::weak_ptr<CWidget> mHoveredWidget;

public:
	void SetWorld(const std::weak_ptr<class CWorld> World)
	{
		mWorld = World;
	}


public:
	bool Init();
	void Update(float DeltaTime);
	bool CollisionMouse(float DeltaTime, const FVector2& MousePos);
	bool CollisionScreenUI(float DetltTime, const FVector2& MousePos);
	bool CollisionWorldUI(float DetltTime, const FVector2& MousePos);

	void Render();

	std::weak_ptr<CWidgetContainer> GetWidget(const std::string& Name);

public:
	std::shared_ptr<CUIManager> GetThisPtr()
	{
		return shared_from_this();
	}

public:
	template<typename T>
	std::weak_ptr<T> CreateWidgetContainer(const std::string& Name, int ZOrder = 0)
	{
		std::shared_ptr<CWidgetContainer> NewWidget = std::make_shared<T>();

		NewWidget->SetWorld(mWorld);
		NewWidget->SetName(Name);
		NewWidget->SetUIManager(GetThisPtr());
		NewWidget->SetZOrder(ZOrder);

		if (!NewWidget->Init())
		{
			return std::weak_ptr<T>();
		}

		mWidgetList.push_back(NewWidget);

		return std::dynamic_pointer_cast<T>(NewWidget);
	}

	template<typename T>
	std::weak_ptr<T> FindWidget(const std::string& Name)
	{
		auto iter = mWidgetList.begin();
		auto iterEnd = mWidgetList.end();

		for (; iter != iterEnd; ++iter)
		{
			if ((*iter)->GetName() == Name)
			{
				return std::dynamic_pointer_cast<T>(*iter);
			}
		}

		return std::weak_ptr<T>();
	}

public:
	void AddWorldWidget(const std::weak_ptr<CWidget>& Widget)
	{
		if (Widget.expired())
		{
			return;
		}

		mWorldWidgetList.push_back(Widget);
	}

protected:
	//렌더링같은 경우에는 후순위가 화면에 나타나기 때문에
	static bool SortRender(const std::shared_ptr<class CWidget>& Src, const std::shared_ptr<class CWidget>& Dest);
	static bool SortWorldRender(const std::weak_ptr<class CWidget>& Src, const std::weak_ptr<class CWidget>& Dest);

	//ZOrder가장 큰 위젯이 가장 먼저 충돌연산을 진행한다.
	static bool SortCollision(const std::shared_ptr<class CWidget>& Src, const std::shared_ptr<class CWidget>& Dest);
	static bool SortWorldCollision(const std::weak_ptr<class CWidget>& Src, const std::weak_ptr<class CWidget>& Dest);

};

