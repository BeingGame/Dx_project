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
#include "Component/AnimStateComponent.h"
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

CEditorMenuBar::CEditorMenuBar()
{}

CEditorMenuBar::CEditorMenuBar(const CEditorMenuBar& src)
    : CWidgetContainer(src)
{}

CEditorMenuBar::~CEditorMenuBar()
{}

// ---- 헬퍼 함수 ----

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

// ---- 초기화 ----

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

    // 빈 액터 생성 버튼 (x=10)
    mEmptyActorButton = MakeMenuButton(this, "EmptyActorBtn", 10.f, 120.f, TEXT("Empty Actor"));
    if (auto B = mEmptyActorButton.lock())
        B->SetWidgetEventFunc(EWidgetEventState::Clicked, this, &CEditorMenuBar::OnEmptyActorClicked);

    // 컴포넌트 추가 버튼 (x=140)
    mAddComponentButton = MakeMenuButton(this, "AddComponentBtn", 140.f, 150.f, TEXT("Add Component"));
    if (auto B = mAddComponentButton.lock())
        B->SetWidgetEventFunc(EWidgetEventState::Hovered, this, &CEditorMenuBar::OnAddComponentHovered);

    // 컴포넌트 추가 서브메뉴
    mSubmenu = CreateWidget<CWidgetContainer>("Submenu", 2);
    auto Submenu = mSubmenu.lock();
    if (Submenu)
    {
        Submenu->SetPos(140.f, BAR_H);
        Submenu->SetSize(SUBMENU_W, SUBMENU_BTN_H * 13.f);
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
        AddCompBtn("DirectionInput",       11, TEXT("Direction Input"),        &CEditorMenuBar::OnDirectionInputComponentClicked);
        AddCompBtn("AnimState",            12, TEXT("Anim State"),             &CEditorMenuBar::OnAnimStateComponentClicked);
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
        PrefabSub->SetSize(SUBMENU_W, SUBMENU_BTN_H * 2.f);
        PrefabSub->SetEnable(false);

        // 0번 행: 프리팹 저장
        auto SaveBtn = MakeSubmenuButton(PrefabSub.get(), "SavePrefabBtn", 0, TEXT("Save Prefab"));
        if (auto B = SaveBtn.lock())
            B->SetWidgetEventFunc(EWidgetEventState::Clicked, this, &CEditorMenuBar::OnSavePrefabClicked);
        mPrefabSubmenuButtons.push_back(SaveBtn);

        // 1번 행: 파일 탐색기로 프리팹 불러오기/스폰
        auto LoadPrefabBtn = MakeSubmenuButton(PrefabSub.get(), "LoadPrefabBtn", 1, TEXT("Load Prefab..."));
        if (auto B = LoadPrefabBtn.lock())
            B->SetWidgetEventFunc(EWidgetEventState::Clicked, this, &CEditorMenuBar::OnLoadPrefabClicked);
        mPrefabSubmenuButtons.push_back(LoadPrefabBtn);
    }

    // ---- "Material" 메뉴 (x=520) ----
    mMaterialButton = MakeMenuButton(this, "MaterialBtn", 520.f, 100.f, TEXT("Material"));
    if (auto B = mMaterialButton.lock())
        B->SetWidgetEventFunc(EWidgetEventState::Hovered, this, &CEditorMenuBar::OnMaterialHovered);

    mMaterialSubmenu = CreateWidget<CWidgetContainer>("MaterialSubmenu", 2);
    auto MatSub = mMaterialSubmenu.lock();
    if (MatSub)
    {
        MatSub->SetPos(520.f, BAR_H);
        MatSub->SetSize(SUBMENU_W, SUBMENU_BTN_H * 1.f);
        MatSub->SetEnable(false);

        auto NewMatBtn = MakeSubmenuButton(MatSub.get(), "NewMaterialBtn", 0, TEXT("New Material Editor..."));
        if (auto B = NewMatBtn.lock())
            B->SetWidgetEventFunc(EWidgetEventState::Clicked, this, &CEditorMenuBar::OnNewMaterialClicked);
    }

    // ---- "Anim Editor" 버튼 (x=630) ----
    mAnimEditorButton = MakeMenuButton(this, "AnimEditorBtn", 630.f, 100.f, TEXT("Anim Editor"));
    if (auto B = mAnimEditorButton.lock())
        B->SetWidgetEventFunc(EWidgetEventState::Clicked, this, &CEditorMenuBar::OnAnimEditorClicked);

    return true;
}

// ---- 업데이트 ----

// Hovered, Clicked, Release 세 상태 모두 "활성" 으로 판단
static bool IsActive(const std::weak_ptr<CButton>& W)
{
    auto B = W.lock();
    if (!B) return false;
    auto S = B->GetWidgetState();
    return S == EWidgetState::Hovered
        || S == EWidgetState::Clicked
        || S == EWidgetState::Release;
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
                    FVector2 MP = Input->GetMousePos();

                    // 메뉴바 자체 영역 (Y < BAR_H)
                    bool InMenuBar = (MP.y >= 0.f && MP.y < BAR_H);

                    // 열린 서브메뉴 영역
                    bool InAnySub = false;
                    auto CheckSub = [&](const std::weak_ptr<CWidgetContainer>& SubW, float LocalX)
                    {
                        auto Sub = SubW.lock();
                        if (!Sub || !Sub->IsEnable()) return;
                        FVector3 SubSize = Sub->GetSize();
                        if (MP.x >= LocalX && MP.x < LocalX + SubSize.x &&
                            MP.y >= BAR_H  && MP.y < BAR_H  + SubSize.y)
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
                                             const std::weak_ptr<CWidgetContainer>& SubW)
                        {
                            Open = false; Timer = 0.f;
                            if (auto S = SubW.lock()) S->SetEnable(false);
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
            for (auto& W : mComponentButtons)
                if (IsActive(W)) { AnyActive = true; break; }

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
                if (auto S = mSubmenu.lock()) S->SetEnable(false);
            }
        }
    }

    // ── World 서브메뉴 닫기 ──────────────────────────────────────────────────
    if (mSceneSubmenuOpen)
    {
        bool AnyActive = IsActive(mSceneButton);
        if (!AnyActive)
            for (auto& W : mSceneSubmenuButtons)
                if (IsActive(W)) { AnyActive = true; break; }

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
                if (auto S = mSceneSubmenu.lock()) S->SetEnable(false);
            }
        }
    }

    // ── Prefab 서브메뉴 닫기 ────────────────────────────────────────────────
    if (mPrefabSubmenuOpen)
    {
        bool AnyActive = IsActive(mPrefabButton);
        if (!AnyActive)
            for (auto& W : mPrefabSubmenuButtons)
                if (IsActive(W)) { AnyActive = true; break; }

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
                if (auto S = mPrefabSubmenu.lock()) S->SetEnable(false);
            }
        }
    }

    // ── Material 서브메뉴 닫기 ──────────────────────────────────────────────
    if (mMaterialSubmenuOpen)
    {
        bool AnyActive = IsActive(mMaterialButton);
        if (!AnyActive)
        {
            if (auto Sub = mMaterialSubmenu.lock())
                if (IsActive(Sub->FindWidget<CButton>("NewMaterialBtn")))
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
                if (auto S = mMaterialSubmenu.lock()) S->SetEnable(false);
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
    if (auto S = mSubmenu.lock()) S->SetEnable(true);
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

void CEditorMenuBar::OnAnimStateComponentClicked()
{
    auto Actor = mSelectedActor.lock();
    if (!Actor) { LOG_DEBUG("[MenuBar] No actor selected."); return; }
    Actor->CreateComponent<CAnimStateComponent>("AnimState");
    TrackComponent("AnimState", "AnimState");
    if (mOnActorCreated) mOnActorCreated(mSelectedActor);
    LOG_DEBUG("[MenuBar] Added AnimStateComponent");
}

// ---- 프리팹 콜백 ----

void CEditorMenuBar::OnPrefabHovered()
{
    if (mPrefabSubmenuOpen) return;
    mPrefabSubmenuOpen = true;
    if (auto S = mPrefabSubmenu.lock()) S->SetEnable(true);
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
    if (auto S = mSceneSubmenu.lock()) S->SetEnable(true);
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
    if (auto S = mSubmenu.lock())         S->SetEnable(false); mSubmenuOpen = false;
    if (auto S = mSceneSubmenu.lock())    S->SetEnable(false); mSceneSubmenuOpen = false;
    if (auto S = mPrefabSubmenu.lock())   S->SetEnable(false); mPrefabSubmenuOpen = false;

    mMaterialSubmenuOpen = true;
    if (auto S = mMaterialSubmenu.lock()) S->SetEnable(true);
}

void CEditorMenuBar::OnNewMaterialClicked()
{
    if (auto S = mMaterialSubmenu.lock()) S->SetEnable(false);
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
