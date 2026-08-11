#include "EditorMenuBar.h"

#include "World/Button.h"
#include "World/WidgetContainer.h"
#include "World/Actor.h"
#include "World/World.h"

#include "World/MeshComponent.h"
#include "World/CameraComponent.h"
#include "World/Animation2DComponent.h"
#include "World/ColliderBox2D.h"
#include "World/ColliderSphere2D.h"
#include "World/CharacterMovementComponent.h"
#include "World/ProjectileMovementComponent.h"
#include "World/AIComponent.h"
#include "World/SoundComponent.h"
#include "World/WidgetComponent.h"
#include "World/TileMapComponent.h"

#include "LogManager.h"

static constexpr float BAR_H        = 40.f;
static constexpr float BTN_Y        = 4.f;
static constexpr float BTN_H        = 32.f;
static constexpr float SUBMENU_BTN_H = 32.f;
static constexpr float SUBMENU_W    = 220.f;

CEditorMenuBar::CEditorMenuBar()
{}

CEditorMenuBar::CEditorMenuBar(const CEditorMenuBar& src)
	: CWidgetContainer(src)
{}

CEditorMenuBar::~CEditorMenuBar()
{}

// Helper: create a menu bar button at the given x position
static std::weak_ptr<CButton> MakeMenuButton(CWidgetContainer* Parent,
	const std::string& Name, float X, float W)
{
	auto Btn = Parent->CreateWidget<CButton>(Name, 1);
	auto B = Btn.lock();
	if (B)
	{
		B->SetPos(X, BTN_Y);
		B->SetSize(W, BTN_H);
		B->SetTint(EWidgetState::Normal,  0.65f, 0.65f, 0.70f, 1.f);
		B->SetTint(EWidgetState::Hovered, 0.85f, 0.85f, 1.00f, 1.f);
		B->SetTint(EWidgetState::Clicked, 1.00f, 1.00f, 1.00f, 1.f);
		B->SetTint(EWidgetState::Release, 0.85f, 0.85f, 1.00f, 1.f);
		B->SetTint(EWidgetState::Disable, 0.35f, 0.35f, 0.35f, 0.5f);
	}
	return Btn;
}

// Helper: create a submenu item button
static std::weak_ptr<CButton> MakeSubmenuButton(CWidgetContainer* Parent,
	const std::string& Name, int Row)
{
	auto Btn = Parent->CreateWidget<CButton>(Name, 1);
	auto B = Btn.lock();
	if (B)
	{
		B->SetPos(0.f, Row * SUBMENU_BTN_H);
		B->SetSize(SUBMENU_W, SUBMENU_BTN_H);
		B->SetTint(EWidgetState::Normal,  0.55f, 0.55f, 0.60f, 0.95f);
		B->SetTint(EWidgetState::Hovered, 0.40f, 0.60f, 1.00f, 1.f);
		B->SetTint(EWidgetState::Clicked, 0.60f, 0.80f, 1.00f, 1.f);
		B->SetTint(EWidgetState::Release, 0.40f, 0.60f, 1.00f, 1.f);
		B->SetTint(EWidgetState::Disable, 0.30f, 0.30f, 0.30f, 0.5f);
	}
	return Btn;
}

bool CEditorMenuBar::Init()
{
	CWidgetContainer::Init();

	SetPos(0.f, 0.f);
	SetSize(1280.f, BAR_H);

	// 메뉴바 배경 버튼 (전체 너비, 클릭 이벤트 없음 - 배경 역할)
	auto BgBtn = CreateWidget<CButton>("MenuBarBg", 0);
	if (auto Bg = BgBtn.lock())
	{
		Bg->SetPos(0.f, 0.f);
		Bg->SetSize(1280.f, BAR_H);
		Bg->SetTint(EWidgetState::Normal,  0.20f, 0.20f, 0.22f, 0.95f);
		Bg->SetTint(EWidgetState::Hovered, 0.20f, 0.20f, 0.22f, 0.95f);
		Bg->SetTint(EWidgetState::Clicked, 0.20f, 0.20f, 0.22f, 0.95f);
		Bg->SetTint(EWidgetState::Release, 0.20f, 0.20f, 0.22f, 0.95f);
		Bg->SetTint(EWidgetState::Disable, 0.20f, 0.20f, 0.22f, 0.95f);
	}

	// "Empty Actor" button (x=10)
	mEmptyActorButton = MakeMenuButton(this, "EmptyActorBtn", 10.f, 120.f);
	if (auto B = mEmptyActorButton.lock())
		B->SetWidgetEventFunc(EWidgetEventState::Clicked, this, &CEditorMenuBar::OnEmptyActorClicked);

	// "Add Component" button (x=140)
	mAddComponentButton = MakeMenuButton(this, "AddComponentBtn", 140.f, 150.f);
	if (auto B = mAddComponentButton.lock())
		B->SetWidgetEventFunc(EWidgetEventState::Hovered, this, &CEditorMenuBar::OnAddComponentHovered);

	// Submenu container (directly below "Add Component", starts hidden)
	mSubmenu = CreateWidget<CWidgetContainer>("Submenu", 2);
	auto Submenu = mSubmenu.lock();
	if (Submenu)
	{
		Submenu->SetPos(140.f, BAR_H);
		Submenu->SetSize(SUBMENU_W, SUBMENU_BTN_H * 11.f);
		Submenu->SetEnable(false);

		auto AddCompBtn = [&](const std::string& Name, int Row, void(CEditorMenuBar::* Fn)())
		{
			auto Btn = MakeSubmenuButton(Submenu.get(), Name, Row);
			if (auto B = Btn.lock())
				B->SetWidgetEventFunc(EWidgetEventState::Clicked, this, Fn);
			mComponentButtons.push_back(Btn);
		};

		AddCompBtn("MeshComponent",       0,  &CEditorMenuBar::OnMeshComponentClicked);
		AddCompBtn("CameraComponent",     1,  &CEditorMenuBar::OnCameraComponentClicked);
		AddCompBtn("Animation2DComponent",2,  &CEditorMenuBar::OnAnimation2DComponentClicked);
		AddCompBtn("ColliderBox2D",        3,  &CEditorMenuBar::OnColliderBox2DClicked);
		AddCompBtn("ColliderSphere2D",     4,  &CEditorMenuBar::OnColliderSphere2DClicked);
		AddCompBtn("CharacterMovement",   5,  &CEditorMenuBar::OnCharacterMovementClicked);
		AddCompBtn("ProjectileMovement",  6,  &CEditorMenuBar::OnProjectileMovementClicked);
		AddCompBtn("AIComponent",          7,  &CEditorMenuBar::OnAIComponentClicked);
		AddCompBtn("SoundComponent",       8,  &CEditorMenuBar::OnSoundComponentClicked);
		AddCompBtn("WidgetComponent",      9,  &CEditorMenuBar::OnWidgetComponentClicked);
		AddCompBtn("TileMapComponent",     10, &CEditorMenuBar::OnTileMapComponentClicked);
	}

	return true;
}

void CEditorMenuBar::Update(float DeltaTime)
{
	CWidgetContainer::Update(DeltaTime);

	if (!mSubmenuOpen) return;

	// Keep submenu open while AddComp button or any submenu button is hovered
	auto AddCompBtn = mAddComponentButton.lock();
	bool AnyHovered = AddCompBtn &&
		(AddCompBtn->GetWidgetState() == EWidgetState::Hovered);

	if (!AnyHovered)
	{
		for (auto& WeakBtn : mComponentButtons)
		{
			auto B = WeakBtn.lock();
			if (B && B->GetWidgetState() == EWidgetState::Hovered)
			{
				AnyHovered = true;
				break;
			}
		}
	}

	if (!AnyHovered)
	{
		mSubmenuOpen = false;
		auto Submenu = mSubmenu.lock();
		if (Submenu) Submenu->SetEnable(false);
	}
}

CEditorMenuBar* CEditorMenuBar::Clone()
{
	return new CEditorMenuBar(*this);
}

// ---- Button Callbacks ----

void CEditorMenuBar::OnEmptyActorClicked()
{
	auto World = mWorld.lock();
	if (!World) return;

	std::string Name = "EmptyActor_" + std::to_string(mActorCount++);
	auto NewActor = World->CreateActor<CActor>(Name);
	mSelectedActor = NewActor;

	LOG_DEBUG("Created %s", Name.c_str());
}

void CEditorMenuBar::OnAddComponentHovered()
{
	if (mSubmenuOpen) return;
	mSubmenuOpen = true;
	auto Submenu = mSubmenu.lock();
	if (Submenu) Submenu->SetEnable(true);
}

void CEditorMenuBar::OnMeshComponentClicked()
{
	auto Actor = mSelectedActor.lock();
	if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected. Create one first."); return; }
	Actor->CreateComponent<CMeshComponent>("MeshComponent");
	LOG_DEBUG("[MenuBar] Added MeshComponent");
}

void CEditorMenuBar::OnCameraComponentClicked()
{
	auto Actor = mSelectedActor.lock();
	if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
	Actor->CreateComponent<CCameraComponent>("CameraComponent");
	LOG_DEBUG("[MenuBar] Added CameraComponent");
}

void CEditorMenuBar::OnAnimation2DComponentClicked()
{
	auto Actor = mSelectedActor.lock();
	if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
	Actor->CreateComponent<CAnimation2DComponent>("Animation2DComponent");
	LOG_DEBUG("[MenuBar] Added Animation2DComponent");
}

void CEditorMenuBar::OnColliderBox2DClicked()
{
	auto Actor = mSelectedActor.lock();
	if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
	Actor->CreateComponent<CColliderBox2D>("ColliderBox2D");
	LOG_DEBUG("[MenuBar] Added ColliderBox2D");
}

void CEditorMenuBar::OnColliderSphere2DClicked()
{
	auto Actor = mSelectedActor.lock();
	if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
	Actor->CreateComponent<CColliderSphere2D>("ColliderSphere2D");
	LOG_DEBUG("[MenuBar] Added ColliderSphere2D");
}

void CEditorMenuBar::OnCharacterMovementClicked()
{
	auto Actor = mSelectedActor.lock();
	if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
	Actor->CreateComponent<CCharacterMovementComponent>("CharacterMovement");
	LOG_DEBUG("[MenuBar] Added CharacterMovementComponent");
}

void CEditorMenuBar::OnProjectileMovementClicked()
{
	auto Actor = mSelectedActor.lock();
	if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
	Actor->CreateComponent<CProjectileMovementComponent>("ProjectileMovement");
	LOG_DEBUG("[MenuBar] Added ProjectileMovementComponent");
}

void CEditorMenuBar::OnAIComponentClicked()
{
	auto Actor = mSelectedActor.lock();
	if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
	Actor->CreateComponent<CAIComponent>("AIComponent");
	LOG_DEBUG("[MenuBar] Added AIComponent");
}

void CEditorMenuBar::OnSoundComponentClicked()
{
	auto Actor = mSelectedActor.lock();
	if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
	Actor->CreateComponent<CSoundComponent>("SoundComponent");
	LOG_DEBUG("[MenuBar] Added SoundComponent");
}

void CEditorMenuBar::OnWidgetComponentClicked()
{
	auto Actor = mSelectedActor.lock();
	if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
	Actor->CreateComponent<CWidgetComponent>("WidgetComponent");
	LOG_DEBUG("[MenuBar] Added WidgetComponent");
}

void CEditorMenuBar::OnTileMapComponentClicked()
{
	auto Actor = mSelectedActor.lock();
	if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
	Actor->CreateComponent<CTileMapComponent>("TileMapComponent");
	LOG_DEBUG("[MenuBar] Added TileMapComponent");
}
