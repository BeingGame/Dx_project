#include "Button.h"

#include "../Asset/AssetManager.h"
#include "../Asset/TextureManager.h"
#include "../Asset/SoundManager.h"
#include "../Asset/Texture.h"
#include "../Asset/Sound.h"

CButton::CButton()
{}

CButton::CButton(const CButton& src)
	:CWidget(src)
{
	//자식은 복사하지 않을수도 있다.
	mChild = std::shared_ptr<CWidget>(src.mChild->Clone());

	for (int i = 0; i < EWidgetState::End; ++i)
	{
		mBrush[i] = src.mBrush[i];
		mSound[i] = src.mSound[i];
	}
}

CButton::~CButton()
{}

void CButton::SetChild(const std::weak_ptr<CWidget>& Child)
{
	//변수로 받아온 Child가 유효한지 아닌지에 따라
	//달라진다.
	if (!Child.expired())
	{
		//자식 유효하면 갱신한다.
		if (mChild)
		{
			mChild->Detach();
		}

		mChild = Child.lock();

		mChild->SetParent(GetThisPtr<CWidget>());
		mChild->SetSize(mSize);
		mChild->SetUIManager(mUIManager);
		mChild->SetWorld(mWorld);
	}
	else
	{
		if (mChild)
		{
			mChild->Detach();
		}

		mChild.reset();
	}
}

void CButton::RemoveChild(std::shared_ptr<CWidget> Child)
{
	if (mChild == Child)
	{
		mChild.reset();
	}
}

void CButton::SetOpacityAll(float Opacity)
{
	CWidget::SetOpacityAll(Opacity);

	for (int i = 0; i < EWidgetState::End; ++i)
	{
		mBrush[i].Tint.w = Opacity;
	}

	if (mChild)
	{
		mChild->SetOpacityAll(Opacity);
	}
}

bool CButton::SetTexture(EWidgetState::Type State, const std::weak_ptr<class CTexture> Texture)
{
	mBrush[State].Texture = Texture;

	return true;
}

bool CButton::SetTexture(EWidgetState::Type State, const std::string& Name)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (!TextureMgr)
	{
		return false;

	}

	mBrush[State].Texture = TextureMgr->FindTexture(Name);

	return true;
}

bool CButton::SetTexture(EWidgetState::Type State, const std::string& Name, const TCHAR* FileName, const std::string& PathName)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (!TextureMgr)
	{
		return false;

	}

	if (!TextureMgr->LoadTexture(Name, FileName, PathName))
	{
		return false;
	}

	mBrush[State].Texture = TextureMgr->FindTexture(Name);

	return true;
}

void CButton::SetTint(EWidgetState::Type State, const FVector4& Color)
{
	mBrush[State].Tint = Color;
}

void CButton::SetTint(EWidgetState::Type State, float r, float g, float b, float a)
{
	mBrush[State].Tint = FVector4(r, g, b, a);
}

void CButton::SetBrushAnimation(EWidgetState::Type State, bool AnimationEnable)
{
	mBrush[State].AnimationEnable = AnimationEnable;
}

void CButton::AddBrushFrame(EWidgetState::Type State, const FVector2& Start, const FVector2& Size)
{
	FTextureFrame Frame;

	Frame.Start = Start;
	Frame.Size = Size;

	mBrush[State].AnimationFrames.push_back(Frame);

	mBrush[State].FrameTime = mBrush[State].PlayTime / mBrush[State].AnimationFrames.size();
}

void CButton::AddBrushFrame(EWidgetState::Type State, float StartX, float StartY, float SizeX, float SizeY)
{
	AddBrushFrame(State, FVector2(StartX, StartY), FVector2(SizeX, SizeY));
}

void CButton::SetCurrentFrame(EWidgetState::Type State, int Frame)
{
	mBrush[State].Frame = Frame;
}

void CButton::SetAnimationPlayTime(EWidgetState::Type State, float PlayTime)
{
	mBrush[State].PlayTime = PlayTime;

	mBrush[State].FrameTime = mBrush[State].PlayTime / mBrush[State].AnimationFrames.size();
}

void CButton::SetAnimationPlayRate(EWidgetState::Type State, float PlayRate)
{
	mBrush[State].PlayRate = PlayRate;
}

void CButton::SetSound(EWidgetEventState::Type State, const std::string& Name)
{
	auto SoundMgr = CAssetManager::GetInst()->GetSubManager<CSoundManager>(EAssetType::Sound);

	if (SoundMgr)
	{
		mSound[State] = SoundMgr->FindSound(Name);
	}
}

void CButton::SetSound(EWidgetEventState::Type State, const std::string& Name, const char* FileName, const std::string& PathName)
{
	auto SoundMgr = CAssetManager::GetInst()->GetSubManager<CSoundManager>(EAssetType::Sound);

	if (SoundMgr)
	{
		if (!SoundMgr->LoadSound(Name, "UI", false, FileName, PathName))
		{
			return;
		}

		mSound[State] = SoundMgr->FindSound(Name);
	}
}

bool CButton::Init()
{
	CWidget::Init();

	for (int i = 0; i < EWidgetState::End; ++i)
	{
		SetTexture((EWidgetState::Type)i, "DefaultButton", TEXT("Start.png"));
	}

	SetTint(EWidgetState::Hovered, 1.f, 1.f, 1.f, 0.7f);
	SetTint(EWidgetState::Clicked, 1.f, 1.f, 1.f, 0.4f);
	SetTint(EWidgetState::Disable, 0.3f, 0.3f, 0.3f, 1.f);

	return true;
}

void CButton::Update(float DeltaTime)
{
	CWidget::Update(DeltaTime);
}

void CButton::Render()
{
	CWidget::Render();

	RenderBrush(mBrush[mWidgetState], mRenderPos, mSize);

	if (mChild)
	{
		mChild->Render();
	}
}

bool CButton::CollisionMouse(std::weak_ptr<CWidget>& Result, const FVector2& MousePos)
{
	if (mChild)
	{
		if (mChild->CollisionMouse(Result, MousePos))
		{
			return true;
		}
	}

	return CWidget::CollisionMouse(Result, MousePos);
}

void CButton::MouseHovered()
{
	CWidget::MouseHovered();

	if (!mSound[EWidgetEventState::Hovered].expired())
	{
		mSound[EWidgetEventState::Hovered].lock()->Play();
	}
}

void CButton::MouseUnHovered()
{
	CWidget::MouseUnHovered();
}

void CButton::MouseClicked()
{
	CWidget::MouseClicked();

	if (!mSound[EWidgetEventState::Clicked].expired())
	{
		mSound[EWidgetEventState::Clicked].lock()->Play();
	}
}

void CButton::MouseRelease()
{
	CWidget::MouseRelease();
}

CButton* CButton::Clone()
{
	return new CButton(*this);
}
