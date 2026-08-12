#pragma once

#include "SceneComponent.h"
#include "Widget.h"
#include "World.h"
#include "UIManager.h"

class CWidgetComponent :
	public CSceneComponent
{
public:
	CWidgetComponent();
	CWidgetComponent(const CWidgetComponent& src);
	CWidgetComponent(CWidgetComponent&& src) noexcept;
	virtual ~CWidgetComponent();

protected:
	std::shared_ptr<CWidget> mWidget;

public:
	void SetWidget(const std::shared_ptr<CWidget> Widget)
	{
		mWidget = Widget;
	}

	std::shared_ptr<CWidget> GetWidget() const
	{
		return mWidget;
	}

public:
	virtual bool Init();
	virtual void PostUpdate(float DeltaTime);

public:
	virtual CWidgetComponent* Clone() const;
	virtual std::string GetTypeName() const override { return "CWidgetComponent"; }

public:
	template<typename T>
	std::weak_ptr<T> SetWidget(const std::string& Name)
	{
		mWidget = std::make_shared<T>();

		mWidget->SetWorld(mWorld);
		mWidget->SetName(Name);
		mWidget->SetUIManager(mWorld.lock()->GetUIManager());

		if (!mWidget->Init())
		{
			mWidget.reset();
			return std::weak_ptr<T>();
		}

		//위젯이 성공적으로 초기화 되면 UIManager에 WorldList에 등록해준다.

		auto UIManager = mWorld.lock()->GetUIManager().lock();

		if (UIManager)
		{
			UIManager->AddWorldWidget(mWidget);
		}

		return std::dynamic_pointer_cast<T>(mWidget);
	}
};

