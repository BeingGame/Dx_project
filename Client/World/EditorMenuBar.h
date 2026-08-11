#pragma once

#include "World/WidgetContainer.h"

class CEditorMenuBar : public CWidgetContainer
{
public:
	CEditorMenuBar();
	CEditorMenuBar(const CEditorMenuBar& src);
	virtual ~CEditorMenuBar();

protected:
	std::weak_ptr<class CButton> mEmptyActorButton;
	std::weak_ptr<class CButton> mAddComponentButton;
	std::weak_ptr<class CWidgetContainer> mSubmenu;
	std::vector<std::weak_ptr<class CButton>> mComponentButtons;

	std::weak_ptr<class CActor> mSelectedActor;

	bool mSubmenuOpen = false;
	int mActorCount = 0;

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual CEditorMenuBar* Clone();

public:
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
};
