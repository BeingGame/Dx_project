#include "WidgetContainer.h"

CWidgetContainer::CWidgetContainer()
{}

CWidgetContainer::CWidgetContainer(const CWidgetContainer& src)
	:CWidget(src)
{
	//자식 위젯들을 전부 복사해준다.
	for (auto& It : src.mChildList)
	{
		std::shared_ptr<CWidget> Child(It->Clone());
		mChildList.push_back(Child);
	}

	//컨테이너를 복사하고 난뒤 복사한 위치에서 SetParentAll을 호출해서 부모구조를 만들어줘야한다.
}

CWidgetContainer::~CWidgetContainer()
{}

void CWidgetContainer::SetParentAll()
{
	for (auto& It : mChildList)
	{
		It->SetParent(GetThisPtr<CWidget>());

		It->SetParentAll();
	}
}

void CWidgetContainer::SetOpacityAll(float Opacity)
{
	CWidget::SetOpacityAll(Opacity);

	for (auto& It : mChildList)
	{
		It->SetOpacityAll(Opacity);
	}
}

void CWidgetContainer::RemoveChild(std::shared_ptr<CWidget> Child)
{
	auto iter = mChildList.begin();
	auto iterEnd = mChildList.end();

	for (; iter != iterEnd; ++iter)
	{
		if (*iter == Child)
		{
			mChildList.erase(iter);
			break;
		}
	}
}

bool CWidgetContainer::Init()
{
	CWidget::Init();

	return true;
}

void CWidgetContainer::Update(float DeltaTime)
{
	CWidget::Update(DeltaTime);

	//자식을 업데이트해준다.
	auto iter = mChildList.begin();
	auto iterEnd = mChildList.end();

	for (; iter != iterEnd;)
	{
		if (!(*iter)->IsAlive())
		{
			iter = mChildList.erase(iter);
			iterEnd = mChildList.end();
			continue;
		}
		else if (!(*iter)->IsEnable())
		{
			++iter;
			continue;
		}

		(*iter)->Update(DeltaTime);

		++iter;
	}
}

void CWidgetContainer::Render()
{
	CWidget::Render();

	//자식 위젯이 2개이상일때 정렬을 시도한다.
	if (mChildList.size() >= 2)
	{
		std::sort(mChildList.begin(), mChildList.end(), CWidget::SortRender);
	}

	auto iter = mChildList.begin();
	auto iterEnd = mChildList.end();

	for (; iter != iterEnd;)
	{
		if (!(*iter)->IsAlive())
		{
			iter = mChildList.erase(iter);
			iterEnd = mChildList.end();
			continue;
		}
		else if (!(*iter)->IsEnable())
		{
			++iter;
			continue;
		}

		(*iter)->Render();

		++iter;
	}

}

bool CWidgetContainer::CollisionMouse(std::weak_ptr<CWidget>& Result, const FVector2& MousePos)
{
	//자식 위젯이 2개이상일때 정렬을 시도한다.
	if (mChildList.size() >= 2)
	{
		std::sort(mChildList.begin(), mChildList.end(), CWidget::SortCollision);
	}

	auto iter = mChildList.begin();
	auto iterEnd = mChildList.end();

	for (; iter != iterEnd;)
	{
		if (!(*iter)->IsAlive())
		{
			iter = mChildList.erase(iter);
			iterEnd = mChildList.end();
			continue;
		}
		else if (!(*iter)->IsEnable())
		{
			++iter;
			continue;
		}

		if ((*iter)->CollisionMouse(Result, MousePos))
		{
			return true;
		}

		++iter;
	}

	if (CWidget::CollisionMouse(Result, MousePos))
	{
		return true;
	}

	return false;
}

void CWidgetContainer::MouseHovered()
{}

void CWidgetContainer::MouseUnHovered()
{}

void CWidgetContainer::MouseClicked()
{}

void CWidgetContainer::MouseRelease()
{}

CWidgetContainer* CWidgetContainer::Clone()
{
	return new CWidgetContainer(*this);
}
