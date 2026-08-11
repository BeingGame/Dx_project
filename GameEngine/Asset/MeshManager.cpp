#include "MeshManager.h"

CMeshManager::CMeshManager()
{}

CMeshManager::~CMeshManager()
{}

bool CMeshManager::Init()
{


	//정점에 대한 정보
	//위치랑 색상을 가진 구조체
	//정점 -1 ~ 1, ex) 정점이 -1~1 사이면 원점은 0,0, 정점이 0~1 사이면 원점은 0.5,0.5
	//메쉬의 재사용성, 메모리의 효율성을 높여주기 위함
	//크기는 Scale 조절(월드행렬)
	//pivot을 0,0 원점으로 설정하기 위해서 0.5로 정점을 설정한다.
	FVertexColor ColorRect[4] =
	{
		FVertexColor(-0.5f,0.5f,0.f,1.f,1.f,0.f,1.f),
		FVertexColor(0.5f,0.5f,0.f,1.f,1.f,0.f,1.f),
		FVertexColor(-0.5f,-0.5f,0.f,1.f,1.f,1.f,1.f),
		FVertexColor(0.5f,-0.5f,0.f,1.f,1.f,0.f,1.f),
	};

	//인덱스의 순서는 시계방향으로 만들어줘야 한다.
	//시계방향의 의미는 정면
	//백페이스 컬링이 후면을 기준으로 컬링된다.
	//렌더링 출력이 안되는 상황을 막기위함
	unsigned short IndexRect[6] = { 0,1,3, 0,3,2 };

	//Primitive를 설정할때 list와 strip 두가지 방식으로 설정할수 있으며
	//list는 정점을 도형에 맞춰 읽는 방식으로 삼각형이라면 정점을 3개씩 읽는다.
	//strip은 연속적으로 정점을 읽는 방식
	//보통은 list를 사용한다.
	if (!CreateMesh("Mesh_ColorRect", ColorRect, sizeof(FVertexColor), 4, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
		, IndexRect, sizeof(unsigned short), 6, DXGI_FORMAT_R16_UINT, D3D11_USAGE_IMMUTABLE))
	{
		return false;
	}

	FVector3 FrameRect[4] =
	{
		FVector3(-0.5f,0.5f,0.f),
		FVector3(0.5f,0.5f,0.f),
		FVector3(-0.5f,-0.5f,0.f),
		FVector3(0.5f,-0.5f,0.f),
	};

	//Strip으로 선을 그어서 만든다.
	unsigned short IndexFrame[5] = { 0,1,3,2,0 };

	//선으로 이루어진 메쉬를 만들어준다.
	if (!CreateMesh("Mesh_FrameRect", FrameRect, sizeof(FVector3), 4, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP
		, IndexFrame, sizeof(unsigned short), 5, DXGI_FORMAT_R16_UINT, D3D11_USAGE_IMMUTABLE))
	{
		return false;
	}

	std::vector<FVector3> FrameSphere2D;

	//원의 중심점
	//반복문으로 원의 정점을 찍어준다.
	for (int i = 0; i < 360; i += 12)
	{
		FVector3 Pos;

		//cosf랑 sinf 사용하기위해 각도를 radian으로 변경해준다.
		float radian = DirectX::XMConvertToRadians((float)i);

		//x랑 y값을 pos로 저장한다음에 원에 저장한다.
		Pos.x = cosf(radian);
		Pos.y = sinf(radian);
		Pos.z = 0.f;

		FrameSphere2D.push_back(Pos);
	}

	//인덱스번호
	std::vector<unsigned short> FrameSphereIndex;

	size_t PosCount = FrameSphere2D.size();

	for (size_t i = 0; i < PosCount; ++i)
	{
		FrameSphereIndex.push_back((unsigned short)i);
	}

	FrameSphereIndex.push_back(0);

	if (!CreateMesh("Mesh_FrameSphere", &FrameSphere2D[0], sizeof(FVector3), (int)PosCount, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP
		, &FrameSphereIndex[0], sizeof(unsigned short), (int)FrameSphereIndex.size(), DXGI_FORMAT_R16_UINT, D3D11_USAGE_IMMUTABLE))
	{
		return false;
	}

	std::vector<FVertexColor> Sphere2D;
	Sphere2D.push_back(FVertexColor(0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f));

	for (float i = 0; i < 360.f; i += 0.5f)
	{
		FVertexColor VC;

		//cosf랑 sinf 사용하기위해 각도를 radian으로 변경해준다.
		float radian = DirectX::XMConvertToRadians((float)i);

		//x랑 y값을 pos로 저장한다음에 원에 저장한다.
		VC.Pos.x = cosf(radian);
		VC.Pos.y = sinf(radian);
		VC.Pos.z = 0.f;

		VC.Color = FVector4(cosf(radian), sinf(radian), 1.f, 1.f);

		Sphere2D.push_back(VC);
	}

	std::vector<unsigned short> SphereIndex;

	size_t VCCount = Sphere2D.size() - 1;

	//0~30 
	//1~30
	for (size_t i = 0; i < VCCount; ++i)
	{
		unsigned short Current = (unsigned short)(i + 1);
		unsigned short Next = (unsigned short)((i + 1) % VCCount + 1);


		//원점
		SphereIndex.push_back(0);
		//현재 점
		SphereIndex.push_back(Next);
		SphereIndex.push_back(Current);
		//다음 점
	}

	if (!CreateMesh("Mesh_Sphere", &Sphere2D[0], sizeof(FVertexColor), (int)VCCount + 1, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
		, &SphereIndex[0], sizeof(unsigned short), (int)SphereIndex.size(), DXGI_FORMAT_R16_UINT, D3D11_USAGE_IMMUTABLE))
	{
		return false;
	}

	//텍스처 메쉬
	FVertexTexture TexRect[4] =
	{
		FVertexTexture(-0.5f,0.5f,0.f,0.f,0.f),
		FVertexTexture(0.5f,0.5f,0.f,1.f,0.f),
		FVertexTexture(-0.5f,-0.5f,0.f,0.f,1.f),
		FVertexTexture(0.5f,-0.5f,0.f,1.f,1.f),
	};

	if (!CreateMesh("Mesh_TexRect", TexRect, sizeof(FVertexTexture), 4, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
		, IndexRect, sizeof(unsigned short), 6, DXGI_FORMAT_R16_UINT, D3D11_USAGE_IMMUTABLE))
	{
		return false;
	}

	//정점의 좌표계에 영향을주는건 행렬이 영향을 주기 때문에
	//필요하다면 메쉬를 새로 만들어서 각각 영향받는 행렬에 맞춰서 메쉬를 만들어줘야한다.
	FVertexTexture UIRect[4]
	{
		FVertexTexture(0.f,0.f,0.f,0.f,0.f),//좌측상단
		FVertexTexture(1.f,0.f,0.f,1.f,0.f),//우측상단
		FVertexTexture(0.f,1.f,0.f,0.f,1.f),//좌측하단
		FVertexTexture(1.f,1.f,0.f,1.f,1.f)	//우측하단
	};

	if (!CreateMesh("Mesh_UITexRect", UIRect, sizeof(FVertexTexture), 4, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
		, IndexRect, sizeof(unsigned short), 6, DXGI_FORMAT_R16_UINT, D3D11_USAGE_IMMUTABLE))
	{
		return false;
	}

	FVertexTexture LBRect[4]
	{
		FVertexTexture(0.f,1.f,0.f,0.f,0.f),//좌측상단
		FVertexTexture(1.f,1.f,0.f,1.f,0.f),//우측상단
		FVertexTexture(0.f,0.f,0.f,0.f,1.f),//좌측하단
		FVertexTexture(1.f,0.f,0.f,1.f,1.f)	//우측하단
	};

	if (!CreateMesh("Mesh_LBTexRect", LBRect, sizeof(FVertexTexture), 4, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
		, IndexRect, sizeof(unsigned short), 6, DXGI_FORMAT_R16_UINT, D3D11_USAGE_IMMUTABLE))
	{
		return false;
	}

	FVector3 LBFrameRect[4] =
	{
		FVector3{0.f,1.f,0.f},
		FVector3{1.f,1.f,0.f},
		FVector3{0.f,0.f,0.f},
		FVector3{1.f,0.f,0.f},
	};

	if (!CreateMesh("Mesh_LBFrameRect", LBFrameRect, sizeof(FVector3), 4, D3D11_USAGE_IMMUTABLE, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP
		, IndexFrame, sizeof(unsigned short), 6, DXGI_FORMAT_R16_UINT, D3D11_USAGE_IMMUTABLE))
	{
		return false;
	}

	return true;
}

bool CMeshManager::CreateMesh(const std::string& Name, void* VertexData, int VertexSize, int VertexCount, D3D11_USAGE VertexUsage, D3D11_PRIMITIVE_TOPOLOGY Primitive, void* IndexData, int IndexSize, int IndexCount, DXGI_FORMAT Format, D3D11_USAGE IndexUsage)
{
	std::string MeshName = Name.substr(5, Name.length());

	std::weak_ptr<CMesh> Check = FindMesh(Name);

	if (!Check.expired())
	{
		return true;
	}

	std::shared_ptr<CMesh> NewMesh = std::make_shared<CMesh>();

	if (!NewMesh->CreateMesh(VertexData, VertexSize, VertexCount, VertexUsage, Primitive, IndexData, IndexSize, IndexCount, Format, IndexUsage))
	{
		return false;
	}

	NewMesh->SetName(Name);

	mMeshMap.insert(std::make_pair(Name, NewMesh));

	return true;
}

std::weak_ptr<CMesh> CMeshManager::FindMesh(const std::string& Name)
{
	std::string FullName = "Mesh_" + Name;

	auto iter = mMeshMap.find(FullName);

	if (iter == mMeshMap.end())
	{
		return std::weak_ptr<CMesh>();
	}

	return iter->second;
}
