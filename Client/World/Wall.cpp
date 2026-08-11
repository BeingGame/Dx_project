#include "Wall.h"
#include "World/MeshComponent.h"
#include "World/ColliderBox2D.h"

CWall::CWall()
{}

CWall::CWall(const CWall & src)
{}

CWall::CWall(CWall && src) noexcept
{}

CWall::~CWall()
{}

bool CWall::Init()
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
		Box->SetCollisionProfile("BlockAll");
	}
    return true;
}
