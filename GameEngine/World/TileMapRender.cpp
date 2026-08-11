#include "TileMapRender.h"
#include "TileMapComponent.h"

#include "../RenderManager.h"
#include "../RenderState.h"

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
#include "../Asset/CBufferMaterial.h"

#include "World.h"
#include "WorldManager.h"
#include "CameraManager.h"

#include <fstream>

CTileMapRender::CTileMapRender()
{}

CTileMapRender::CTileMapRender(const CTileMapRender& src)
	:CSceneComponent(src)
{}

CTileMapRender::CTileMapRender(CTileMapRender&& src) noexcept
	:CSceneComponent(std::move(src))
{}

CTileMapRender::~CTileMapRender()
{}

void CTileMapRender::SetTileMapComponent(const std::weak_ptr<class CTileMapComponent>& TileMap)
{
	mTileMap = TileMap;

	auto TileMapComp = mTileMap.lock();

	if (TileMapComp)
	{
		TileMapComp->SetTileMapRender(GetThisPtr<CTileMapRender>());

		SetWorldScale(TileMapComp->GetMapSize());
	}
}

void CTileMapRender::EnableAlphaBlend()
{
	mAlphaBlend = CRenderManager::GetInst()->FindRenderState("AlphaBlend");
}

void CTileMapRender::EnableTileAlphaBlend()
{
	mTileAlphaBlend = CRenderManager::GetInst()->FindRenderState("AlphaBlend");
}

bool CTileMapRender::SetTexture(ETileTextureType::Type Type, const std::weak_ptr<class CTexture>& Texture)
{
	mTexture[Type] = Texture;

	if (Type == ETileTextureType::Tile)
	{
		auto Texture = mTexture[Type].lock();
		auto Comp = mTileMap.lock();

		if (Comp)
		{
			Comp->SetTileTextureSize((float)Texture->GetTexture()->Width, (float)Texture->GetTexture()->Height);
		}
	}

	return true;
}

bool CTileMapRender::SetTexture(ETileTextureType::Type Type, const std::string& Name)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (TextureMgr)
	{
		mTexture[Type] = TextureMgr->FindTexture(Name);
	}

	if (Type == ETileTextureType::Tile)
	{
		auto Texture = mTexture[Type].lock();
		auto Comp = mTileMap.lock();

		if (Comp)
		{
			Comp->SetTileTextureSize((float)Texture->GetTexture()->Width, (float)Texture->GetTexture()->Height);
		}
	}

	return true;

}

bool CTileMapRender::SetTexture(ETileTextureType::Type Type, const std::string& Name, const TCHAR* FileName, const std::string& PathName)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (TextureMgr)
	{
		if (TextureMgr->LoadTexture(Name, FileName, PathName))
		{
			mTexture[Type] = TextureMgr->FindTexture(Name);

			if (Type == ETileTextureType::Tile)
			{
				auto Texture = mTexture[Type].lock();
				auto Comp = mTileMap.lock();

				if (Comp)
				{
					Comp->SetTileTextureSize((float)Texture->GetTexture()->Width, (float)Texture->GetTexture()->Height);
				}
			}

		}
	}

	return false;
}

bool CTileMapRender::SetTextureFullPath(ETileTextureType::Type Type, const std::string& Name, const TCHAR* FullPath)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (TextureMgr)
	{
		if (TextureMgr->LoadTexture(Name, FullPath))
		{
			mTexture[Type] = TextureMgr->FindTexture(Name);

			if (Type == ETileTextureType::Tile)
			{
				auto Texture = mTexture[Type].lock();
				auto Comp = mTileMap.lock();

				if (Comp)
				{
					Comp->SetTileTextureSize((float)Texture->GetTexture()->Width, (float)Texture->GetTexture()->Height);
				}
			}

		}
	}

	return false;
}

bool CTileMapRender::SetTextureFullPath(ETileTextureType::Type Type, const std::string& Name, const std::vector<const TCHAR*>& FullPath)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (TextureMgr)
	{
		if (TextureMgr->LoadTexture(Name, FullPath))
		{
			mTexture[Type] = TextureMgr->FindTexture(Name);

			if (Type == ETileTextureType::Tile)
			{
				auto Texture = mTexture[Type].lock();
				auto Comp = mTileMap.lock();

				if (Comp)
				{
					Comp->SetTileTextureSize((float)Texture->GetTexture()->Width, (float)Texture->GetTexture()->Height);
				}
			}

		}
	}

	return false;
}

void CTileMapRender::SetBackMesh(const std::string& Name)
{
	auto MeshMgr = CAssetManager::GetInst()->GetSubManager<CMeshManager>(EAssetType::Mesh);

	if (MeshMgr)
	{
		mBackMesh = MeshMgr->FindMesh(Name);
	}
}

void CTileMapRender::SetBackMesh(const std::weak_ptr<class CMesh>& Mesh)
{
	mBackMesh = Mesh;
}

void CTileMapRender::SetBackShader(const std::string& Name)
{
	auto ShaderMgr = CAssetManager::GetInst()->GetSubManager<CShaderManager>(EAssetType::Shader);

	if (ShaderMgr)
	{
		mBackShader = ShaderMgr->FindShader(Name);
	}
}

void CTileMapRender::SetBackShader(const std::weak_ptr<class CShader>& Shader)
{
	mBackShader = Shader;
}

void CTileMapRender::SetTileFrameSize(float x, float y)
{}

void CTileMapRender::AddTileFrame(const FVector2& Start, const FVector2& End)
{
	auto Comp = mTileMap.lock();

	if (Comp)
	{
		Comp->AddTileFrame(Start, End);
	}
}

void CTileMapRender::AddTileFrame(float StartX, float StartY, float EndX, float EndY)
{

	auto Comp = mTileMap.lock();

	if (Comp)
	{
		Comp->AddTileFrame(StartX, StartY, EndX, EndY);
	}
}

bool CTileMapRender::Init()
{
	CSceneComponent::Init();

	SetRenderLayer(0);

	auto TileMap = mTileMap.lock();

	if (TileMap)
	{
		SetWorldScale(TileMap->GetMapSize());
	}

	mCBufferTransform = std::make_shared<CCBufferTransform>();
	mCBufferTransform->Init();

	mCBufferMaterial = std::make_shared<CCBufferMaterial>();
	mCBufferMaterial->Init();

	mCBufferMaterial->SetColor(FVector4::White);
	mCBufferMaterial->SetOpacity(1.f);

	SetBackMesh("LBTexRect");
	SetBackShader("Texture");

	return true;
}

void CTileMapRender::Render()
{
	CSceneComponent::Render();

	//먼저 뒤에 출력될 백그라운드 이미지를 출력해준다.

	if (!mTexture[ETileTextureType::Back].expired())
	{
		FMatrix ScaleMat, TranslateMat, WorldMat;

		ScaleMat.Scaling(mWorldScale);
		TranslateMat.Translation(mWorldPos);

		WorldMat = ScaleMat * TranslateMat;

		auto World = mWorld.lock();

		if (!World)
		{
			World = CWorldManager::GetInst()->GetWorld().lock();
		}

		auto CameraManager = World->GetCameraManager().lock();

		mCBufferTransform->SetWorldMatrix(WorldMat);
		mCBufferTransform->SetViewMatrix(CameraManager->GetViewMat());
		mCBufferTransform->SetProjMatrix(CameraManager->GetProjMat());

		mCBufferTransform->UpdateBuffer();
		mCBufferMaterial->UpdateBuffer();

		auto Mesh = mBackMesh.lock();
		auto Shader = mBackShader.lock();

		auto Texture = mTexture[ETileTextureType::Back].lock();

		Texture->SetShader(0, EShaderBufferType::Pixel, 0);

		auto State = mAlphaBlend.lock();

		if (State)
		{
			State->SetState();
		}

		Shader->SetShader();
		Mesh->Render();

		if (State)
		{
			State->ResetState();
		}
	}

	if (!mTileMap.expired())
	{
		auto TileMap = mTileMap.lock();
		auto Owner = TileMap->GetOwner().lock();
		auto World = mWorld.lock();
		if (!World)
		{
			World = CWorldManager::GetInst()->GetWorld().lock();
		}

		auto CameraManager = World->GetCameraManager().lock();

		int StartX = TileMap->GetViewStartX();
		int StartY = TileMap->GetViewStartY();
		int EndX = TileMap->GetViewEndX();
		int EndY = TileMap->GetViewEndY();
		int CountX = TileMap->GetTileCountX();
		FVector2 TileSize = TileMap->GetTileSize();
		FVector2 TileTextureSize = TileMap->GetTileTextureSize();
		const auto& TileList = TileMap->GetTileList();
		auto CBufferTransform = TileMap->GetCBufferTransform();
		auto CBufferTileMap = TileMap->GetCBufferTileMap();
		auto CBufferCollider = TileMap->GetCBufferCollider();

		if (!mTexture[ETileTextureType::Tile].expired())
		{
			auto Texture = mTexture[ETileTextureType::Tile].lock();

			Texture->SetShader(1, EShaderBufferType::Pixel, 0);

			auto State = mTileAlphaBlend.lock();

			if (State)
			{
				State->SetState();
			}

			auto Mesh = TileMap->GetTileMesh().lock();
			auto Shader = TileMap->GetTileShader().lock();

			//현재 렌더링이 가능한 타일의 갯수만큼 렌더링을 시도한다.
			for (int i = StartY; i <= EndY; ++i)
			{
				for (int j = StartX; j <= EndX; ++j)
				{
					int Index = i * CountX + j;

					if (!TileList[Index]->IsRender())
					{
						continue;
					}


					FMatrix ScaleMat, TranslateMat, WorldMat;
					ScaleMat.Scaling(TileSize);

					FVector2 Pos = TileList[Index]->GetPos();

					Pos.x += Owner->GetWorldPos().x;
					Pos.y += Owner->GetWorldPos().y;

					TranslateMat.Translation(Pos);

					WorldMat = ScaleMat * TranslateMat;

					CBufferTransform->SetWorldMatrix(WorldMat);
					CBufferTransform->SetViewMatrix(CameraManager->GetViewMat());
					CBufferTransform->SetProjMatrix(CameraManager->GetProjMat());

					CBufferTransform->UpdateBuffer();

					FVector2 LTUV = TileList[Index]->GetFrameStart() / TileTextureSize;
					FVector2 RBUV = TileList[Index]->GetFrameEnd() / TileTextureSize;

					CBufferTileMap->SetUV(LTUV, RBUV);
					CBufferTileMap->UpdateBuffer();


					if (Shader)
					{
						Shader->SetShader();
					}

					if (Mesh)
					{
						Mesh->Render();
					}

				}
			}

			if (State)
			{
				State->ResetState();
			}

		}

		auto OutLineMesh = TileMap->GetOutLineMesh().lock();
		auto OutLineShader = TileMap->GetOutLineShader().lock();

		//현재 렌더링이 가능한 타일의 갯수만큼 렌더링을 시도한다.
		for (int i = StartY; i <= EndY; ++i)
		{
			for (int j = StartX; j <= EndX; ++j)
			{
				int Index = i * CountX + j;

				if (!TileList[Index]->IsOutLineRender())
				{
					continue;
				}

				FMatrix ScaleMat, TranslateMat, WorldMat;
				FVector2 OutLineScale = TileSize * 0.99f;
				ScaleMat.Scaling(OutLineScale);

				FVector2 Pos = TileList[Index]->GetPos();

				Pos.x += Owner->GetWorldPos().x;
				Pos.y += Owner->GetWorldPos().y;

				FVector2 Offset = (TileSize - OutLineScale) * 0.5f;

				Pos.x += Offset.x;
				Pos.y += Offset.y;
				TranslateMat.Translation(Pos);

				WorldMat = ScaleMat * TranslateMat;

				CBufferTransform->SetWorldMatrix(WorldMat);
				CBufferTransform->SetViewMatrix(CameraManager->GetViewMat());
				CBufferTransform->SetProjMatrix(CameraManager->GetProjMat());

				CBufferTransform->UpdateBuffer();

				FVector4 Color = TileList[Index]->GetOutLineColor();
				CBufferCollider->SetColor(Color);
				CBufferCollider->UpdateBuffer();

				if (OutLineShader)
				{
					OutLineShader->SetShader();
				}

				if (OutLineMesh)
				{
					OutLineMesh->Render();
				}
			}
		}
	}
}

CTileMapRender* CTileMapRender::Clone() const
{
	return new CTileMapRender(*this);
}

void CTileMapRender::Save(std::ofstream& File)
{
	auto BackMesh = mBackMesh.lock();

	bool Enable = false;

	if (BackMesh)
	{
		Enable = true;
	}

	File.write((char*)(&Enable), sizeof(bool));

	if (BackMesh)
	{
		std::string Name = BackMesh->GetName().substr(5);

		size_t Count = Name.length();

		File.write((char*)(&Count), sizeof(size_t));
		File.write((char*)(Name.c_str()), Name.length());
	}

	auto BackShader = mBackShader.lock();

	Enable = false;

	if (BackShader)
	{
		Enable = true;
	}

	File.write((char*)(&Enable), sizeof(bool));

	if (BackShader)
	{
		std::string Name = BackShader->GetName();

		size_t Count = Name.length();

		File.write((char*)(&Count), sizeof(size_t));

		File.write((char*)(Name.c_str()), Name.length());
	}

	for (int i = 0; i < ETileTextureType::End; ++i)
	{
		auto Texture = mTexture[i].lock();

		Enable = false;

		if (Texture)
		{
			Enable = true;
		}

		File.write((char*)(&Enable), sizeof(bool));

		if (Texture)
		{
			std::string Name = Texture->GetName().substr(8);

			size_t Count = Name.length();

			File.write((char*)(&Count), sizeof(size_t));

			File.write((char*)(Name.c_str()), Name.length());

			Texture->Save(File);
		}

	}

	auto Alpha = mAlphaBlend.lock();

	Enable = false;

	if (Alpha)
	{
		Enable = true;
	}

	File.write((char*)(&Enable), sizeof(bool));

	auto TileAlpha = mTileAlphaBlend.lock();

	Enable = false;

	if (TileAlpha)
	{
		Enable = true;
	}

	File.write((char*)(&Enable), sizeof(bool));

	File.write((char*)(&mWorldScale), sizeof(FVector3));
	File.write((char*)(&mWorldRot), sizeof(FVector3));
	File.write((char*)(&mWorldPos), sizeof(FVector3));
	File.write((char*)(&mWorldAxis), sizeof(FVector3) * EAxis::End);
}

void CTileMapRender::Load(std::ifstream& File)
{
	bool Enable = false;

	File.read((char*)(&Enable), sizeof(bool));

	if (Enable)
	{
		char Name[256] = {};

		size_t Count = 0;

		File.read((char*)(&Count), sizeof(size_t));

		File.read((char*)(&Name), sizeof(char) * Count);

		SetBackMesh(Name);
	}

	Enable = false;

	File.read((char*)(&Enable), sizeof(bool));

	if (Enable)
	{
		char Name[256] = {};

		size_t Count = 0;

		File.read((char*)(&Count), sizeof(size_t));

		File.read((char*)(&Name), sizeof(char) * Count);

		SetBackShader(Name);
	}

	for (int i = 0; i < ETileTextureType::End; ++i)
	{
		Enable = false;

		File.read((char*)(&Enable), sizeof(bool));

		if (Enable)
		{
			char Name[256] = {};

			size_t Count = 0;

			File.read((char*)(&Count), sizeof(size_t));

			File.read((char*)(&Name), sizeof(char) * Count);

			size_t TexCount = 0;
			File.read((char*)(&TexCount), sizeof(size_t));

			if (TexCount > 1)
			{
				std::vector<const TCHAR*> FullPathArray;

				for (size_t j = 0; j < TexCount; ++j)
				{
					size_t PathCount = 0;

					TCHAR* FullPath = new TCHAR[MAX_PATH];
					memset(FullPath, 0, sizeof(TCHAR) * MAX_PATH);

					File.read((char*)(&PathCount), sizeof(size_t));
					File.read((char*)(&FullPath), sizeof(wchar_t) * PathCount);

					FullPathArray.push_back(FullPath);
				}

				SetTextureFullPath((ETileTextureType::Type)i, Name, FullPathArray);

				for (size_t j = 0; j < TexCount; ++j)
				{
					if (FullPathArray[j])
					{
						delete[] FullPathArray[j];
					}
				}
			}
			else
			{
				size_t PathCount = 0;
				TCHAR FullPath[MAX_PATH] = {};

				File.read((char*)(&PathCount), sizeof(size_t));
				File.read((char*)(&FullPath), sizeof(wchar_t) * PathCount);

				SetTextureFullPath((ETileTextureType::Type)i, Name, FullPath);
			}
		}
	}

	Enable = false;

	File.read((char*)(&Enable), sizeof(bool));

	if (Enable)
	{
		EnableAlphaBlend();
	}
	Enable = false;

	File.read((char*)(&Enable), sizeof(bool));

	if (Enable)
	{
		EnableTileAlphaBlend();
	}

	File.read((char*)(&mWorldScale), sizeof(FVector3));
	File.read((char*)(&mWorldRot), sizeof(FVector3));
	File.read((char*)(&mWorldPos), sizeof(FVector3));
	File.read((char*)(&mWorldAxis), sizeof(FVector3) * EAxis::End);

	SetWorldPos(mWorldPos);
	SetWorldRotation(mWorldRot);
	SetWorldScale(mWorldScale);
}
