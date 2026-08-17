#include "UIManager.h"

#include "../Device.h"
#include "World.h"
#include "Input.h"

CUIManager::CUIManager()
{}

CUIManager::~CUIManager()
{}

bool CUIManager::Init()
{
	const FResolution RS = CDevice::GetInst()->GetResolution();

	CWidget::CreateUIProjection(RS.Width, RS.Height);

	return true;
}

void CUIManager::Update(float DeltaTime)
{
	auto iter = mWidgetList.begin();
	auto iterEnd = mWidgetList.end();

	for (; iter != iterEnd;)
	{
		if (!(*iter)->IsAlive())
		{
			iter = mWidgetList.erase(iter);
			iterEnd = mWidgetList.end();
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

bool CUIManager::CollisionMouse(float DeltaTime, const FVector2& MousePos)
{
	if (!CollisionScreenUI(DeltaTime, MousePos))
	{
		return CollisionWorldUI(DeltaTime, MousePos);
	}

	return true;
}

bool CUIManager::CollisionScreenUI(float DetltTime, const FVector2& MousePos)
{
	if (mWidgetList.size() >= 2)
	{
		std::sort(mWidgetList.begin(), mWidgetList.end(), CUIManager::SortCollision);
	}

	//충돌된 위젯을 확인한다.
	if (mHoveredWidget.expired())
	{
		mHoveredWidget = std::weak_ptr<CWidget>();
	}

	auto iter = mWidgetList.begin();
	auto iterEnd = mWidgetList.end();

	for (; iter != iterEnd;)
	{
		if (!(*iter)->IsAlive())
		{
			iter = mWidgetList.erase(iter);
			iterEnd = mWidgetList.end();
			continue;
		}
		else if (!(*iter)->IsEnable())
		{
			++iter;
			continue;
		}

		std::weak_ptr<CWidget> HoveredWidget;

		if ((*iter)->CollisionMouse(HoveredWidget, MousePos))
		{
			//현재 충돌된 위젯과, 이전 위젯을 체크한다.
			auto Current = HoveredWidget.lock();
			auto Prev = mHoveredWidget.lock();

			//현재와 이전 위젯이 다를경우
			if (Prev != Current)
			{
				if (Prev && Prev->GetWidgetState() != EWidgetState::Disable)
				{
					//현재 위젯이 존재하고, 위젯이 disable이 아니면 본래 상태로 돌려준다.(Hovered가 아니게 처리)
					Prev->MouseUnHovered();
				}

				//현재 충돌된 위젯이 nullptr이 되도 상관은 없다.
				mHoveredWidget = HoveredWidget;


				if (Current && Current->GetWidgetState() != EWidgetState::Disable)
				{
					Current->MouseHovered();
				}
			}

			//위젯에 마우스가 올라와 있는 상태에서 마우스 버튼이 눌렸을 경우
			if (Current && Current->IsEnable()
				&& Current->GetWidgetState() != EWidgetState::Disable)
			{
				if (Current->IsMouseOn())
				{
					auto World = mWorld.lock();

					if (World)
					{
						auto Input = World->GetInput().lock();

						if (Input)
						{
							//위젯이 호버된 상태에서 마우스가 눌렸다면
							if (Current->GetWidgetState() == EWidgetState::Hovered && Input->GetMouseState(EMouseType::LButton, EInputType::Press))
							{
								Current->MouseClicked();
							}
							//반대로 위젯이 클릭된 상태에서 release호출되면
							else if (Current->GetWidgetState() == EWidgetState::Clicked && Input->GetMouseState(EMouseType::LButton, EInputType::Release))
							{
								Current->MouseRelease();
							}
							else if (Current->GetWidgetState() == EWidgetState::Release)
							{
								Current->SetWidgetState(EWidgetState::Hovered);
							}
						}

					}
				}
			}

			return true;
		}

		++iter;
	}

	//여기까지 코드가 진행됬으면
	//월드상에 충돌된 UI가 없다는 뜻
	if (!mHoveredWidget.expired())
	{
		auto Widget = mHoveredWidget.lock();

		if (Widget->GetWidgetState() != EWidgetState::Disable)
		{
			Widget->MouseUnHovered();
		}

		mHoveredWidget = std::weak_ptr<CWidget>();
	}

	return false;
}

bool CUIManager::CollisionWorldUI(float DetltTime, const FVector2& MousePos)
{
	//월드UI 충돌을 진행하기전 위젯이 유효한지 체크해준다.
	auto iter = mWorldWidgetList.begin();
	auto iterEnd = mWorldWidgetList.end();

	for (; iter != iterEnd;)
	{
		if (iter->expired())
		{
			iter = mWorldWidgetList.erase(iter);
			iterEnd = mWorldWidgetList.end();
			continue;
		}

		++iter;
	}


	if (mWorldWidgetList.size() >= 2)
	{
		std::sort(mWorldWidgetList.begin(), mWorldWidgetList.end(), CUIManager::SortWorldCollision);
	}

	//충돌된 위젯을 확인한다.
	if (mHoveredWidget.expired())
	{
		mHoveredWidget = std::weak_ptr<CWidget>();
	}

	iter = mWorldWidgetList.begin();
	iterEnd = mWorldWidgetList.end();

	for (; iter != iterEnd;)
	{
		auto Widget = iter->lock();

		if (!Widget->IsAlive())
		{
			iter = mWorldWidgetList.erase(iter);
			iterEnd = mWorldWidgetList.end();
			continue;
		}
		else if (!Widget->IsEnable())
		{
			++iter;
			continue;
		}

		std::weak_ptr<CWidget> HoveredWidget;

		if (Widget->CollisionMouse(HoveredWidget, MousePos))
		{
			//현재 충돌된 위젯과, 이전 위젯을 체크한다.
			auto Current = HoveredWidget.lock();
			auto Prev = mHoveredWidget.lock();

			//현재와 이전 위젯이 다를경우
			if (Prev != Current)
			{
				if (Prev && Prev->GetWidgetState() != EWidgetState::Disable)
				{
					//현재 위젯이 존재하고, 위젯이 disable이 아니면 본래 상태로 돌려준다.(Hovered가 아니게 처리)
					Prev->MouseUnHovered();
				}

				//현재 충돌된 위젯이 nullptr이 되도 상관은 없다.
				mHoveredWidget = HoveredWidget;


				if (Current && Current->GetWidgetState() != EWidgetState::Disable)
				{
					Current->MouseHovered();
				}
			}

			//위젯에 마우스가 올라와 있는 상태에서 마우스 버튼이 눌렸을 경우
			if (Current && Current->IsEnable()
				&& Current->GetWidgetState() != EWidgetState::Disable)
			{
				if (Current->IsMouseOn())
				{
					auto World = mWorld.lock();

					if (World)
					{
						auto Input = World->GetInput().lock();

						if (Input)
						{
							//위젯이 호버된 상태에서 마우스가 눌렸다면
							if (Current->GetWidgetState() == EWidgetState::Hovered && Input->GetMouseState(EMouseType::LButton, EInputType::Press))
							{
								Current->MouseClicked();
							}
							//반대로 위젯이 클릭된 상태에서 release호출되면
							else if (Current->GetWidgetState() == EWidgetState::Clicked && Input->GetMouseState(EMouseType::LButton, EInputType::Release))
							{
								Current->MouseRelease();
							}
							else if (Current->GetWidgetState() == EWidgetState::Release)
							{
								Current->SetWidgetState(EWidgetState::Hovered);
							}
						}

					}
				}
			}

			return true;
		}

		++iter;
	}

	//여기까지 코드가 진행됬으면
	//월드상에 충돌된 UI가 없다는 뜻
	if (!mHoveredWidget.expired())
	{
		auto Widget = mHoveredWidget.lock();

		if (Widget->GetWidgetState() != EWidgetState::Disable)
		{
			Widget->MouseUnHovered();
		}

		mHoveredWidget = std::weak_ptr<CWidget>();
	}

	return false;
}

void CUIManager::Render()
{
	//월드UI를 먼저 렌더링한뒤, 스크린 UI를 렌더링한다.
	//월드UI 렌더링을 진행하기 전 위젯이 유효한지 체크해준다.
	auto Worlditer = mWorldWidgetList.begin();
	auto WorlditerEnd = mWorldWidgetList.end();

	for (; Worlditer != WorlditerEnd;)
	{
		if (Worlditer->expired())
		{
			Worlditer = mWorldWidgetList.erase(Worlditer);
			WorlditerEnd = mWorldWidgetList.end();
			continue;
		}

		++Worlditer;
	}

	if (mWorldWidgetList.size() >= 2)
	{
		std::sort(mWorldWidgetList.begin(), mWorldWidgetList.end(), CUIManager::SortWorldRender);
	}

	Worlditer = mWorldWidgetList.begin();
	WorlditerEnd = mWorldWidgetList.end();

	//렌더링 확인
	for (; Worlditer != WorlditerEnd;)
	{
		auto Widget = Worlditer->lock();

		if (!Widget->IsAlive())
		{
			Worlditer = mWorldWidgetList.erase(Worlditer);
			WorlditerEnd = mWorldWidgetList.end();
			continue;
		}
		else if (!Widget->IsEnable())
		{
			++Worlditer;
			continue;
		}

		Widget->Render();

		++Worlditer;
	}

	if (mWidgetList.size() >= 2)
	{
		std::sort(mWidgetList.begin(), mWidgetList.end(), CUIManager::SortRender);
	}

	auto iter = mWidgetList.begin();
	auto iterEnd = mWidgetList.end();

	for (; iter != iterEnd;)
	{
		if (!(*iter)->IsAlive())
		{
			iter = mWidgetList.erase(iter);
			iterEnd = mWidgetList.end();
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

std::weak_ptr<CWidgetContainer> CUIManager::GetWidget(const std::string& Name)
{
	auto iter = mWidgetList.begin();
	auto iterEnd = mWidgetList.end();

	for (; iter != iterEnd; ++iter)
	{
		if ((*iter)->GetName() == Name)
		{
			return *iter;
		}
	}

	return std::weak_ptr<CWidgetContainer>();
}

bool CUIManager::SortRender(const std::shared_ptr<class CWidget>& Src, const std::shared_ptr<class CWidget>& Dest)
{
	//ZOrder 동률일 때 생성 순번으로 순서를 고정 (불안정 정렬로 인한 깜빡임 방지)
	if (Src->GetZOrder() != Dest->GetZOrder())
	{
		return Src->GetZOrder() < Dest->GetZOrder();
	}

	return Src->GetCreateSeq() < Dest->GetCreateSeq();
}

bool CUIManager::SortWorldRender(const std::weak_ptr<class CWidget>& Src, const std::weak_ptr<class CWidget>& Dest)
{
	return SortRender(Src.lock(), Dest.lock());
}

bool CUIManager::SortCollision(const std::shared_ptr<class CWidget>& Src, const std::shared_ptr<class CWidget>& Dest)
{
	//렌더 순서의 정확한 역순
	if (Src->GetZOrder() != Dest->GetZOrder())
	{
		return Src->GetZOrder() > Dest->GetZOrder();
	}

	return Src->GetCreateSeq() > Dest->GetCreateSeq();
}

bool CUIManager::SortWorldCollision(const std::weak_ptr<class CWidget>& Src, const std::weak_ptr<class CWidget>& Dest)
{
	return SortCollision(Src.lock(), Dest.lock());
}
