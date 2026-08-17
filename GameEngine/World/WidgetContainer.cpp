#include "WidgetContainer.h"

#include "World.h"
#include "Input.h"

CWidgetContainer::CWidgetContainer()
{}

CWidgetContainer::CWidgetContainer(const CWidgetContainer& src)
	:CWidget(src)
{
	mScrollEnable = src.mScrollEnable;
	mScrollY = src.mScrollY;
	mScrollTop = src.mScrollTop;
	mScrollBottom = src.mScrollBottom;
	mScrollContentEnd = src.mScrollContentEnd;
	mScrollStep = src.mScrollStep;
	mScrollBarW = src.mScrollBarW;

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

// ── 세로 스크롤 ──────────────────────────────────────────────────────────────

void CWidgetContainer::SetScrollY(float Y)
{
	float MaxY = GetScrollMax();

	if (Y < 0.f)
	{
		Y = 0.f;
	}

	if (Y > MaxY)
	{
		Y = MaxY;
	}

	mScrollY = Y;
}

bool CWidgetContainer::IsScrolledOut(const std::shared_ptr<CWidget>& Child) const
{
	if (!mScrollEnable || !Child || !Child->IsScrollTarget())
	{
		return false;
	}

	//스크롤이 적용된 뒤의 로컬 y 구간
	float Top = Child->GetPos().y - mScrollY;
	float Bottom = Top + Child->GetSize().y;

	//가위 클리핑이 없으므로 조금이라도 걸친 위젯은 통째로 감춘다.
	//그렇지 않으면 타이틀바 위나 패널 바깥으로 삐져나온다.
	return (Top < mScrollTop - 0.5f) || (Bottom > mScrollBottom + 0.5f);
}

void CWidgetContainer::UpdateScroll()
{
	if (!mScrollEnable)
	{
		return;
	}

	//충돌/렌더 이전에 호출될 수 있으므로 여기서 한번 위치를 확정해둔다.
	auto Parent = mParent.lock();

	if (Parent)
	{
		mRenderPos = Parent->GetRenderPos() + mPos + mScrollOffset;
	}
	else
	{
		mRenderPos = mPos + mScrollOffset;
	}

	//콘텐츠가 줄어들어 현재 스크롤 양이 범위를 벗어났으면 다시 당겨준다.
	SetScrollY(mScrollY);

	//── 휠 입력 ──
	auto World = mWorld.lock();

	if (World)
	{
		auto Input = World->GetInput().lock();

		if (Input)
		{
			float Wheel = Input->GetMouseWheelDelta();

			if (Wheel != 0.f)
			{
				FVector2 Mouse = Input->GetMousePos();

				//패널의 스크롤 영역 위에 마우스가 있을 때만 반응한다.
				if (Mouse.x >= mRenderPos.x && Mouse.x < mRenderPos.x + mSize.x &&
					Mouse.y >= mRenderPos.y + mScrollTop && Mouse.y < mRenderPos.y + mScrollBottom)
				{
					SetScrollY(mScrollY + (Wheel > 0.f ? -mScrollStep : mScrollStep));
				}
			}
		}
	}

	//── 자식에게 이동량 전달 ──
	FVector3 Offset(0.f, -mScrollY, 0.f);

	for (auto& Child : mChildList)
	{
		if (!Child->IsScrollTarget())
		{
			continue;
		}

		Child->SetScrollOffset(Offset);

		//영역 밖으로 나간 위젯은 충돌 대상에서 빠지기 때문에
		//UIManager가 Release 상태를 되돌려주지 못한다.
		//그대로 두면 다시 화면에 들어왔을 때(혹은 그 전에)
		//패널의 버튼 처리 루프가 눌리지도 않은 버튼을 계속 처리하게 된다.
		if (IsScrolledOut(Child) && Child->GetWidgetState() != EWidgetState::Disable)
		{
			Child->SetWidgetState(EWidgetState::Normal);
		}
	}
}

void CWidgetContainer::RenderScrollBar()
{
	if (!mScrollEnable)
	{
		return;
	}

	float ViewH = mScrollBottom - mScrollTop;
	float MaxY = GetScrollMax();

	//스크롤할 것이 없으면 스크롤바도 그리지 않는다.
	if (ViewH <= 0.f || MaxY <= 0.f)
	{
		return;
	}

	float ContentH = ViewH + MaxY;

	float X = mRenderPos.x + mSize.x - mScrollBarW - 1.f;
	float TrackY = mRenderPos.y + mScrollTop;

	FUIBrush Track;
	Track.Tint = FVector4(0.f, 0.f, 0.f, 0.35f);
	RenderBrush(Track, FVector3(X, TrackY, 0.f), FVector3(mScrollBarW, ViewH, 1.f));

	float ThumbH = ViewH * (ViewH / ContentH);

	if (ThumbH < 16.f)
	{
		ThumbH = 16.f;
	}

	float ThumbY = TrackY + (ViewH - ThumbH) * (mScrollY / MaxY);

	FUIBrush Thumb;
	Thumb.Tint = FVector4(0.55f, 0.60f, 0.75f, 0.9f);
	RenderBrush(Thumb, FVector3(X, ThumbY, 0.f), FVector3(mScrollBarW, ThumbH, 1.f));
}

void CWidgetContainer::Update(float DeltaTime)
{
	CWidget::Update(DeltaTime);

	UpdateScroll();

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
		//스크롤 영역 밖으로 나간 자식은 그리지 않는다.
		else if (IsScrolledOut(*iter))
		{
			++iter;
			continue;
		}

		(*iter)->Render();

		++iter;
	}

	//자식 위에 스크롤바를 얹는다.
	RenderScrollBar();
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
		//스크롤 영역 밖으로 나간 자식은 클릭도 받지 않는다.
		else if (IsScrolledOut(*iter))
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
