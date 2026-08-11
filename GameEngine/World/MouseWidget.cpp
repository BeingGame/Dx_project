#include "MouseWidget.h"

#include "../Asset/AssetManager.h"
#include "../Asset/TextureManager.h"
#include "../Asset/Texture.h"

#include "WorldManager.h"
#include "Input.h"

CMouseWidget::CMouseWidget()
{}

CMouseWidget::CMouseWidget(const CMouseWidget& src)
	:CImage(src)
{
}

CMouseWidget::~CMouseWidget()
{}

void CMouseWidget::Update(float DeltaTime)
{
	CImage::Update(DeltaTime);

	if (mWorld.expired())
	{
		mWorld = CWorldManager::GetInst()->GetWorld();
	}

	auto World = mWorld.lock();

	if (World)
	{
		auto Input = World->GetInput().lock();

		if (Input)
		{
			FVector2 Pos = Input->GetMousePos();

			SetPos(Pos.x, Pos.y);
		}
	}
}

CMouseWidget* CMouseWidget::Clone()
{
	return new CMouseWidget(*this);
}