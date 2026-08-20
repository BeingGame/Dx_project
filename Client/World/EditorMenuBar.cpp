#include "EditorMenuBar.h"
#include "PrefabManager.h"

#include "World/Button.h"
#include "World/WidgetContainer.h"
#include "World/Actor.h"
#include "World/World.h"
#include "World/Input.h"

#include "World/MeshComponent.h"
#include "World/CameraComponent.h"
#include "World/Animation2DComponent.h"
#include "Component/DirectionInputComponent.h"
#include "Component/ActionStateComponent.h"
#include "Component/HeightComponent.h"
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

#include "DialogUtil.h"

static constexpr float BAR_H         = 40.f;
static constexpr float BTN_Y         = 4.f;
static constexpr float BTN_H         = 32.f;
static constexpr float SUBMENU_BTN_H = 32.f;
static constexpr float SUBMENU_W     = 220.f;
// 서브메뉴가 이 줄 수를 넘으면 높이를 여기서 자르고 휠 스크롤로 넘긴다.
static constexpr int   SUBMENU_MAX_ROWS = 10;

CEditorMenuBar::CEditorMenuBar()
{}

CEditorMenuBar::CEditorMenuBar(const CEditorMenuBar& src)
    : CWidgetContainer(src)
{}

CEditorMenuBar::~CEditorMenuBar()
{}

// ---- 헬퍼 함수 ----

static std::weak_ptr<CButton> MakeMenuButton(CWidgetContainer* Parent,
    const std::string& Name, float X, float Width, const wchar_t* Label)
{
    auto ButtonWeak = Parent->CreateWidget<CButton>(Name, 1);
    auto Button = ButtonWeak.lock();
    if (Button)
    {
        Button->SetPos(X, BTN_Y);
        Button->SetSize(Width, BTN_H);
        Button->SetTint(EWidgetState::Normal,  0.65f, 0.65f, 0.70f, 1.f);
        Button->SetTint(EWidgetState::Hovered, 0.85f, 0.85f, 1.00f, 1.f);
        Button->SetTint(EWidgetState::Clicked, 1.00f, 1.00f, 1.00f, 1.f);
        Button->SetTint(EWidgetState::Release, 0.85f, 0.85f, 1.00f, 1.f);
        Button->SetTint(EWidgetState::Disable, 0.35f, 0.35f, 0.35f, 0.5f);
    }

    auto TextWeak = Parent->CreateWidget<CTextBlock>(Name + "_Lbl", 2);
    if (auto Text = TextWeak.lock())
    {
        Text->SetPos(X, BTN_Y);
        Text->SetSize(Width, BTN_H);
        Text->SetText(Label);
        Text->SetFontSize(14.f);
        Text->SetTextColor(FVector4::White);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Middle);
    }

    return ButtonWeak;
}

static std::weak_ptr<CButton> MakeSubmenuButton(CWidgetContainer* Parent,
    const std::string& Name, int Row, const wchar_t* Label, bool bScrollTarget = false)
{
    auto ButtonWeak = Parent->CreateWidget<CButton>(Name, 1);
    auto Button = ButtonWeak.lock();
    if (Button)
    {
        Button->SetPos(0.f, Row * SUBMENU_BTN_H);
        Button->SetSize(SUBMENU_W, SUBMENU_BTN_H);
        Button->SetTint(EWidgetState::Normal,  0.55f, 0.55f, 0.60f, 0.95f);
        Button->SetTint(EWidgetState::Hovered, 0.40f, 0.60f, 1.00f, 1.f);
        Button->SetTint(EWidgetState::Clicked, 0.60f, 0.80f, 1.00f, 1.f);
        Button->SetTint(EWidgetState::Release, 0.40f, 0.60f, 1.00f, 1.f);
        Button->SetTint(EWidgetState::Disable, 0.30f, 0.30f, 0.30f, 0.5f);

        // 스크롤되는 서브메뉴는 버튼/텍스트를 스크롤 대상으로 표시해야
        // 휠에 맞춰 같이 움직이고, 영역 밖으로 나가면 자동으로 숨겨진다.
        if (bScrollTarget) Button->SetScrollTarget(true);
    }

    auto TextWeak = Parent->CreateWidget<CTextBlock>(Name + "_Lbl", 2);
    if (auto Text = TextWeak.lock())
    {
        Text->SetPos(0.f, Row * SUBMENU_BTN_H);
        Text->SetSize(SUBMENU_W, SUBMENU_BTN_H);
        Text->SetText(Label);
        Text->SetFontSize(14.f);
        Text->SetTextColor(FVector4::White);
        Text->SetAlignH(ETextAlignH::Center);
        Text->SetAlignV(ETextAlignV::Middle);

        if (bScrollTarget) Text->SetScrollTarget(true);
    }

    return ButtonWeak;
}

// ---- 초기화 ----

bool CEditorMenuBar::Init()
{
    CWidgetContainer::Init();

    SetPos(0.f, 0.f);
    SetSize(1280.f, BAR_H);

    // 배경
    auto BackgroundButton = CreateWidget<CButton>("MenuBarBg", 0);
    if (auto Background = BackgroundButton.lock())
    {
        Background->SetPos(0.f, 0.f);
        Background->SetSize(1280.f, BAR_H);
        Background->SetTint(EWidgetState::Normal,  0.20f, 0.20f, 0.22f, 0.95f);
        Background->SetTint(EWidgetState::Hovered, 0.20f, 0.20f, 0.22f, 0.95f);
        Background->SetTint(EWidgetState::Clicked, 0.20f, 0.20f, 0.22f, 0.95f);
        Background->SetTint(EWidgetState::Release, 0.20f, 0.20f, 0.22f, 0.95f);
        Background->SetTint(EWidgetState::Disable, 0.20f, 0.20f, 0.22f, 0.95f);
    }

    // 빈 액터 생성 버튼 (x=10)
    mEmptyActorButton = MakeMenuButton(this, "EmptyActorBtn", 10.f, 120.f, TEXT("Empty Actor"));
    if (auto Button = mEmptyActorButton.lock())
        Button->SetWidgetEventFunc(EWidgetEventState::Clicked, this, &CEditorMenuBar::OnEmptyActorClicked);

    // 컴포넌트 추가 버튼 (x=140)
    mAddComponentButton = MakeMenuButton(this, "AddComponentBtn", 140.f, 150.f, TEXT("Add Component"));
    if (auto Button = mAddComponentButton.lock())
        Button->SetWidgetEventFunc(EWidgetEventState::Hovered, this, &CEditorMenuBar::OnAddComponentHovered);

    // 컴포넌트 추가 서브메뉴
    mSubmenu = CreateWidget<CWidgetContainer>("Submenu", 2);
    auto Submenu = mSubmenu.lock();
    if (Submenu)
    {
        Submenu->SetPos(140.f, BAR_H);
        Submenu->SetEnable(false);

        auto AddComponentButton = [&](const std::string& Name, int Row, const wchar_t* Label, void(CEditorMenuBar::* ClickFunc)())
        {
            auto ButtonWeak = MakeSubmenuButton(Submenu.get(), Name, Row, Label, true);
            if (auto Button = ButtonWeak.lock())
                Button->SetWidgetEventFunc(EWidgetEventState::Clicked, this, ClickFunc);
            mComponentButtons.push_back(ButtonWeak);
        };

        AddComponentButton("MeshComponent",        0,  TEXT("Mesh Component"),        &CEditorMenuBar::OnMeshComponentClicked);
        AddComponentButton("CameraComponent",      1,  TEXT("Camera Component"),       &CEditorMenuBar::OnCameraComponentClicked);
        AddComponentButton("Animation2DComponent", 2,  TEXT("Animation2D Component"),  &CEditorMenuBar::OnAnimation2DComponentClicked);
        AddComponentButton("ColliderBox2D",        3,  TEXT("Collider Box2D"),         &CEditorMenuBar::OnColliderBox2DClicked);
        AddComponentButton("ColliderSphere2D",     4,  TEXT("Collider Sphere2D"),      &CEditorMenuBar::OnColliderSphere2DClicked);
        AddComponentButton("CharacterMovement",    5,  TEXT("Character Movement"),     &CEditorMenuBar::OnCharacterMovementClicked);
        AddComponentButton("ProjectileMovement",   6,  TEXT("Projectile Movement"),    &CEditorMenuBar::OnProjectileMovementClicked);
        AddComponentButton("AIComponent",          7,  TEXT("AI Component"),           &CEditorMenuBar::OnAIComponentClicked);
        AddComponentButton("SoundComponent",       8,  TEXT("Sound Component"),        &CEditorMenuBar::OnSoundComponentClicked);
        AddComponentButton("WidgetComponent",      9,  TEXT("Widget Component"),       &CEditorMenuBar::OnWidgetComponentClicked);
        AddComponentButton("TileMapComponent",     10, TEXT("TileMap Component"),      &CEditorMenuBar::OnTileMapComponentClicked);
        AddComponentButton("DirectionInput",       11, TEXT("Direction Input"),        &CEditorMenuBar::OnDirectionInputComponentClicked);
        AddComponentButton("ActionState",          12, TEXT("Action State"),           &CEditorMenuBar::OnActionStateComponentClicked);
        AddComponentButton("Height",               13, TEXT("Height (Jump/Fall)"),     &CEditorMenuBar::OnHeightComponentClicked);

        // 줄 수가 SUBMENU_MAX_ROWS를 넘으면 높이를 잘라 화면 밖으로 나가지 않게 하고,
        // 나머지는 휠 스크롤로 넘긴다. (넘지 않으면 스크롤바가 안 그려져 예전과 동일)
        int   TotalRows = (int)mComponentButtons.size();
        int   ViewRows  = (TotalRows < SUBMENU_MAX_ROWS) ? TotalRows : SUBMENU_MAX_ROWS;
        float ViewH     = SUBMENU_BTN_H * (float)ViewRows;

        Submenu->SetSize(SUBMENU_W, ViewH);
        Submenu->EnableScroll(true);
        Submenu->SetScrollArea(0.f, ViewH);
        Submenu->SetScrollContentEnd(SUBMENU_BTN_H * (float)TotalRows);
        Submenu->SetScrollStep(SUBMENU_BTN_H);
    }

    // ---- "World" 메뉴 (x=300) ----
    mSceneButton = MakeMenuButton(this, "SceneBtn", 300.f, 100.f, TEXT("World"));
    if (auto Button = mSceneButton.lock())
        Button->SetWidgetEventFunc(EWidgetEventState::Hovered, this, &CEditorMenuBar::OnSceneHovered);

    mSceneSubmenu = CreateWidget<CWidgetContainer>("SceneSubmenu", 2);
    auto WorldSubmenu = mSceneSubmenu.lock();
    if (WorldSubmenu)
    {
        WorldSubmenu->SetPos(300.f, BAR_H);
        WorldSubmenu->SetSize(SUBMENU_W, SUBMENU_BTN_H * 2.f);
        WorldSubmenu->SetEnable(false);

        auto SaveWorldButton = MakeSubmenuButton(WorldSubmenu.get(), "SaveSceneBtn", 0, TEXT("Save World"));
        if (auto Button = SaveWorldButton.lock())
            Button->SetWidgetEventFunc(EWidgetEventState::Clicked, this, &CEditorMenuBar::OnSaveSceneClicked);
        mSceneSubmenuButtons.push_back(SaveWorldButton);

        auto LoadWorldButton = MakeSubmenuButton(WorldSubmenu.get(), "LoadSceneBtn", 1, TEXT("Load World"));
        if (auto Button = LoadWorldButton.lock())
            Button->SetWidgetEventFunc(EWidgetEventState::Clicked, this, &CEditorMenuBar::OnLoadSceneClicked);
        mSceneSubmenuButtons.push_back(LoadWorldButton);
    }

    // ---- "Prefab" 메뉴 (x=410) ----
    mPrefabButton = MakeMenuButton(this, "PrefabBtn", 410.f, 100.f, TEXT("Prefab"));
    if (auto Button = mPrefabButton.lock())
        Button->SetWidgetEventFunc(EWidgetEventState::Hovered, this, &CEditorMenuBar::OnPrefabHovered);

    // Prefab 서브메뉴
    mPrefabSubmenu = CreateWidget<CWidgetContainer>("PrefabSubmenu", 2);
    auto PrefabSubmenu = mPrefabSubmenu.lock();
    if (PrefabSubmenu)
    {
        PrefabSubmenu->SetPos(410.f, BAR_H);
        PrefabSubmenu->SetSize(SUBMENU_W, SUBMENU_BTN_H * 2.f);
        PrefabSubmenu->SetEnable(false);

        // 0번 행: 프리팹 저장
        auto SavePrefabButton = MakeSubmenuButton(PrefabSubmenu.get(), "SavePrefabBtn", 0, TEXT("Save Prefab"));
        if (auto Button = SavePrefabButton.lock())
            Button->SetWidgetEventFunc(EWidgetEventState::Clicked, this, &CEditorMenuBar::OnSavePrefabClicked);
        mPrefabSubmenuButtons.push_back(SavePrefabButton);

        // 1번 행: 파일 탐색기로 프리팹 불러오기/스폰
        auto LoadPrefabButton = MakeSubmenuButton(PrefabSubmenu.get(), "LoadPrefabBtn", 1, TEXT("Load Prefab..."));
        if (auto Button = LoadPrefabButton.lock())
            Button->SetWidgetEventFunc(EWidgetEventState::Clicked, this, &CEditorMenuBar::OnLoadPrefabClicked);
        mPrefabSubmenuButtons.push_back(LoadPrefabButton);
    }

    // ---- "Material" 메뉴 (x=520) ----
    mMaterialButton = MakeMenuButton(this, "MaterialBtn", 520.f, 100.f, TEXT("Material"));
    if (auto Button = mMaterialButton.lock())
        Button->SetWidgetEventFunc(EWidgetEventState::Hovered, this, &CEditorMenuBar::OnMaterialHovered);

    mMaterialSubmenu = CreateWidget<CWidgetContainer>("MaterialSubmenu", 2);
    auto MaterialSubmenu = mMaterialSubmenu.lock();
    if (MaterialSubmenu)
    {
        MaterialSubmenu->SetPos(520.f, BAR_H);
        MaterialSubmenu->SetSize(SUBMENU_W, SUBMENU_BTN_H * 1.f);
        MaterialSubmenu->SetEnable(false);

        auto NewMaterialButton = MakeSubmenuButton(MaterialSubmenu.get(), "NewMaterialBtn", 0, TEXT("New Material Editor..."));
        if (auto Button = NewMaterialButton.lock())
            Button->SetWidgetEventFunc(EWidgetEventState::Clicked, this, &CEditorMenuBar::OnNewMaterialClicked);
    }

    // ---- "Anim Editor" 버튼 (x=630) ----
    mAnimEditorButton = MakeMenuButton(this, "AnimEditorBtn", 630.f, 100.f, TEXT("Anim Editor"));
    if (auto Button = mAnimEditorButton.lock())
        Button->SetWidgetEventFunc(EWidgetEventState::Clicked, this, &CEditorMenuBar::OnAnimEditorClicked);

    return true;
}

// ---- 업데이트 ----

// Hovered, Clicked, Release 세 상태 모두 "활성" 으로 판단
static bool IsActive(const std::weak_ptr<CButton>& ButtonWeak)
{
    auto Button = ButtonWeak.lock();
    if (!Button) return false;
    auto State = Button->GetWidgetState();
    return State == EWidgetState::Hovered
        || State == EWidgetState::Clicked
        || State == EWidgetState::Release;
}

static constexpr float CLOSE_DELAY = 0.08f; // 허용 갭 시간 (초)

void CEditorMenuBar::Update(float DeltaTime)
{
    CWidgetContainer::Update(DeltaTime);

    // ── 서브메뉴 영역 밖 클릭 시 즉시 닫기  ──────────────
    if (mSubmenuOpen || mSceneSubmenuOpen || mPrefabSubmenuOpen || mMaterialSubmenuOpen)
    {
        if (auto World = mWorld.lock())
        {
            if (auto Input = World->GetInput().lock())
            {
                if (Input->GetMouseState(EMouseType::LButton, EInputType::Press))
                {
                    FVector2 MousePos = Input->GetMousePos();

                    // 메뉴바 자체 영역 (Y < BAR_H)
                    bool InMenuBar = (MousePos.y >= 0.f && MousePos.y < BAR_H);

                    // 열린 서브메뉴 영역
                    bool InAnySub = false;
                    auto CheckSub = [&](const std::weak_ptr<CWidgetContainer>& SubmenuWeak, float LocalX)
                    {
                        auto Submenu = SubmenuWeak.lock();
                        if (!Submenu || !Submenu->IsEnable()) return;
                        FVector3 SubmenuSize = Submenu->GetSize();
                        if (MousePos.x >= LocalX && MousePos.x < LocalX + SubmenuSize.x &&
                            MousePos.y >= BAR_H  && MousePos.y < BAR_H  + SubmenuSize.y)
                            InAnySub = true;
                    };
                    CheckSub(mSubmenu,         140.f);
                    CheckSub(mSceneSubmenu,    300.f);
                    CheckSub(mPrefabSubmenu,   410.f);
                    CheckSub(mMaterialSubmenu, 520.f);

                    if (!InMenuBar && !InAnySub)
                    {
                        // 모든 서브메뉴 즉시 닫기
                        auto ForceClose = [](bool& Open, float& Timer,
                                             const std::weak_ptr<CWidgetContainer>& SubmenuWeak)
                        {
                            Open = false; Timer = 0.f;
                            if (auto Submenu = SubmenuWeak.lock()) Submenu->SetEnable(false);
                        };
                        ForceClose(mSubmenuOpen,        mSubmenuCloseTimer, mSubmenu);
                        ForceClose(mSceneSubmenuOpen,   mSceneCloseTimer,   mSceneSubmenu);
                        ForceClose(mPrefabSubmenuOpen,  mPrefabCloseTimer,  mPrefabSubmenu);
                        ForceClose(mMaterialSubmenuOpen,mMaterialCloseTimer,mMaterialSubmenu);
                    }
                }
            }
        }
    }

    // ── Add Component 서브메뉴 닫기 ─────────────────────────────────────────
    if (mSubmenuOpen)
    {
        bool AnyActive = IsActive(mAddComponentButton);
        if (!AnyActive)
            for (auto& ButtonWeak : mComponentButtons)
                if (IsActive(ButtonWeak)) { AnyActive = true; break; }

        if (AnyActive)
        {
            mSubmenuCloseTimer = 0.f;
        }
        else
        {
            mSubmenuCloseTimer += DeltaTime;
            if (mSubmenuCloseTimer >= CLOSE_DELAY)
            {
                mSubmenuOpen = false;
                mSubmenuCloseTimer = 0.f;
                if (auto Submenu = mSubmenu.lock()) Submenu->SetEnable(false);
            }
        }
    }

    // ── World 서브메뉴 닫기 ──────────────────────────────────────────────────
    if (mSceneSubmenuOpen)
    {
        bool AnyActive = IsActive(mSceneButton);
        if (!AnyActive)
            for (auto& ButtonWeak : mSceneSubmenuButtons)
                if (IsActive(ButtonWeak)) { AnyActive = true; break; }

        if (AnyActive)
        {
            mSceneCloseTimer = 0.f;
        }
        else
        {
            mSceneCloseTimer += DeltaTime;
            if (mSceneCloseTimer >= CLOSE_DELAY)
            {
                mSceneSubmenuOpen = false;
                mSceneCloseTimer  = 0.f;
                if (auto Submenu = mSceneSubmenu.lock()) Submenu->SetEnable(false);
            }
        }
    }

    // ── Prefab 서브메뉴 닫기 ────────────────────────────────────────────────
    if (mPrefabSubmenuOpen)
    {
        bool AnyActive = IsActive(mPrefabButton);
        if (!AnyActive)
            for (auto& ButtonWeak : mPrefabSubmenuButtons)
                if (IsActive(ButtonWeak)) { AnyActive = true; break; }

        if (AnyActive)
        {
            mPrefabCloseTimer = 0.f;
        }
        else
        {
            mPrefabCloseTimer += DeltaTime;
            if (mPrefabCloseTimer >= CLOSE_DELAY)
            {
                mPrefabSubmenuOpen = false;
                mPrefabCloseTimer  = 0.f;
                if (auto Submenu = mPrefabSubmenu.lock()) Submenu->SetEnable(false);
            }
        }
    }

    // ── Material 서브메뉴 닫기 ──────────────────────────────────────────────
    if (mMaterialSubmenuOpen)
    {
        bool AnyActive = IsActive(mMaterialButton);
        if (!AnyActive)
        {
            if (auto Submenu = mMaterialSubmenu.lock())
                if (IsActive(Submenu->FindWidget<CButton>("NewMaterialBtn")))
                    AnyActive = true;
        }

        if (AnyActive)
        {
            mMaterialCloseTimer = 0.f;
        }
        else
        {
            mMaterialCloseTimer += DeltaTime;
            if (mMaterialCloseTimer >= CLOSE_DELAY)
            {
                mMaterialSubmenuOpen = false;
                mMaterialCloseTimer  = 0.f;
                if (auto Submenu = mMaterialSubmenu.lock()) Submenu->SetEnable(false);
            }
        }
    }
}

CEditorMenuBar* CEditorMenuBar::Clone()
{
    return new CEditorMenuBar(*this);
}

// ---- 내부 헬퍼 ----

void CEditorMenuBar::TrackComponent(const std::string& TypeName,
                                     const std::string& CompName,
                                     const std::string& Parent)
{
    mTrackedComponents.push_back({ TypeName, CompName, Parent });
    RefreshInspector();
}

void CEditorMenuBar::RefreshInspector()
{
    if (mOnActorCreated)
        mOnActorCreated(mSelectedActor);
}

void CEditorMenuBar::UntrackComponent(const std::string& CompName)
{
    for (auto It = mTrackedComponents.begin(); It != mTrackedComponents.end(); ++It)
    {
        if (It->Name != CompName)
            continue;

        mTrackedComponents.erase(It);
        LOG_DEBUG("[MenuBar] Untracked component: %s", CompName.c_str());
        return;
    }
}

// ---- 컴포넌트 추가 콜백 ----

void CEditorMenuBar::OnEmptyActorClicked()
{
    auto World = mWorld.lock();
    if (!World) return;

    std::string Name = "EmptyActor_" + std::to_string(mActorCount++);
    auto NewActor = World->CreateActor<CActor>(Name);
    mSelectedActor = NewActor;
    mTrackedComponents.clear();

    if (mOnActorCreated)
        mOnActorCreated(mSelectedActor);

    LOG_DEBUG("[MenuBar] Created %s", Name.c_str());
}

void CEditorMenuBar::OnAddComponentHovered()
{
    if (mSubmenuOpen) return;
    mSubmenuOpen = true;
    if (auto Submenu = mSubmenu.lock()) Submenu->SetEnable(true);
}

void CEditorMenuBar::OnMeshComponentClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CMeshComponent>("MeshComponent");
    TrackComponent("MeshComponent", "MeshComponent");
    if (mOnActorCreated) mOnActorCreated(mSelectedActor);
    LOG_DEBUG("[MenuBar] Added MeshComponent");
}

void CEditorMenuBar::OnCameraComponentClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CCameraComponent>("CameraComponent");
    TrackComponent("CameraComponent", "CameraComponent");
    if (mOnActorCreated) mOnActorCreated(mSelectedActor);
    LOG_DEBUG("[MenuBar] Added CameraComponent");
}

void CEditorMenuBar::OnAnimation2DComponentClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CAnimation2DComponent>("Animation2DComponent");
    TrackComponent("Animation2DComponent", "Animation2DComponent");
    if (mOnActorCreated) mOnActorCreated(mSelectedActor);
    LOG_DEBUG("[MenuBar] Added Animation2DComponent");
}

void CEditorMenuBar::OnColliderBox2DClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CColliderBox2D>("ColliderBox2D");
    TrackComponent("ColliderBox2D", "ColliderBox2D");
    if (mOnActorCreated) mOnActorCreated(mSelectedActor);
    LOG_DEBUG("[MenuBar] Added ColliderBox2D");
}

void CEditorMenuBar::OnColliderSphere2DClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CColliderSphere2D>("ColliderSphere2D");
    TrackComponent("ColliderSphere2D", "ColliderSphere2D");
    if (mOnActorCreated) mOnActorCreated(mSelectedActor);
    LOG_DEBUG("[MenuBar] Added ColliderSphere2D");
}

void CEditorMenuBar::OnCharacterMovementClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CCharacterMovementComponent>("CharacterMovement");
    TrackComponent("CharacterMovement", "CharacterMovement");
    if (mOnActorCreated) mOnActorCreated(mSelectedActor);
    LOG_DEBUG("[MenuBar] Added CharacterMovementComponent");
}

void CEditorMenuBar::OnProjectileMovementClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CProjectileMovementComponent>("ProjectileMovement");
    TrackComponent("ProjectileMovement", "ProjectileMovement");
    if (mOnActorCreated) mOnActorCreated(mSelectedActor);
    LOG_DEBUG("[MenuBar] Added ProjectileMovementComponent");
}

void CEditorMenuBar::OnAIComponentClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CAIComponent>("AIComponent");
    TrackComponent("AIComponent", "AIComponent");
    if (mOnActorCreated) mOnActorCreated(mSelectedActor);
    LOG_DEBUG("[MenuBar] Added AIComponent");
}

void CEditorMenuBar::OnSoundComponentClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CSoundComponent>("SoundComponent");
    TrackComponent("SoundComponent", "SoundComponent");
    if (mOnActorCreated) mOnActorCreated(mSelectedActor);
    LOG_DEBUG("[MenuBar] Added SoundComponent");
}

void CEditorMenuBar::OnWidgetComponentClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CWidgetComponent>("WidgetComponent");
    TrackComponent("WidgetComponent", "WidgetComponent");
    if (mOnActorCreated) mOnActorCreated(mSelectedActor);
    LOG_DEBUG("[MenuBar] Added WidgetComponent");
}

void CEditorMenuBar::OnTileMapComponentClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CTileMapComponent>("TileMapComponent");
    TrackComponent("TileMapComponent", "TileMapComponent");
    if (mOnActorCreated) mOnActorCreated(mSelectedActor);
    LOG_DEBUG("[MenuBar] Added TileMapComponent");
}

void CEditorMenuBar::OnDirectionInputComponentClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CDirectionInputComponent>("DirectionInput");
    TrackComponent("DirectionInput", "DirectionInput");
    if (mOnActorCreated) mOnActorCreated(mSelectedActor);
    LOG_DEBUG("[MenuBar] Added DirectionInputComponent");
}

void CEditorMenuBar::OnActionStateComponentClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CActionStateComponent>("ActionState");
    TrackComponent("ActionState", "ActionState");
    if (mOnActorCreated) mOnActorCreated(mSelectedActor);
    LOG_DEBUG("[MenuBar] Added ActionStateComponent");
}

void CEditorMenuBar::OnHeightComponentClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) return;

    Actor->CreateComponent<CHeightComponent>("Height");
    TrackComponent("Height", "Height");

    LOG_DEBUG("[MenuBar] Added HeightComponent");
}

// ---- 프리팹 콜백 ----

void CEditorMenuBar::OnPrefabHovered()
{
    if (mPrefabSubmenuOpen) return;
    mPrefabSubmenuOpen = true;
    if (auto Submenu = mPrefabSubmenu.lock()) Submenu->SetEnable(true);
}

void CEditorMenuBar::OnSavePrefabClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }

    // 액터를 직접 훑어서 구성을 만든다.
    // mTrackedComponents는 이 세션에서 메뉴바로 추가한 것만 담고 있어서,
    // 월드에서 불러왔거나 프리팹으로 스폰한 액터를 저장하면 비어 있었다.
    FPrefabData Data;
    CPrefabManager::BuildPrefabData(Actor, Data);

    if (Data.Components.empty())
    {
        LOG_DEBUG("[MenuBar] Actor has no components to save.");
        return;
    }

    // 파일명을 직접 정하게 한다.
    // 예전에는 "Prefab_" + 카운터로 자동 생성했는데, 카운터가 실행할 때마다 0으로
    // 돌아가서 다시 켜고 저장하면 Prefab_0을 말없이 덮어썼다.
    std::string Dir = DialogUtil::GetExeDir() + "Asset\\Prefab\\";
    CreateDirectoryA((DialogUtil::GetExeDir() + "Asset\\").c_str(), nullptr);
    CreateDirectoryA(Dir.c_str(), nullptr);

    std::string Path = DialogUtil::SaveFile(
        "Prefab Files\0*.prefab\0All Files\0*.*\0", Dir.c_str(), "prefab");
    if (Path.empty()) return;

    // 프리팹은 항상 Asset\Prefab\ 아래에서 관리되므로 파일명만 취한다.
    // (SpawnPrefab / GetPrefabNames가 그 폴더를 기준으로 동작한다)
    std::string PrefabName = DialogUtil::ExtractBaseName(Path);
    if (PrefabName.empty()) return;

    // Actor를 같이 넘겨야 각 컴포넌트의 내부 상태(애니메이션 시퀀스 등)까지 저장된다.
    CPrefabManager::GetInst()->SavePrefab(PrefabName, Data, Actor);
}

void CEditorMenuBar::OnLoadPrefabClicked()
{
    std::string PrefabDir = DialogUtil::GetExeDir() + "Asset\\Prefab\\";
    std::string FullPath = DialogUtil::OpenFile(
        "Prefab Files\0*.prefab\0All Files\0*.*\0", PrefabDir.c_str());
    if (FullPath.empty()) return;

    auto World = mWorld.lock();
    if (!World) return;

    // 고른 파일의 전체 경로로 연다.
    // 이름만 넘기면 Asset\Prefab\ 안에서 다시 찾기 때문에,
    // 다이얼로그로 다른 폴더의 파일을 고르면 "not found"로 조용히 실패했다.
    auto Spawned = CPrefabManager::GetInst()->SpawnPrefabFromFile(FullPath, World);

    if (Spawned.expired())
    {
        LOG_ERROR("[MenuBar] Prefab spawn failed: %s", FullPath.c_str());
        return;
    }

    // 스폰한 액터를 바로 선택 상태로 만들어 인스펙터/애님 에디터가 따라오게 한다.
    mSelectedActor = Spawned;
    if (mOnActorCreated)
        mOnActorCreated(mSelectedActor);

    LOG_DEBUG("[MenuBar] Prefab spawned from file: %s", FullPath.c_str());
}

// ---- 월드 저장/불러오기 콜백 ----

void CEditorMenuBar::OnSceneHovered()
{
    if (mSceneSubmenuOpen) return;
    mSceneSubmenuOpen = true;
    if (auto Submenu = mSceneSubmenu.lock()) Submenu->SetEnable(true);
}

void CEditorMenuBar::OnSaveSceneClicked()
{
    auto World = mWorld.lock();
    if (!World) return;

    std::string WorldDir = DialogUtil::GetExeDir() + "Asset\\World\\";
    CreateDirectoryA((DialogUtil::GetExeDir() + "Asset\\").c_str(), nullptr);
    CreateDirectoryA(WorldDir.c_str(), nullptr);

    std::string FilePath = DialogUtil::SaveFile(
        "World Files\0*.world\0All Files\0*.*\0", WorldDir.c_str(), "world");
    if (FilePath.empty()) return;

    World->SaveScene(FilePath);
    LOG_DEBUG("[MenuBar] World saved: %s", FilePath.c_str());
}

void CEditorMenuBar::OnLoadSceneClicked()
{
    auto World = mWorld.lock();
    if (!World) return;

    std::string WorldDir = DialogUtil::GetExeDir() + "Asset\\World\\";
    std::string FilePath = DialogUtil::OpenFile(
        "World Files\0*.world\0All Files\0*.*\0", WorldDir.c_str());
    if (FilePath.empty()) return;

    if (World->LoadScene(FilePath))
        LOG_DEBUG("[MenuBar] World loaded: %s", FilePath.c_str());
    else
        LOG_ERROR("[MenuBar] World load failed: %s", FilePath.c_str());
}

// ---- Material 콜백 ----

void CEditorMenuBar::OnMaterialHovered()
{
    if (auto Submenu = mSubmenu.lock())         Submenu->SetEnable(false); mSubmenuOpen = false;
    if (auto Submenu = mSceneSubmenu.lock())    Submenu->SetEnable(false); mSceneSubmenuOpen = false;
    if (auto Submenu = mPrefabSubmenu.lock())   Submenu->SetEnable(false); mPrefabSubmenuOpen = false;

    mMaterialSubmenuOpen = true;
    if (auto Submenu = mMaterialSubmenu.lock()) Submenu->SetEnable(true);
}

void CEditorMenuBar::OnNewMaterialClicked()
{
    if (auto Submenu = mMaterialSubmenu.lock()) Submenu->SetEnable(false);
    mMaterialSubmenuOpen = false;

    if (mOnOpenMaterialEditor)
        mOnOpenMaterialEditor();
}

// ---- Anim Editor 콜백 ----

void CEditorMenuBar::OnAnimEditorClicked()
{
    if (mOnOpenAnimEditor)
        mOnOpenAnimEditor();
}
