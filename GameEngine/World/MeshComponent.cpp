#include "MeshComponent.h"

#include "../Asset/AssetManager.h"
#include "../Asset/MeshManager.h"
#include "../Asset/Mesh.h"
#include "../Asset/ShaderManager.h"
#include "../Asset/Shader.h"
#include "../Asset/CBufferTransform.h"
#include "../Asset/Material.h"
#include "../Asset/MaterialManager.h"
#include "../RenderState.h"

#include "../Device.h"

#include "World.h"
#include "CameraManager.h"
#include "Animation2DComponent.h"

CMeshComponent::CMeshComponent()
{}

CMeshComponent::CMeshComponent(const CMeshComponent& src)
	:CSceneComponent(src)
{}

CMeshComponent::CMeshComponent(CMeshComponent&& src) noexcept
	:CSceneComponent(std::move(src))
{}

CMeshComponent::~CMeshComponent()
{}

void CMeshComponent::SetMesh(const std::weak_ptr<class CMesh> Mesh)
{
	mMesh = Mesh;

	//메쉬가 설정될때 메쉬슬롯의 머티리얼을 컴포넌트에 복제해준다.
	//단 CreateMaterialInstance를 사용하지 않는다.
	//이유는 생성은 되긴 하는데 메쉬가 갖고있는 고유한 머티리얼 속성이 사라지게 된다.

	auto _Mesh = mMesh.lock();

	//메쉬를 바꾸면 슬롯도 새 메쉬 것으로 갈아엎는다.
	//예전에는 지우지 않고 push_back만 해서, 메쉬를 두 번 고르면 슬롯이 쌓이고
	//슬롯 0은 계속 옛날 메쉬의 머티리얼을 가리키고 있었다.
	mMaterialSlot.clear();

	if (_Mesh)
	{
		size_t Size = _Mesh->GetMeshSlot().size();

		for (size_t i = 0; i < Size; ++i)
		{
			auto MatInstance = _Mesh->GetMeshSlot()[i]->Material->Clone();

			std::shared_ptr<CMaterial> NewMat(MatInstance);

			mMaterialSlot.push_back(NewMat);
		}
	}
}

void CMeshComponent::SetMesh(const std::string& Name)
{
	auto MeshMgr = CAssetManager::GetInst()->GetSubManager<CMeshManager>(EAssetType::Mesh);

	if (MeshMgr)
	{
		//FindMesh는 이름 앞에 "Mesh_"를 붙여서 찾는다.
		//그래서 CMesh::GetName()이 주는 "Mesh_TexRect"를 그대로 넘기면
		//"Mesh_Mesh_TexRect"를 찾다가 실패한다. (예전 씬 파일이 이 형태로 저장돼 있다)
		std::string Key = Name;

		static const std::string Prefix = "Mesh_";

		if (Key.size() > Prefix.size() && Key.compare(0, Prefix.size(), Prefix) == 0)
		{
			Key = Key.substr(Prefix.size());
		}

		auto Check = MeshMgr->FindMesh(Key);

		if (Check.expired())
		{
			return;
		}

		SetMesh(Check);
	}
}

void CMeshComponent::SetShader(const std::weak_ptr<class CShader> Shader)
{
	mShader = Shader;

	if (!mShader.expired())
	{
		size_t Size = mMaterialSlot.size();

		for (size_t i = 0; i < Size; ++i)
		{
			mMaterialSlot[i]->SetPixelShader(mShader.lock()->GetName());
		}
	}
}

void CMeshComponent::SetShader(const std::string& Name)
{
	auto ShaderMgr = CAssetManager::GetInst()->GetSubManager<CShaderManager>(EAssetType::Shader);

	if (ShaderMgr)
	{
		auto Check = ShaderMgr->FindShader(Name);

		if (Check.expired())
		{
			return;
		}

		SetShader(Check);
	}
}

void CMeshComponent::SetBlendState(int SlotIndex, const std::string& Name)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetRenderState(Name);
}

void CMeshComponent::AddTexture(int SlotIndex, const std::weak_ptr<class CTexture>& Texture, int Register, int ShaderBufferType, int Index)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->AddTexture(Texture, Register, ShaderBufferType, Index);
}

void CMeshComponent::AddTexture(int SlotIndex, const std::string& Name, int Register, int ShaderBufferType, int Index)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->AddTexture(Name, Register, ShaderBufferType, Index);
}

void CMeshComponent::AddTexture(int SlotIndex, const std::string& Name, const TCHAR* FileName, const std::string& PathName, int Register, int ShaderBufferType, int Index)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->AddTexture(Name, FileName, PathName, Register, ShaderBufferType, Index);
}

void CMeshComponent::AddTextureFullPath(int SlotIndex, const std::string& Name, const TCHAR* FullPath, int Register, int ShaderBufferType, int Index)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->AddTextureFullPath(Name, FullPath, Register, ShaderBufferType, Index);
}

void CMeshComponent::AddTexture(int SlotIndex, const std::string& Name, std::vector<const TCHAR*> FileName, const std::string& PathName, int Register, int ShaderBufferType, int Index)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->AddTexture(Name, FileName, PathName, Register, ShaderBufferType, Index);
}

void CMeshComponent::AddTextureFullPath(int SlotIndex, const std::string& Name, std::vector<const TCHAR*> FullPath, int Register, int ShaderBufferType, int Index)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->AddTextureFullPath(Name, FullPath, Register, ShaderBufferType, Index);
}

bool CMeshComponent::SetTexture(int SlotIndex, int TextureIndex, const std::weak_ptr<class CTexture>& Texture)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return false;
	}

	return mMaterialSlot[SlotIndex]->SetTexture(TextureIndex, Texture);
}

void CMeshComponent::SetBaseColor(int SlotIndex, const FVector4& Color)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetBaseColor(Color);
}

void CMeshComponent::SetBaseColor(int SlotIndex, float r, float g, float b, float a)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetBaseColor(r, g, b, a);
}

void CMeshComponent::SetBaseColor(int SlotIndex, int r, int g, int b, int a)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetBaseColor(r, g, b, a);
}

void CMeshComponent::SetOpacity(int SlotIndex, float Opacity)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetOpacity(Opacity);
}

void CMeshComponent::SetChromaKeyEnable(int SlotIndex, bool Enable)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetChromaKeyEnable(Enable);
}

void CMeshComponent::SetChromaKeyColor(int SlotIndex, const FVector3& Color)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetChromaKeyColor(Color);
}

void CMeshComponent::SetChromaKeyThreshold(int SlotIndex, float Threshold)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetChromaKeyThreshold(Threshold);
}

void CMeshComponent::SetChromaKey(int SlotIndex, bool Enable, const FVector3& Color, float Threshold)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetChromaKey(Enable, Color, Threshold);
}

void CMeshComponent::SetUVScrollEnable(int SlotIndex, bool Enable)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetUVScrollEnable(Enable);
}

void CMeshComponent::SetUVScrollSpeed(int SlotIndex, const FVector2& Speed)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetUVScrollSpeed(Speed);
}

void CMeshComponent::SetUVScrollTime(int SlotIndex, float Time)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetUVScrollTime(Time);
}

void CMeshComponent::SetUVScroll(int SlotIndex, bool Enable, const FVector2& Speed, float Time)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetUVScroll(Enable, Speed, Time);
}

void CMeshComponent::SetDissolveEnable(int SlotIndex, bool Enable)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetDissolveEnable(Enable);
}

void CMeshComponent::SetDissolveEdgeColor(int SlotIndex, const FVector4& Color)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetDissolveEdgeColor(Color);
}

void CMeshComponent::SetDissolveProgress(int SlotIndex, float Progress)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetDissolveProgress(Progress);
}

void CMeshComponent::SetDissolveEdgeWidth(int SlotIndex, float Width)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetDissolveEdgeWidth(Width);
}

void CMeshComponent::SetDissolve(int SlotIndex, bool Enable, const FVector4& Color, float Progress, float Width)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetDissolve(Enable, Color, Progress, Width);
}

void CMeshComponent::SetHitEffectEnable(int SlotIndex, bool Enable)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetHitEffectEnable(Enable);
}

void CMeshComponent::SetHitIntensity(int SlotIndex, float Intensity)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetHitIntensity(Intensity);
}

void CMeshComponent::SetHitEffectColor(int SlotIndex, const FVector4& Color)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetHitEffectColor(Color);
}

void CMeshComponent::SetHitEffect(int SlotIndex, bool Enable, float Intensity, const FVector4& Color)
{
	if (SlotIndex < 0 || SlotIndex >= mMaterialSlot.size())
	{
		return;
	}

	mMaterialSlot[SlotIndex]->SetHitEffect(Enable, Intensity, Color);
}

bool CMeshComponent::Init()
{
	CSceneComponent::Init();

	mCBufferTransform = std::make_shared<CCBufferTransform>();

	mCBufferTransform->Init();

	return true;
}

void CMeshComponent::Render()
{
	if (mShader.expired() || mMesh.expired())
	{
		return;
	}

	auto Mesh = mMesh.lock();
	auto Shader = mShader.lock();

	////뷰 행렬
	////카메라, 카메라 설정할때 카메라의 위쪽, 위치, 카메라가 바라보고있는곳(정면)을 설정한다.
	//FVector3 CameraPos(0.f, 0.f, -2.f);
	//FVector3 Lookat(0.f, 0.f, 0.f);
	//FVector3 CameraUp(0.f, 1.f, 0.f);

	//FMatrix ViewMat = DirectX::XMMatrixLookAtLH(CameraPos.Convert(), Lookat.Convert(), CameraUp.Convert());

	////투영행렬
	//FResolution RS = CDevice::GetInst()->GetResolution();

	////해상도의 절반만큼의 -를 좌로, 절반만큼을 우로 둔다.
	////2D환경인 직교투영으로 만들어준다.
	//FMatrix ProjMat = DirectX::XMMatrixOrthographicOffCenterLH((float)RS.Width / -2.f, (float)RS.Width / 2.f, (float)RS.Height / -2.f, (float)RS.Height / 2.f, 0.f, 1000.f);

	FMatrix ViewMat;
	FMatrix ProjMat;

	auto World = mWorld.lock();

	if (World)
	{
		auto CameraMgr = World->GetCameraManager().lock();

		if (CameraMgr)
		{
			ViewMat = CameraMgr->GetViewMat();
			ProjMat = CameraMgr->GetProjMat();
		}
	}

	mCBufferTransform->SetWorldMatrix(mWorldMat);
	mCBufferTransform->SetViewMatrix(ViewMat);
	mCBufferTransform->SetProjMatrix(ProjMat);

	mCBufferTransform->UpdateBuffer();

	Shader->SetShader();

	size_t Size = mMaterialSlot.size();

	for (size_t i = 0; i < Size; ++i)
	{
		//머티리얼 사이에 애니메이션이 있을경우 텍스처를 애니메이션 프레임을 이용해 변경해준다.
		if (!mAnimComp.expired())
		{
			auto Anim = mAnimComp.lock();

			Anim->SetShader();

			//타입이 None(-1)으로 나오는 경우가 있다. (시퀀스가 비었거나 애니메이션 에셋이 끊겼을 때)
			//예전에는 두 분기 어디에도 안 걸려서 SetMaterial 자체가 호출되지 않았고,
			//그러면 텍스처도 샘플러도 안 물린 채로 그려져서 화면에서 사라졌다.
			//이제는 스프라이트시트와 동일하게 처리해서 최소한 그림은 나오게 한다.
			if (Anim->GetTextureType() == EAnimation2DTextureType::Frame)
			{
				mMaterialSlot[i]->SetMaterial(Anim->GetAnimationFrame());
			}
			else
			{
				mMaterialSlot[i]->SetMaterial();
			}

		}
		else
		{
			mMaterialSlot[i]->SetMaterial();
		}

		Mesh->Render((int)i);

		mMaterialSlot[i]->ResetMaterial();
	}

	CSceneComponent::Render();
}

void CMeshComponent::Destroy()
{
	CSceneComponent::Destroy();
}

void CMeshComponent::Save(std::ofstream& File) const
{
	CSceneComponent::Save(File);

	//메쉬 이름은 "Mesh_" 접두사를 떼고 저장한다.
	//불러올 때 쓰는 FindMesh가 접두사를 스스로 붙이기 때문이다.
	auto Mesh = mMesh.lock();
	if (Mesh)
	{
		std::string MeshName = Mesh->GetName();
		static const std::string Prefix = "Mesh_";
		if (MeshName.size() > Prefix.size() && MeshName.compare(0, Prefix.size(), Prefix) == 0)
			MeshName = MeshName.substr(Prefix.size());

		File << "Mesh=" << MeshName << "\n";
	}

	auto Shader = mShader.lock();
	if (Shader) File << "Shader=" << Shader->GetName() << "\n";

	// 머티리얼 슬롯 0
	std::string MatName = GetMaterialName(0);
	if (!MatName.empty())
		File << "Material=" << MatName << "\n";

	if (!mMaterialSlot.empty() && mMaterialSlot[0])
	{
		const auto& Mat = mMaterialSlot[0];

		// 블렌드 스테이트
		auto RS = Mat->GetBlendState().lock();
		File << "Blend=" << (RS ? RS->GetName() : "(none)") << "\n";

		// 인스펙터에서 고친 머티리얼 값.
		// 슬롯의 머티리얼은 에셋의 복사본이라 여기 적어두지 않으면 되돌아간다.
		const FVector4& Color = Mat->GetBaseColor();
		File << "MatBaseColor=" << Color.x << " " << Color.y << " " << Color.z << " " << Color.w << "\n";
		File << "MatOpacity=" << Mat->GetOpacity() << "\n";
	}
}

void CMeshComponent::Load(const std::unordered_map<std::string, std::string>& Props)
{
	CSceneComponent::Load(Props);

	// 1) 메쉬 — 머티리얼 슬롯이 여기서 만들어지므로 제일 먼저 처리해야 한다.
	//    이게 실패하면 슬롯이 비어서 머티리얼/블렌드/색 전부 적용되지 않는다.
	{
		auto it = Props.find("Mesh");
		if (it != Props.end() && !it->second.empty())
			SetMesh(it->second);
	}

	// 2) 머티리얼 — 슬롯 0을 저장해 둔 머티리얼의 인스턴스로 교체한다.
	{
		auto it = Props.find("Material");
		if (it != Props.end() && !it->second.empty())
		{
			auto MatMgr = CAssetManager::GetInst()->GetSubManager<CMaterialManager>(EAssetType::Material);
			if (MatMgr)
			{
				auto NewMat = MatMgr->CreateMaterialInstance(it->second);
				if (NewMat)
					SetMaterialSlot(0, NewMat);
			}
		}
	}

	// 3) 셰이더 — 저장된 값이 최종이다.
	//    예전에는 머티리얼을 붙이면서 그 머티리얼의 셰이더로 덮어썼는데,
	//    그러면 Animation2D로 저장해 둔 게 Material로 되돌아가 스프라이트 시트가
	//    통째로 그려졌다. 파일에 셰이더가 없을 때만 머티리얼 쪽을 쓴다.
	{
		auto it = Props.find("Shader");
		if (it != Props.end() && !it->second.empty())
		{
			SetShader(it->second);
		}
		else if (auto Mat = GetMaterial(0))
		{
			const std::string& ShaderName = Mat->GetShaderName();
			if (!ShaderName.empty())
				SetShader(ShaderName);
		}
	}

	// 4) 블렌드 스테이트. "(none)"이면 머티리얼 에셋에 붙어 있던 것도 떼어낸다.
	{
		auto it = Props.find("Blend");
		if (it != Props.end() && !it->second.empty())
			SetBlendState(0, it->second == "(none)" ? "" : it->second);
	}

	// 5) 인스펙터에서 고쳐둔 머티리얼 값
	{
		auto it = Props.find("MatBaseColor");
		if (it != Props.end())
		{
			FVector4 Color(1.f, 1.f, 1.f, 1.f);
			sscanf_s(it->second.c_str(), "%f %f %f %f", &Color.x, &Color.y, &Color.z, &Color.w);
			SetBaseColor(0, Color);
		}
	}
	{
		auto it = Props.find("MatOpacity");
		if (it != Props.end())
		{
			try { SetOpacity(0, std::stof(it->second)); }
			catch (...) {}
		}
	}
}

std::string CMeshComponent::GetMaterialName(int SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= (int)mMaterialSlot.size()) return "";
	auto Mat = mMaterialSlot[SlotIndex];
	if (!Mat) return "";
	const std::string& N = Mat->GetName();
	if (N.size() > 9 && N[0] == 'M' && N[8] == '_')
		return N.substr(9);
	return N;
}

void CMeshComponent::SetMaterialSlot(int SlotIndex, std::shared_ptr<CMaterial> Mat)
{
	if (SlotIndex < 0 || SlotIndex >= (int)mMaterialSlot.size()) return;
	if (!Mat) return;
	mMaterialSlot[SlotIndex] = Mat;
}

std::string CMeshComponent::GetMeshName() const
{
	auto M = mMesh.lock(); return M ? M->GetName() : "";
}
std::string CMeshComponent::GetShaderName() const
{
	auto S = mShader.lock(); return S ? S->GetName() : "";
}
