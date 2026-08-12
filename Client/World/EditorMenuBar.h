#pragma once

#include "World/WidgetContainer.h"
#include "PrefabManager.h"

class CEditorMenuBar : public CWidgetContainer
{
public:
    CEditorMenuBar();
    CEditorMenuBar(const CEditorMenuBar& src);
    virtual ~CEditorMenuBar();

protected:
    // ---- Add Component menu ----
    std::weak_ptr<class CButton> mEmptyActorButton;
    std::weak_ptr<class CButton> mAddComponentButton;
    std::weak_ptr<class CWidgetContainer> mSubmenu;
    std::vector<std::weak_ptr<class CButton>> mComponentButtons;

    bool mSubmenuOpen = false;
    int  mActorCount  = 0;

    // ---- Prefab menu ----
    std::weak_ptr<class CButton>          mPrefabButton;
    std::weak_ptr<class CWidgetContainer> mPrefabSubmenu;
    std::vector<std::weak_ptr<class CButton>> mPrefabSubmenuButtons;

    bool mPrefabSubmenuOpen = false;
    int  mPrefabSaveCount   = 0;

    // ---- Scene menu ----
    std::weak_ptr<class CButton>          mSceneButton;
    std::weak_ptr<class CWidgetContainer> mSceneSubmenu;
    std::vector<std::weak_ptr<class CButton>> mSceneSubmenuButtons;
    bool mSceneSubmenuOpen = false;
    int  mSceneSaveCount   = 0;

    // 현재 액터에 UI로 추가한 컴포넌트 추적
    std::vector<FPrefabComponentEntry> mTrackedComponents;

    // Spawn 슬롯 (버튼 6개 고정, 내용은 OnPrefabHovered 시 갱신)
    static constexpr int MAX_PREFAB_SLOTS = 6;
    struct FPrefabSlot
    {
        std::weak_ptr<class CButton>    Btn;
        std::weak_ptr<class CTextBlock> Label;
        std::string                     PrefabName;
    };
    FPrefabSlot mPrefabSlots[MAX_PREFAB_SLOTS];

    std::weak_ptr<class CActor> mSelectedActor;

public:
    virtual bool Init();
    virtual void Update(float DeltaTime);
    virtual CEditorMenuBar* Clone();

public:
    // Add Component 콜백
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

    void OnSpawnPrefab0();
    void OnSpawnPrefab1();
    void OnSpawnPrefab2();
    void OnSpawnPrefab3();
    void OnSpawnPrefab4();
    void OnSpawnPrefab5();

    // Scene 콜백
    void OnSceneHovered();
    void OnSaveSceneClicked();
    void OnLoadSceneClicked();

private:
    void TrackComponent(const std::string& TypeName, const std::string& CompName, const std::string& Parent = "");
    void ExecuteSpawnSlot(int Idx);
};
