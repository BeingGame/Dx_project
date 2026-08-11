#include "WidgetComponent.h"

#include "CameraManager.h"
#include "../Device.h"

CWidgetComponent::CWidgetComponent()
{}

CWidgetComponent::CWidgetComponent(const CWidgetComponent& src)
	:CSceneComponent(src)
{}

CWidgetComponent::CWidgetComponent(CWidgetComponent&& src) noexcept
	:CSceneComponent(std::move(src))
{}

CWidgetComponent::~CWidgetComponent()
{}

bool CWidgetComponent::Init()
{
	CSceneComponent::Init();

	return true;
}

void CWidgetComponent::PostUpdate(float DeltaTime)
{
	CSceneComponent::PostUpdate(DeltaTime);

	//위젯컴포넌트가 소유하고있는 위젯의 월드 위치를 이용해서
	//스크린 좌표로 변경해준다.

	if (mWidget)
	{
		//UI의 좌표를 계산하긴 먼저 해상도와 카메라 위치, 그리고 위젯의 크기를 가져와준다.
		FVector3 Size = mWidget->GetSize();
		FVector3 CameraPos = mWorld.lock()->GetCameraManager().lock()->GetMainCameraPos();
		FResolution RS = CDevice::GetInst()->GetResolution();

		//스크린의 좌측상단 월드 좌표
		FVector3 LT;

		//좌측상단 좌표를 구하기위해 먼저 구해준 카메라의 월드위치를 기준으로
		//해상도의 절반만큼 빼준다.
		LT.x = CameraPos.x - RS.Width * 0.5f;
		LT.y = CameraPos.y + RS.Height * 0.5f;

		//좌측상단 좌표를 구한뒤 해당 좌표를 스크린 좌표로 변경해준다.
		FVector3 Pos;

		//컴포넌트 월드좌표와 좌측 상단 좌표를 이용해 연산을 진행한다.
		//스크린 좌표를 기준으로 음수가 없기 때문에 WorldPos.x > LT.x 
		//월드에 좌측의 x좌표를 빼주면 스크린 좌표가 나온다.
		Pos.x = mWorldPos.x - LT.x;
		//Y는 역으로 LT이 더 위에있어 LT에 월드y좌표를 빼준다.
		Pos.y = LT.y - mWorldPos.y;

		//최종적으로 구해진 좌표를 이용해서 위젯의 좌측 상단 좌표를 구해준다.
		Pos.x -= Size.x * 0.5f;
		Pos.y -= Size.y * 0.5f;

		mWidget->SetPos(Pos);
	}
}

CWidgetComponent* CWidgetComponent::Clone() const
{
	return new CWidgetComponent(*this);
}
