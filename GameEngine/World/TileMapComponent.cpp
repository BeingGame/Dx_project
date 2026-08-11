#include "TileMapComponent.h"
#include "TileMapRender.h"

#include "../Device.h"

#include "CameraManager.h"
#include "WorldManager.h"
#include "World.h"

#include "../Asset/AssetManager.h"
#include "../Asset/TextureManager.h"
#include "../Asset/Texture.h"
#include "../Asset/MeshManager.h"
#include "../Asset/Mesh.h"
#include "../Asset/ShaderManager.h"
#include "../Asset/Shader.h"
#include "../Asset/CBufferTransform.h"
#include "../Asset/CBufferTileMap.h"
#include "../Asset/CBufferCollider.h"

#include "../LogManager.h"
#include "../Asset/PathManager.h"

CTileMapComponent::CTileMapComponent()
{}

CTileMapComponent::CTileMapComponent(const CTileMapComponent& src)
	:CActorComponent(src)
{}

CTileMapComponent::CTileMapComponent(CTileMapComponent&& src) noexcept
	:CActorComponent(std::move(src))
{}

CTileMapComponent::~CTileMapComponent()
{}


ETileType CTileMapComponent::GetTileType(int Index) const
{
	if (Index < 0 || Index >= mTileList.size())
	{
		return ETileType::End;
	}

	return mTileList[Index]->GetType();
}

int CTileMapComponent::GetTileIndex(const FVector2& Pos) const
{
	//이 함수에서 타일의 인덱스를 Position을 이용해서 구해준다.
	//현재 컴포넌트의 위치 - Position을 한뒤
	//이걸 타일 사이즈만큼 나눠줘서 int로 변환하면 인덱스가 나온다.

	auto Owner = mOwner.lock();

	if (!Owner)
	{
		return -1;
	}


	FVector3 OwnerPos = Owner->GetWorldPos();
	FVector2 ConvertPos;

	ConvertPos.x = Pos.x - OwnerPos.x;
	ConvertPos.y = Pos.y - OwnerPos.y;



	int IndexX = GetTileIndexX(ConvertPos);

	if (IndexX == -1)
	{
		return -1;
	}

	int IndexY = GetTileIndexY(ConvertPos);

	if (IndexY == -1)
	{
		return -1;
	}

	return IndexY * mCountX + IndexX;
}

int CTileMapComponent::GetTileIndex(const FVector3& Pos) const
{
	return GetTileIndex(FVector2(Pos.x, Pos.y));
}

int CTileMapComponent::GetTileIndex(float x, float y) const
{
	return GetTileIndex(FVector2(x, y));
}

std::weak_ptr<class CTile> CTileMapComponent::GetTile(const FVector2& Pos) const
{
	return GetTile(GetTileIndex(Pos));
}

std::weak_ptr<class CTile> CTileMapComponent::GetTile(const FVector3& Pos) const
{
	return  GetTile(GetTileIndex(Pos));
}

std::weak_ptr<class CTile> CTileMapComponent::GetTile(float x, float y) const
{
	return  GetTile(GetTileIndex(x, y));
}

std::weak_ptr<class CTile> CTileMapComponent::GetTile(int Index) const
{
	if (Index < 0 || Index >= (int)mTileList.size())
	{
		return std::weak_ptr<CTile>();
	}

	return mTileList[Index];
}

int CTileMapComponent::GetTileIndexX(const FVector2& Pos) const
{
	switch (mShape)
	{
	case ETileShape::Rect:
	{
		//타일맵의 시작지점부터 상대적인 위치를 구해준다.
		float Convert = Pos.x;

		if (Convert < 0.f)
		{
			return -1;
		}

		LOG_DEBUG("Mouse Local Pos x : ", Convert);

		//상대적인 위치랑 타일사이즈를 나눠준다.
		float Value = Convert / mTileSize.x;

		int Index = (int)Value;

		if (Index >= mCountX)
		{
			return -1;
		}

		return Index;
	}
	}
	return -1;
}

int CTileMapComponent::GetTileIndexY(const FVector2& Pos) const
{
	switch (mShape)
	{
	case ETileShape::Rect:
	{
		//타일맵의 시작지점부터 상대적인 위치를 구해준다.
		float Convert = Pos.y;

		if (Convert < 0.f)
		{
			return -1;
		}

		LOG_DEBUG("Mouse Local Pos y : ", Convert);

		//상대적인 위치랑 타일사이즈를 나눠준다.
		float Value = Convert / mTileSize.y;

		int Index = (int)Value;

		if (Index >= mCountY)
		{
			return -1;
		}

		return Index;
	}
	}
	return -1;
}

void CTileMapComponent::SetTileOutLineRender(bool Enable)
{
	mTileOutLineRender = Enable;

	size_t Size = mTileList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		mTileList[i]->SetOutLineRender(Enable);
	}
}

void CTileMapComponent::SetTileOutLineRender(bool Enable, int Index)
{
	mTileList[Index]->SetOutLineRender(Enable);
}

void CTileMapComponent::SetOutLineMesh(const std::string& Name)
{
	auto MeshMgr = CAssetManager::GetInst()->GetSubManager<CMeshManager>(EAssetType::Mesh);

	if (MeshMgr)
	{
		mOutLineMesh = MeshMgr->FindMesh(Name);
	}
}

void CTileMapComponent::SetOutLineShader(const std::string& Name)
{
	auto ShaderMgr = CAssetManager::GetInst()->GetSubManager<CShaderManager>(EAssetType::Shader);

	if (ShaderMgr)
	{
		mOutLineShader = ShaderMgr->FindShader(Name);
	}
}

void CTileMapComponent::SetTileMesh(const std::string& Name)
{
	auto MeshMgr = CAssetManager::GetInst()->GetSubManager<CMeshManager>(EAssetType::Mesh);

	if (MeshMgr)
	{
		mTileMesh = MeshMgr->FindMesh(Name);
	}
}

void CTileMapComponent::SetTileShader(const std::string& Name)
{
	auto ShaderMgr = CAssetManager::GetInst()->GetSubManager<CShaderManager>(EAssetType::Shader);

	if (ShaderMgr)
	{
		mTileShader = ShaderMgr->FindShader(Name);
	}
}

bool CTileMapComponent::SetTileTexture(ETileTextureType::Type Type, const std::weak_ptr<class CTexture>& Texture)
{
	if (mTileMapRender.expired())
	{
		return false;
	}

	return mTileMapRender.lock()->SetTexture(Type, Texture);
}

bool CTileMapComponent::SetTileTexture(ETileTextureType::Type Type, const std::string& Name)
{
	if (mTileMapRender.expired())
	{
		return false;
	}

	return mTileMapRender.lock()->SetTexture(Type, Name);;
}

bool CTileMapComponent::SetTileTexture(ETileTextureType::Type Type, const std::string& Name, const TCHAR* FileName, const std::string& PathName)
{
	if (mTileMapRender.expired())
	{
		return false;
	}

	return mTileMapRender.lock()->SetTexture(Type, Name, FileName, PathName);
}

void CTileMapComponent::AddTileFrame(const FVector2& Start, const FVector2& End)
{
	FTileFrame Frame;
	Frame.Start = Start;
	Frame.End = End;

	mTileFrame.push_back(Frame);
}

void CTileMapComponent::AddTileFrame(float StartX, float StartY, float EndX, float EndY)
{
	AddTileFrame(FVector2(StartX, StartY), FVector2(EndX, EndY));
}

void CTileMapComponent::SetTileFrameAll(int FrameIndex)
{
	if (FrameIndex < 0 || FrameIndex >= mTileFrame.size())
	{
		return;
	}

	auto iter = mTileList.begin();
	auto iterEnd = mTileList.end();

	FTileFrame Frame = mTileFrame[FrameIndex];

	for (; iter != iterEnd; ++iter)
	{
		(*iter)->SetFrame(Frame.Start, Frame.End);
	}
}

void CTileMapComponent::SetTileFrame(int Index, int FrameIndex)
{
	if (FrameIndex < 0 || FrameIndex >= mTileFrame.size())
	{
		return;
	}

	if (Index < 0 || Index >= mTileList.size())
	{
		return;
	}

	FTileFrame Frame = mTileFrame[FrameIndex];

	mTileList[Index]->SetFrame(Frame.Start, Frame.End);
}

bool CTileMapComponent::Init()
{
	mCBufferTransform = std::make_shared<CCBufferTransform>();
	mCBufferTransform->Init();

	mCBufferTileMap = std::make_shared<CCBufferTileMap>();
	mCBufferTileMap->Init();

	mCBufferCollider = std::make_shared<CCBufferCollider>();
	mCBufferCollider->Init();

	//타일의 기본 메쉬, 셰이더를 설정한다.
	SetTileMesh("LBTexRect");
	SetTileShader("TileMap");
	SetOutLineShader("FrameColor2D");

	return true;
}

void CTileMapComponent::Update(float DeltaTime)
{
	CActorComponent::Update(DeltaTime);
}

void CTileMapComponent::PostUpdate(float DeltaTime)
{
	CActorComponent::PostUpdate(DeltaTime);

	//타일이 렌더링 되기전에 렌더링이 불필요한 타일을 제외하기 위해서
	//반복문을 이용해 보이는 타일의 갯수를 갱신해준다.

	FVector3 Center;

	auto World = mWorld.lock();

	if (!World)
	{
		World = CWorldManager::GetInst()->GetWorld().lock();
	}

	Center = World->GetCameraManager().lock()->GetMainCameraPos();

	//타일맵의 시작점부터 상대적인 위치를 구해준다.
	//중앙점을 맵의 위치만큼 빼준다.
	Center -= mOwner.lock()->GetWorldPos();

	FResolution	RS = CDevice::GetInst()->GetResolution();

	switch (mShape)
	{
	case ETileShape::Rect:
		//타일 인덱스를 구해주기 위해 타일사이즈 만큼 나눠준다.
		mViewStartX = (int)((Center.x - RS.Width * 0.5f) / mTileSize.x);
		mViewStartY = (int)((Center.y - RS.Height * 0.5f) / mTileSize.y);
		mViewEndX = (int)((Center.x + RS.Width * 0.5f) / mTileSize.x);
		mViewEndY = (int)((Center.y + RS.Height * 0.5f) / mTileSize.y);
		break;
	default:
		break;
	}

	//인덱스가 0 미만이거나, 현재 타일맵의 Count보다 크지않게 Clamp를 해준다.

	mViewStartX = std::clamp<int>(mViewStartX, 0, mCountX - 1);
	mViewStartY = std::clamp<int>(mViewStartY, 0, mCountY - 1);

	mViewEndX = std::clamp<int>(mViewEndX, 0, mCountX - 1);
	mViewEndY = std::clamp<int>(mViewEndY, 0, mCountY - 1);
}

void CTileMapComponent::CreateTile(ETileShape Shape, int CountX, int CountY, const FVector2& TileSize, int TileTextureFrame, bool OutLineRender)
{
	//타일 생성
	mShape = Shape;
	mCountX = CountX;
	mCountY = CountY;
	mTileSize = TileSize;

	auto MeshMgr = CAssetManager::GetInst()->GetSubManager<CMeshManager>(EAssetType::Mesh);

	//타일 사이즈에 따라 맵크기를 지정해준다.
	switch (mShape)
	{
	case ETileShape::Rect:
		mMapSize = mTileSize * FVector2((float)mCountX, (float)mCountY);

		if (MeshMgr)
		{
			mOutLineMesh = MeshMgr->FindMesh("LBFrameRect");
		}

		break;
	default:
		break;
	}

	auto Render = mTileMapRender.lock();

	if (Render)
	{
		Render->SetWorldScale(mMapSize);
	}

	mTileList.clear();

	//미리 공간을 할당한다.
	mTileList.resize(mCountX * mCountY);

	for (int i = 0; i < mCountY; ++i)
	{
		for (int j = 0; j < mCountX; ++j)
		{
			int Index = i * mCountX + j;

			mTileList[Index] = std::make_shared<CTile>();

			mTileList[Index]->SetIndex(i, j, Index);

			//사각형의 경우 타일의 위치를 타일 사이즈 * 현재 인덱스 번호만큼으로 지정해준다.

			switch (mShape)
			{
			case ETileShape::Rect:
				mTileList[Index]->SetPos(j * mTileSize.x, i * mTileSize.y);
				break;
			default:
				break;
			}

			mTileList[Index]->SetCenter(mTileList[Index]->GetPos() + mTileSize * 0.5f);
			mTileList[Index]->SetSize(mTileSize);
			mTileList[Index]->SetTextureFrame(TileTextureFrame);
			mTileList[Index]->SetOutLineRender(OutLineRender);
		}
	}
}

void CTileMapComponent::Save(const TCHAR* FileName, const std::string& PathName)
{
	TCHAR FullPath[MAX_PATH] = {};
	lstrcpy(FullPath, CPathManager::FindPath(PathName));
	lstrcat(FullPath, FileName);

	SaveFullPath(FullPath);
}

void CTileMapComponent::SaveFullPath(const TCHAR* FullPath)
{
	char FullPathMultiByte[MAX_PATH] = {};

	int Count = WideCharToMultiByte(CP_ACP, 0, FullPath, -1, nullptr, 0, nullptr, nullptr);
	WideCharToMultiByte(CP_ACP, 0, FullPath, -1, FullPathMultiByte, Count, nullptr, nullptr);

	std::ofstream File(FullPathMultiByte, std::ios::out | std::ios::binary);

	if (!File.is_open())
	{
		return;
	}

	auto Render = mTileMapRender.lock();

	Render->Save(File);

	size_t Size = mTileList.size();

	File.write((char*)(&Size), (sizeof(size_t)));

	for (size_t i = 0; i < Size; ++i)
	{
		mTileList[i]->Save(File);
	}

	File.write((char*)(&mShape), sizeof(ETileShape));
	File.write((char*)(&mTileSize), sizeof(FVector2));
	File.write((char*)(&mMapSize), sizeof(FVector2));

	File.write((char*)(&mCountX), sizeof(int));
	File.write((char*)(&mCountY), sizeof(int));
	File.write((char*)(&mTileOutLineRender), sizeof(bool));

	auto OutLineMesh = mOutLineMesh.lock();

	bool Enable = false;

	if (OutLineMesh)
	{
		Enable = true;
	}

	File.write((char*)(&Enable), sizeof(bool));

	if (OutLineMesh)
	{
		std::string Name = OutLineMesh->GetName().substr(5);

		size_t Count = Name.length();

		File.write((char*)(&Count), sizeof(size_t));
		File.write((char*)(Name.c_str()), Name.length());
	}

	auto OutLineShader = mOutLineShader.lock();

	Enable = false;

	if (OutLineShader)
	{
		Enable = true;
	}

	File.write((char*)(&Enable), sizeof(bool));

	if (OutLineShader)
	{
		std::string Name = OutLineShader->GetName();

		size_t Count = Name.length();

		File.write((char*)(&Count), sizeof(size_t));

		File.write((char*)(Name.c_str()), Name.length());
	}

	auto TileMesh = mTileMesh.lock();

	Enable = false;

	if (TileMesh)
	{
		Enable = true;
	}

	File.write((char*)(&Enable), sizeof(bool));

	if (TileMesh)
	{
		std::string Name = TileMesh->GetName().substr(5);

		size_t Count = Name.length();

		File.write((char*)(&Count), sizeof(size_t));
		File.write((char*)(Name.c_str()), Name.length());
	}

	auto TileShader = mTileShader.lock();

	Enable = false;

	if (TileShader)
	{
		Enable = true;
	}

	File.write((char*)(&Enable), sizeof(bool));

	if (TileShader)
	{
		std::string Name = TileShader->GetName();

		size_t Count = Name.length();

		File.write((char*)(&Count), sizeof(size_t));

		File.write((char*)(Name.c_str()), Name.length());
	}

	File.write((char*)(&mTileTextureSize), sizeof(FVector2));

	Size = mTileFrame.size();

	File.write((char*)(&Size), sizeof(size_t));
	File.write((char*)(&mTileFrame[0]), sizeof(FTileFrame) * Size);
}

void CTileMapComponent::Load(const TCHAR* FileName, const std::string& PathName)
{
	TCHAR FullPath[MAX_PATH] = {};
	lstrcpy(FullPath, CPathManager::FindPath(PathName));
	lstrcat(FullPath, FileName);

	LoadFullPath(FullPath);
}

void CTileMapComponent::LoadFullPath(const TCHAR* FullPath)
{
	char FullPathMultiByte[MAX_PATH] = {};

	int Count = WideCharToMultiByte(CP_ACP, 0, FullPath, -1, nullptr, 0, nullptr, nullptr);
	WideCharToMultiByte(CP_ACP, 0, FullPath, -1, FullPathMultiByte, Count, nullptr, nullptr);

	std::ifstream File(FullPathMultiByte, std::ios::in | std::ios::binary);

	if (!File.is_open())
	{
		return;
	}

	auto Render = mTileMapRender.lock();

	Render->Load(File);

	size_t Size = mTileList.size();

	File.read((char*)(&Size), (sizeof(size_t)));

	mTileList.clear();
	mTileList.resize(Size);

	for (size_t i = 0; i < Size; ++i)
	{
		mTileList[i] = std::make_shared<CTile>();

		mTileList[i]->Load(File);
	}

	File.read((char*)(&mShape), sizeof(ETileShape));
	File.read((char*)(&mTileSize), sizeof(FVector2));
	File.read((char*)(&mMapSize), sizeof(FVector2));

	File.read((char*)(&mCountX), sizeof(int));
	File.read((char*)(&mCountY), sizeof(int));
	File.read((char*)(&mTileOutLineRender), sizeof(bool));

	bool Enable = false;

	File.read((char*)(&Enable), sizeof(bool));

	if (Enable)
	{
		char Name[256] = {};

		size_t Count = 0;

		File.read((char*)(&Count), sizeof(size_t));

		File.read((char*)(&Name), sizeof(char) * Count);

		SetOutLineMesh(Name);
	}

	Enable = false;

	File.read((char*)(&Enable), sizeof(bool));

	if (Enable)
	{
		char Name[256] = {};

		size_t Count = 0;

		File.read((char*)(&Count), sizeof(size_t));

		File.read((char*)(&Name), sizeof(char) * Count);

		SetOutLineShader(Name);
	}
	
	Enable = false;

	File.read((char*)(&Enable), sizeof(bool));

	if (Enable)
	{
		char Name[256] = {};

		size_t Count = 0;

		File.read((char*)(&Count), sizeof(size_t));

		File.read((char*)(&Name), sizeof(char) * Count);

		SetTileMesh(Name);
	}

	Enable = false;

	File.read((char*)(&Enable), sizeof(bool));

	if (Enable)
	{
		char Name[256] = {};

		size_t Count = 0;

		File.read((char*)(&Count), sizeof(size_t));

		File.read((char*)(&Name), sizeof(char) * Count);

		SetTileShader(Name);
	}

	File.read((char*)(&mTileTextureSize), sizeof(FVector2));

	Size = 0;

	File.read((char*)(&Size), sizeof(size_t));

	mTileFrame.clear();
	mTileFrame.resize(Size);

	File.read((char*)(&mTileFrame[0]), sizeof(FTileFrame) * Size);
}

//실제적으로 타일이 보여질 인덱스를 구해준다.
int CTileMapComponent::GetTileRenderIndexX(const FVector2& Pos) const
{
	switch (mShape)
	{
	case ETileShape::Rect:
	{
		//타일맵의 시작지점부터 상대적인 위치를 구해준다.
		float Convert = Pos.x - mOwner.lock()->GetWorldPos().x;

		//상대적인 위치랑 타일사이즈를 나눠준다.
		float Value = Convert / mTileSize.x;

		int Index = (int)Value;

		if (Index < 0)
		{
			return 0;
		}

		else if (Index >= mCountX)
		{
			return mCountX - 1;
		}

		return Index;
	}
	}

	return -1;
}

int CTileMapComponent::GetTileRenderIndexY(const FVector2& Pos) const
{
	switch (mShape)
	{
	case ETileShape::Rect:
	{
		//타일맵의 시작지점부터 상대적인 위치를 구해준다.
		float Convert = Pos.y - mOwner.lock()->GetWorldPos().y;

		//상대적인 위치랑 타일사이즈를 나눠준다.
		float Value = Convert / mTileSize.y;

		int Index = (int)Value;

		if (Index < 0)
		{
			return 0;
		}

		else if (Index >= mCountY)
		{
			return mCountY - 1;
		}

		return Index;
	}
	}

	return -1;
}