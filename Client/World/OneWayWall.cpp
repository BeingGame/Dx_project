#include "OneWayWall.h"
#include "World/MeshComponent.h"
#include "World/ColliderBox2D.h"

COneWayWall::COneWayWall()
{}

COneWayWall::COneWayWall(const COneWayWall& src)
{}

COneWayWall::COneWayWall(COneWayWall&& src) noexcept
{}

COneWayWall::~COneWayWall()
{}

bool COneWayWall::Init()
{
	CActor::Init();

	mMeshComp = CreateComponent<CMeshComponent>("Root");
	mBox = CreateComponent<CColliderBox2D>("Box");

	auto Mesh = mMeshComp.lock();

	if (Mesh)
	{
		Mesh->SetMesh("TexRect");
		Mesh->SetShader("Material");
		Mesh->AddTexture(0, "OneWayWall", TEXT("block_bar.png"));
		Mesh->SetRelativeScale(40.f, 400.f);
	}

	auto Box = mBox.lock();

	if (Box)
	{
		Box->SetInheritScale(false);
		Box->SetBoxSize(50.f, 400.f);
		Box->SetDebugDraw(true);
		Box->SetCollisionProfile("OverlapAll");// Overlap상태로 이벤트처리
		Box->SetBeginOverlapFunc(this, &COneWayWall::OnBeginOverlap);
	}

	return true;
}

void COneWayWall::OnBeginOverlap(const FVector3& HitPoint, const FVector3& Normal, std::weak_ptr<class CCollider> Collider)
{
	auto _Collider = Collider.lock();

	if (!_Collider)
	{
		return;
	}

	auto Actor = _Collider->GetOwner().lock();
	
	if (!Actor)
	{
		return;
	}

	//액터의 이동방향을 알아와야한다.
	//벽에 왼쪽에 있는 액터인지 오른쪽에 있는 액터인지 체크해준다.
	//벽에 오른쪽에 있다면 움직인만큼 뒤로 다시 움직이게 한다.
	FVector3 WallPos = GetWorldPos();

	//현재 오른쪽움직이고 있는지 체크한다.
	bool IsMoveRight = (Actor->GetWorldPos().x < WallPos.x);

	if (!IsMoveRight)
	{
		Actor->AddWorldPos(Normal);
	}

}
