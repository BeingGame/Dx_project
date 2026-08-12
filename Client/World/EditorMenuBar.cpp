#include "EditorMenuBar.h"
#include "PrefabManager.h"

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

#include "World/TextBlock.h"

#include "LogManager.h"

static constexpr float BAR_H         = 40.f;
static constexpr float BTN_Y         = 4.f;
static constexpr float BTN_H         = 32.f;
static constexpr float SUBMENU_BTN_H = 32.f;
static constexpr float SUBMENU_W     = 220.f;

CEditorMenuBar::CEditorMenuBar()
{}

CEditorMenuBar::CEditorMenuBar(const CEditorMenuBar& src)
    : CWidgetContainer(src)
{}

CEditorMenuBar::~CEditorMenuBar()
{}

// ---- Helpers ----

static std::weak_ptr<CButton> MakeMenuButton(CWidgetContainer* Parent,
    const std::string& Name, float X, float W, const wchar_t* Label)
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

    auto Text = Parent->CreateWidget<CTextBlock>(Name + "_Lbl", 2);
    if (auto T = Text.lock())
    {
        T->SetPos(X, BTN_Y);
        T->SetSize(W, BTN_H);
        T->SetText(Label);
        T->SetFontSize(14.f);
        T->SetTextColor(FVector4::White);
        T->SetAlignH(ETextAlignH::Center);
        T->SetAlignV(ETextAlignV::Middle);
    }

    return Btn;
}

static std::weak_ptr<CButton> MakeSubmenuButton(CWidgetContainer* Parent,
    const std::string& Name, int Row, const wchar_t* Label)
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

    auto Text = Parent->CreateWidget<CTextBlock>(Name + "_Lbl", 2);
    if (auto T = Text.lock())
    {
        T->SetPos(0.f, Row * SUBMENU_BTN_H);
        T->SetSize(SUBMENU_W, SUBMENU_BTN_H);
        T->SetText(Label);
        T->SetFontSize(14.f);
        T->SetTextColor(FVector4::White);
        T->SetAlignH(ETextAlignH::Center);
        T->SetAlignV(ETextAlignV::Middle);
    }

    return Btn;
}

// ---- Init ----

bool CEditorMenuBar::Init()
{
    CWidgetContainer::Init();

    SetPos(0.f, 0.f);
    SetSize(1280.f, BAR_H);

    // 배경
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

    // "Empty Actor" (x=10)
    mEmptyActorButton = MakeMenuButton(this, "EmptyActorBtn", 10.f, 120.f, TEXT("Empty Actor"));
    if (auto B = mEmptyActorButton.lock())
        B->SetWidgetEventFunc(EWidgetEventState::Clicked, this, &CEditorMenuBar::OnEmptyActorClicked);

    // "Add Component" (x=140)
    mAddComponentButton = MakeMenuButton(this, "AddComponentBtn", 140.f, 150.f, TEXT("Add Component"));
    if (auto B = mAddComponentButton.lock())
        B->SetWidgetEventFunc(EWidgetEventState::Hovered, this, &CEditorMenuBar::OnAddComponentHovered);

    // Add Component 서브메뉴
    mSubmenu = CreateWidget<CWidgetContainer>("Submenu", 2);
    auto Submenu = mSubmenu.lock();
    if (Submenu)
    {
        Submenu->SetPos(140.f, BAR_H);
        Submenu->SetSize(SUBMENU_W, SUBMENU_BTN_H * 11.f);
        Submenu->SetEnable(false);

        auto AddCompBtn = [&](const std::string& Name, int Row, const wchar_t* Label, void(CEditorMenuBar::* Fn)())
        {
            auto Btn = MakeSubmenuButton(Submenu.get(), Name, Row, Label);
            if (auto B = Btn.lock())
                B->SetWidgetEventFunc(EWidgetEventState::Clicked, this, Fn);
            mComponentButtons.push_back(Btn);
        };

        AddCompBtn("MeshComponent",        0,  TEXT("Mesh Component"),        &CEditorMenuBar::OnMeshComponentClicked);
        AddCompBtn("CameraComponent",      1,  TEXT("Camera Component"),       &CEditorMenuBar::OnCameraComponentClicked);
        AddCompBtn("Animation2DComponent", 2,  TEXT("Animation2D Component"),  &CEditorMenuBar::OnAnimation2DComponentClicked);
        AddCompBtn("ColliderBox2D",        3,  TEXT("Collider Box2D"),         &CEditorMenuBar::OnColliderBox2DClicked);
        AddCompBtn("ColliderSphere2D",     4,  TEXT("Collider Sphere2D"),      &CEditorMenuBar::OnColliderSphere2DClicked);
        AddCompBtn("CharacterMovement",    5,  TEXT("Character Movement"),     &CEditorMenuBar::OnCharacterMovementClicked);
        AddCompBtn("ProjectileMovement",   6,  TEXT("Projectile Movement"),    &CEditorMenuBar::OnProjectileMovementClicked);
        AddCompBtn("AIComponent",          7,  TEXT("AI Component"),           &CEditorMenuBar::OnAIComponentClicked);
        AddCompBtn("SoundComponent",       8,  TEXT("Sound Component"),        &CEditorMenuBar::OnSoundComponentClicked);
        AddCompBtn("WidgetComponent",      9,  TEXT("Widget Component"),       &CEditorMenuBar::OnWidgetComponentClicked);
        AddCompBtn("TileMapComponent",     10, TEXT("TileMap Component"),      &CEditorMenuBar::OnTileMapComponentClicked);
    }

    // ---- "World" 메뉴 (x=300) ----
    mSceneButton = MakeMenuButton(this, "SceneBtn", 300.f, 100.f, TEXT("World"));
    if (auto B = mSceneButton.lock())
        B->SetWidgetEventFunc(EWidgetEventState::Hovered, this, &CEditorMenuBar::OnSceneHovered);

    mSceneSubmenu = CreateWidget<CWidgetContainer>("SceneSubmenu", 2);
    auto SceneSub = mSceneSubmenu.lock();
    if (SceneSub)
    {
        SceneSub->SetPos(300.f, BAR_H);
        SceneSub->SetSize(SUBMENU_W, SUBMENU_BTN_H * 2.f);
        SceneSub->SetEnable(false);

        auto SaveSceneBtn = MakeSubmenuButton(SceneSub.get(), "SaveSceneBtn", 0, TEXT("Save World"));
        if (auto B = SaveSceneBtn.lock())
            B->SetWidgetEventFunc(EWidgetEventState::Clicked, this, &CEditorMenuBar::OnSaveSceneClicked);
        mSceneSubmenuButtons.push_back(SaveSceneBtn);

        auto LoadSceneBtn = MakeSubmenuButton(SceneSub.get(), "LoadSceneBtn", 1, TEXT("Load World"));
        if (auto B = LoadSceneBtn.lock())
            B->SetWidgetEventFunc(EWidgetEventState::Clicked, this, &CEditorMenuBar::OnLoadSceneClicked);
        mSceneSubmenuButtons.push_back(LoadSceneBtn);
    }

    // ---- "Prefab" 메뉴 (x=410) ----
    mPrefabButton = MakeMenuButton(this, "PrefabBtn", 410.f, 100.f, TEXT("Prefab"));
    if (auto B = mPrefabButton.lock())
        B->SetWidgetEventFunc(EWidgetEventState::Hovered, this, &CEditorMenuBar::OnPrefabHovered);

    // Prefab 서브메뉴
    mPrefabSubmenu = CreateWidget<CWidgetContainer>("PrefabSubmenu", 2);
    auto PrefabSub = mPrefabSubmenu.lock();
    if (PrefabSub)
    {
        PrefabSub->SetPos(410.f, BAR_H);
        PrefabSub->SetSize(SUBMENU_W, SUBMENU_BTN_H * (1 + MAX_PREFAB_SLOTS));
        PrefabSub->SetEnable(false);

        // Row 0: Save Prefab
        auto SaveBtn = MakeSubmenuButton(PrefabSub.get(), "SavePrefabBtn", 0, TEXT("Save Prefab"));
        if (auto B = SaveBtn.lock())
            B->SetWidgetEventFunc(EWidgetEventState::Clicked, this, &CEditorMenuBar::OnSavePrefabClicked);
        mPrefabSubmenuButtons.push_back(SaveBtn);

        // 콜백 배열 (SetWidgetEventFunc는 멤버 함수 포인터만 받으므로 고정 메서드 사용)
        using SpawnFn = void(CEditorMenuBar::*)();
        const SpawnFn SpawnCallbacks[MAX_PREFAB_SLOTS] = {
            &CEditorMenuBar::OnSpawnPrefab0, &CEditorMenuBar::OnSpawnPrefab1,
            &CEditorMenuBar::OnSpawnPrefab2, &CEditorMenuBar::OnSpawnPrefab3,
            &CEditorMenuBar::OnSpawnPrefab4, &CEditorMenuBar::OnSpawnPrefab5
        };

        for (int i = 0; i < MAX_PREFAB_SLOTS; ++i)
        {
            std::string BtnName = "SpawnSlot" + std::to_string(i);
            auto SlotBtn = MakeSubmenuButton(PrefabSub.get(), BtnName, i + 1, TEXT("---"));

            if (auto B = SlotBtn.lock())
            {
                B->SetWidgetEventFunc(EWidgetEventState::Clicked, this, SpawnCallbacks[i]);
                B->SetWidgetEnable(false);
            }

            // Label은 BtnName + "_Lbl" 로 FindWidget
            mPrefabSlots[i].Btn   = SlotBtn;
            mPrefabSlots[i].Label = PrefabSub->FindWidget<CTextBlock>(BtnName + "_Lbl");
            mPrefabSlots[i].PrefabName = "";

            mPrefabSubmenuButtons.push_back(SlotBtn);
        }
    }

    return true;
}

// ---- Update ----

void CEditorMenuBar::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);

    // Add Component 서브메뉴 닫기 로직
    if (mSubmenuOpen)
    {
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
            if (auto S = mSubmenu.lock()) S->SetEnable(false);
        }
    }

    // Scene 서브메뉴 닫기 로직
    if (mSceneSubmenuOpen)
    {
        auto SceneBtn = mSceneButton.lock();
        bool AnyHovered = SceneBtn && (SceneBtn->GetWidgetState() == EWidgetState::Hovered);

        if (!AnyHovered)
        {
            for (auto& WeakBtn : mSceneSubmenuButtons)
            {
                auto B = WeakBtn.lock();
                if (B && B->GetWidgetState() == EWidgetState::Hovered) { AnyHovered = true; break; }
            }
        }

        if (!AnyHovered)
        {
            mSceneSubmenuOpen = false;
            if (auto S = mSceneSubmenu.lock()) S->SetEnable(false);
        }
    }

    // Prefab 서브메뉴 닫기 로직
    if (mPrefabSubmenuOpen)
    {
        auto PrefabBtn = mPrefabButton.lock();
        bool AnyHovered = PrefabBtn &&
            (PrefabBtn->GetWidgetState() == EWidgetState::Hovered);

        if (!AnyHovered)
        {
            for (auto& WeakBtn : mPrefabSubmenuButtons)
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
            mPrefabSubmenuOpen = false;
            if (auto S = mPrefabSubmenu.lock()) S->SetEnable(false);
        }
    }
}

CEditorMenuBar* CEditorMenuBar::Clone()
{
    return new CEditorMenuBar(*this);
}

// ---- Internal helpers ----

void CEditorMenuBar::TrackComponent(const std::string& TypeName,
                                     const std::string& CompName,
                                     const std::string& Parent)
{
    mTrackedComponents.push_back({ TypeName, CompName, Parent });
}

void CEditorMenuBar::ExecuteSpawnSlot(int Idx)
{
    if (Idx < 0 || Idx >= MAX_PREFAB_SLOTS) return;
    const std::string& Name = mPrefabSlots[Idx].PrefabName;
    if (Name.empty()) return;

    auto World = mWorld.lock();
    if (!World) return;

    CPrefabManager::GetInst()->SpawnPrefab(Name, World);
}

// ---- Add Component 콜백 ----

void CEditorMenuBar::OnEmptyActorClicked()
{
    auto World = mWorld.lock();
    if (!World) return;

    std::string Name = "EmptyActor_" + std::to_string(mActorCount++);
    auto NewActor = World->CreateActor<CActor>(Name);
    mSelectedActor = NewActor;
    mTrackedComponents.clear();  // 새 Actor이므로 추적 초기화

    LOG_DEBUG("[MenuBar] Created %s", Name.c_str());
}

void CEditorMenuBar::OnAddComponentHovered()
{
    if (mSubmenuOpen) return;
    mSubmenuOpen = true;
    if (auto S = mSubmenu.lock()) S->SetEnable(true);
}

void CEditorMenuBar::OnMeshComponentClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CMeshComponent>("MeshComponent");
    TrackComponent("MeshComponent", "MeshComponent");
    LOG_DEBUG("[MenuBar] Added MeshComponent");
}

void CEditorMenuBar::OnCameraComponentClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CCameraComponent>("CameraComponent");
    TrackComponent("CameraComponent", "CameraComponent");
    LOG_DEBUG("[MenuBar] Added CameraComponent");
}

void CEditorMenuBar::OnAnimation2DComponentClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CAnimation2DComponent>("Animation2DComponent");
    TrackComponent("Animation2DComponent", "Animation2DComponent");
    LOG_DEBUG("[MenuBar] Added Animation2DComponent");
}

void CEditorMenuBar::OnColliderBox2DClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CColliderBox2D>("ColliderBox2D");
    TrackComponent("ColliderBox2D", "ColliderBox2D");
    LOG_DEBUG("[MenuBar] Added ColliderBox2D");
}

void CEditorMenuBar::OnColliderSphere2DClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CColliderSphere2D>("ColliderSphere2D");
    TrackComponent("ColliderSphere2D", "ColliderSphere2D");
    LOG_DEBUG("[MenuBar] Added ColliderSphere2D");
}

void CEditorMenuBar::OnCharacterMovementClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CCharacterMovementComponent>("CharacterMovement");
    TrackComponent("CharacterMovement", "CharacterMovement");
    LOG_DEBUG("[MenuBar] Added CharacterMovementComponent");
}

void CEditorMenuBar::OnProjectileMovementClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CProjectileMovementComponent>("ProjectileMovement");
    TrackComponent("ProjectileMovement", "ProjectileMovement");
    LOG_DEBUG("[MenuBar] Added ProjectileMovementComponent");
}

void CEditorMenuBar::OnAIComponentClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CAIComponent>("AIComponent");
    TrackComponent("AIComponent", "AIComponent");
    LOG_DEBUG("[MenuBar] Added AIComponent");
}

void CEditorMenuBar::OnSoundComponentClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CSoundComponent>("SoundComponent");
    TrackComponent("SoundComponent", "SoundComponent");
    LOG_DEBUG("[MenuBar] Added SoundComponent");
}

void CEditorMenuBar::OnWidgetComponentClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CWidgetComponent>("WidgetComponent");
    TrackComponent("WidgetComponent", "WidgetComponent");
    LOG_DEBUG("[MenuBar] Added WidgetComponent");
}

void CEditorMenuBar::OnTileMapComponentClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CTileMapComponent>("TileMapComponent");
    TrackComponent("TileMapComponent", "TileMapComponent");
    LOG_DEBUG("[MenuBar] Added TileMapComponent");
}

// ---- Prefab 콜백 ----

void CEditorMenuBar::OnPrefabHovered()
{
    if (mPrefabSubmenuOpen) return;
    mPrefabSubmenuOpen = true;

    // 현재 Prefab 목록으로 슬롯 갱신
    auto Names = CPrefabManager::GetInst()->GetPrefabNames();

    for (int i = 0; i < MAX_PREFAB_SLOTS; ++i)
    {
        auto& Slot = mPrefabSlots[i];
        auto Btn   = Slot.Btn.lock();
        auto Lbl   = Slot.Label.lock();
        bool HasPrefab = (i < (int)Names.size());

        Slot.PrefabName = HasPrefab ? Names[i] : "";

        if (Btn) Btn->SetWidgetEnable(HasPrefab);

        if (Lbl)
        {
            if (HasPrefab)
            {
                std::wstring WName(Names[i].begin(), Names[i].end());
                Lbl->SetText(WName.c_str());
            }
            else
            {
                Lbl->SetText(TEXT("---"));
            }
        }
    }

    if (auto S = mPrefabSubmenu.lock()) S->SetEnable(true);
}

void CEditorMenuBar::OnSavePrefabClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }

    FPrefabData Data;
    Data.ActorTag   = Actor->GetActorTag();
    Data.WorldPos   = Actor->GetWorldPos();
    Data.WorldScale = Actor->GetWorldScale();
    Data.WorldRot   = Actor->GetWorldRot();
    Data.Components = mTrackedComponents;

    std::string PrefabName = "Prefab_" + std::to_string(mPrefabSaveCount++);
    CPrefabManager::GetInst()->SavePrefab(PrefabName, Data);
}

void CEditorMenuBar::OnSpawnPrefab0() { ExecuteSpawnSlot(0); }
void CEditorMenuBar::OnSpawnPrefab1() { ExecuteSpawnSlot(1); }
void CEditorMenuBar::OnSpawnPrefab2() { ExecuteSpawnSlot(2); }
void CEditorMenuBar::OnSpawnPrefab3() { ExecuteSpawnSlot(3); }
void CEditorMenuBar::OnSpawnPrefab4() { ExecuteSpawnSlot(4); }
void CEditorMenuBar::OnSpawnPrefab5() { ExecuteSpawnSlot(5); }

// ---- Scene 콜백 ----

void CEditorMenuBar::OnSceneHovered()
{
    if (mSceneSubmenuOpen) return;
    mSceneSubmenuOpen = true;
    if (auto S = mSceneSubmenu.lock()) S->SetEnable(true);
}

void CEditorMenuBar::OnSaveSceneClicked()
{
    auto World = mWorld.lock();
    if (!World) return;

    CreateDirectoryA("World", nullptr);
    std::string FilePath = "World/World_" + std::to_string(mSceneSaveCount++) + ".world";
    World->SaveScene(FilePath);
    LOG_DEBUG("[MenuBar] World saved: %s", FilePath.c_str());
}

void CEditorMenuBar::OnLoadSceneClicked()
{
    auto World = mWorld.lock();
    if (!World) return;

    std::string FilePath = "World/World_" + std::to_string(mSceneSaveCount > 0 ? mSceneSaveCount - 1 : 0) + ".world";
    if (World->LoadScene(FilePath))
        LOG_DEBUG("[MenuBar] World loaded: %s", FilePath.c_str());
    else
        LOG_ERROR("[MenuBar] World load failed: %s", FilePath.c_str());
}
