#pragma once

#include "World/WidgetContainer.h"
#include "PrefabManager.h"
#include <functional>

class CEditorMenuBar : public CWidgetContainer
{
public:
    CEditorMenuBar();
    CEditorMenuBar(const CEditorMenuBar& src);
    virtual ~CEditorMenuBar();

protected:
    // ---- 컴포넌트 추가 메뉴 ----
    std::weak_ptr<class CButton> mEmptyActorButton;
    std::weak_ptr<class CButton> mAddComponentButton;
    std::weak_ptr<class CWidgetContainer> mSubmenu;
    std::vector<std::weak_ptr<class CButton>> mComponentButtons;

    bool  mSubmenuOpen = false;
    float mSubmenuCloseTimer = 0.f;
    int   mActorCount  = 0;

    // ---- 프리팹 메뉴 ----
    std::weak_ptr<class CButton>          mPrefabButton;
    std::weak_ptr<class CWidgetContainer> mPrefabSubmenu;
    std::vector<std::weak_ptr<class CButton>> mPrefabSubmenuButtons;

    bool  mPrefabSubmenuOpen = false;
    float mPrefabCloseTimer  = 0.f;
    int   mPrefabSaveCount   = 0;

    // ---- 씬 메뉴 ----
    std::weak_ptr<class CButton>          mSceneButton;
    std::weak_ptr<class CWidgetContainer> mSceneSubmenu;
    std::vector<std::weak_ptr<class CButton>> mSceneSubmenuButtons;
    bool  mSceneSubmenuOpen  = false;
    float mSceneCloseTimer   = 0.f;
    int   mSceneSaveCount    = 0;

    // ---- 머티리얼 메뉴 ----
    std::weak_ptr<class CButton>          mMaterialButton;
    std::weak_ptr<class CWidgetContainer> mMaterialSubmenu;
    bool  mMaterialSubmenuOpen = false;
    float mMaterialCloseTimer  = 0.f;

    std::function<void()> mOnOpenMaterialEditor;
    std::function<void()> mOnOpenAnimEditor;

    // ---- Anim Editor 버튼 ----
    std::weak_ptr<class CButton> mAnimEditorButton;

    // 현재 액터에 UI로 추가한 컴포넌트 추적
    std::vector<FPrefabComponentEntry> mTrackedComponents;

    std::weak_ptr<class CActor> mSelectedActor;

    std::function<void(std::weak_ptr<class CActor>)> mOnActorCreated;

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual CEditorMenuBar* Clone();

    void SetOnActorCreated(std::function<void(std::weak_ptr<class CActor>)> Fn) { mOnActorCreated = std::move(Fn); }
    void SetSelectedActor(std::weak_ptr<class CActor> Actor) { mSelectedActor = Actor; }

public:
    // 컴포넌트 추가 콜백
    void OnEmptyActorClicked();
    void OnAddComponentHovered();

    void OnMeshComponentClicked();
    void OnCameraComponentClicked();
    void OnAnimation2DComponentClicked();
    void OnColliderBox2DClicked();
    void OnColliderSphere2DClicked();
    void OnCharacterMovementClicked();
    void OnProjectileMovementClicked();
    void OnAIComponentClicked();
    void OnSoundComponentClicked();
    void OnWidgetComponentClicked();
    void OnTileMapComponentClicked();

    // Prefab 콜백
    void OnPrefabHovered();
    void OnSavePrefabClicked();
    void OnLoadPrefabClicked();

    // 씬 콜백
    void OnSceneHovered();
    void OnSaveSceneClicked();
    void OnLoadSceneClicked();

    // Material 콜백
    void OnMaterialHovered();
    void OnNewMaterialClicked();

public:
    void SetOnOpenMaterialEditor(std::function<void()> Fn) { mOnOpenMaterialEditor = std::move(Fn); }
    void SetOnOpenAnimEditor(std::function<void()> Fn) { mOnOpenAnimEditor = std::move(Fn); }
    void OnAnimEditorClicked();

private:
    void TrackComponent(const std::string& TypeName, const std::string& CompName, const std::string& Parent = "");
    void RefreshInspector();
};
