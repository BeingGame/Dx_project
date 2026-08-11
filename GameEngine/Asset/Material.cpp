#include "Material.h"

#include "AssetManager.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include "Texture.h"
#include "CBufferMaterial.h"

#include "../RenderManager.h"
#include "../RenderState.h"

CMaterial::CMaterial()
{
	mType = EAssetType::Material;
}

CMaterial::CMaterial(const CMaterial& src)
{
	mShader = src.mShader;
	mBaseColor = src.mBaseColor;
	mOpacity = src.mOpacity;
	mSampler = src.mSampler;

	mCBufferMaterial = std::make_shared<CCBufferMaterial>();
	mCBufferMaterial->Init();

	mCBufferMaterial->SetColor(mBaseColor);
	mCBufferMaterial->SetOpacity(mOpacity);

	//텍스처를 업데이트한다.

	size_t Size = src.mTextureArray.size();

	for (size_t i = 0; i < Size; ++i)
	{
		std::shared_ptr<FMaterialTextureInfo> NewInfo = std::make_shared<FMaterialTextureInfo>();

		*NewInfo = *src.mTextureArray[i];
	}

	mRenderState = src.mRenderState;
}

CMaterial::~CMaterial()
{}

void CMaterial::SetBaseColor(const FVector4& Color)
{
	mBaseColor = Color;
	mCBufferMaterial->SetColor(Color);
}

void CMaterial::SetBaseColor(float r, float g, float b, float a)
{
	SetBaseColor(FVector4(r, g, b, a));
}

void CMaterial::SetBaseColor(int r, int g, int b, int a)
{
	SetBaseColor(FVector4(r / 255.f, g / 255.f, b / 255.f, a / 255.f));
}

void CMaterial::SetOpacity(float Opacity)
{
	mOpacity = Opacity;
	mCBufferMaterial->SetOpacity(Opacity);
}

void CMaterial::SetChromaKeyEnable(bool Enable)
{
	mCBufferMaterial->SetChromaKeyEnable(Enable);
}

void CMaterial::SetChromaKeyColor(const FVector3& Color)
{
	mCBufferMaterial->SetChromaKeyColor(Color);
}

void CMaterial::SetChromaKeyThreshold(float Threshold)
{
	mCBufferMaterial->SetChromaKeyThreshold(Threshold);
}

void CMaterial::SetChromaKey(bool Enable, const FVector3& Color, float Threshold)
{
	mCBufferMaterial->SetChromaKey(Enable, Color, Threshold);
}

void CMaterial::SetUVScrollEnable(bool Enable)
{
	mCBufferMaterial->SetUVScrollEnable(Enable);
}

void CMaterial::SetUVScrollSpeed(const FVector2& Speed)
{
	mCBufferMaterial->SetUVScrollSpeed(Speed);

}

void CMaterial::SetUVScrollTime(float Time)
{
	mCBufferMaterial->SetUVScrollTime(Time);

}

void CMaterial::SetUVScroll(bool Enable, const FVector2& Speed = FVector2(0.f, 0.f), float Time = 0.1f)
{
	mCBufferMaterial->SetUVScroll(Enable, Speed, Time);
}

void CMaterial::SetDissolveEnable(bool Enable)
{
	mCBufferMaterial->SetDissolveEnable(Enable);
}

void CMaterial::SetDissolveEdgeColor(const FVector4& Color)
{
	mCBufferMaterial->SetDissolveEdgeColor(Color);
}

void CMaterial::SetDissolveProgress(float Progress)
{
	mCBufferMaterial->SetDissolveProgress(Progress);
}

void CMaterial::SetDissolveEdgeWidth(float Width)
{
	mCBufferMaterial->SetDissolveEdgeWidth(Width);
}

void CMaterial::SetDissolve(bool Enable, const FVector4& Color = FVector4::Red, float Progress = 0.f, float Width = 0.04f)
{
	mCBufferMaterial->SetDissolve(Enable, Color, Progress, Width);
}

void CMaterial::SetHitEffectEnable(bool Enable)
{
	mCBufferMaterial->SetHitEffectEnable(Enable);
}

void CMaterial::SetHitIntensity(float Intensity)
{
	mCBufferMaterial->SetHitIntensity(Intensity);
}

void CMaterial::SetHitEffectColor(const FVector4& Color)
{
	mCBufferMaterial->SetHitEffectColor(Color);
}

void CMaterial::SetHitEffect(bool Enable, float Intensity, const FVector4& Color)
{
	mCBufferMaterial->SetHitEffect(Enable, Intensity, Color);
}

void CMaterial::SetPixelShader(const std::string& Name)
{
	//셰이더를 가져와준다.
	auto ShaderMgr = CAssetManager::GetInst()->GetSubManager<CShaderManager>(EAssetType::Shader);

	if (ShaderMgr)
	{
		mShader = ShaderMgr->FindShader(Name);
	}
}

void CMaterial::SetSampler(const std::string& Name)
{
	//샘플러는 이름만 가지고 있어서 셰이더매니저에서 호출하도록 만들어준다.
	mSampler = Name;
}

void CMaterial::SetRenderState(const std::string& Name)
{
	mRenderState = CRenderManager::GetInst()->FindRenderState(Name);
}

void CMaterial::AddTexture(const std::weak_ptr<class CTexture>& Texture, int Register, int ShaderBufferType, int Index)
{
	if (Texture.expired())
	{
		return;
	}

	std::shared_ptr<FMaterialTextureInfo> NewInfo = std::make_shared<FMaterialTextureInfo>();

	std::shared_ptr<CTexture> _Texture = Texture.lock();

	NewInfo->Texture = _Texture;
	NewInfo->Name = _Texture->GetName();
	NewInfo->Register = Register;
	NewInfo->ShaderBufferType = ShaderBufferType;
	NewInfo->Index = Index;

	mTextureArray.push_back(NewInfo);
}

void CMaterial::AddTexture(const std::string& Name, int Register, int ShaderBufferType, int Index)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (TextureMgr)
	{
		auto Texture = TextureMgr->FindTexture(Name);

		if (Texture.expired())
		{
			return;
		}
		std::shared_ptr<FMaterialTextureInfo> NewInfo = std::make_shared<FMaterialTextureInfo>();

		std::shared_ptr<CTexture> _Texture = Texture.lock();

		NewInfo->Texture = _Texture;
		NewInfo->Name = _Texture->GetName();
		NewInfo->Register = Register;
		NewInfo->ShaderBufferType = ShaderBufferType;
		NewInfo->Index = Index;

		mTextureArray.push_back(NewInfo);
	}
}

void CMaterial::AddTexture(const std::string& Name, const TCHAR* FileName, const std::string& PathName, int Register, int ShaderBufferType, int Index)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (TextureMgr)
	{
		if (!TextureMgr->LoadTexture(Name, FileName, PathName))
		{
			return;
		}

		auto Texture = TextureMgr->FindTexture(Name);

		if (Texture.expired())
		{
			return;
		}

		std::shared_ptr<FMaterialTextureInfo> NewInfo = std::make_shared<FMaterialTextureInfo>();

		std::shared_ptr<CTexture> _Texture = Texture.lock();

		NewInfo->Texture = _Texture;
		NewInfo->Name = _Texture->GetName();
		NewInfo->Register = Register;
		NewInfo->ShaderBufferType = ShaderBufferType;
		NewInfo->Index = Index;

		mTextureArray.push_back(NewInfo);
	}
}

void CMaterial::AddTextureFullPath(const std::string& Name, const TCHAR* FullPath, int Register, int ShaderBufferType, int Index)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (TextureMgr)
	{
		if (!TextureMgr->LoadTexture(Name, FullPath))
		{
			return;
		}

		auto Texture = TextureMgr->FindTexture(Name);

		if (Texture.expired())
		{
			return;
		}

		std::shared_ptr<FMaterialTextureInfo> NewInfo = std::make_shared<FMaterialTextureInfo>();

		std::shared_ptr<CTexture> _Texture = Texture.lock();

		NewInfo->Texture = _Texture;
		NewInfo->Name = _Texture->GetName();
		NewInfo->Register = Register;
		NewInfo->ShaderBufferType = ShaderBufferType;
		NewInfo->Index = Index;

		mTextureArray.push_back(NewInfo);
	}
}

void CMaterial::AddTexture(const std::string& Name, std::vector<const TCHAR*> FileName, const std::string& PathName, int Register, int ShaderBufferType, int Index)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (TextureMgr)
	{
		if (!TextureMgr->LoadTexture(Name, FileName, PathName))
		{
			return;
		}

		auto Texture = TextureMgr->FindTexture(Name);

		if (Texture.expired())
		{
			return;
		}

		std::shared_ptr<FMaterialTextureInfo> NewInfo = std::make_shared<FMaterialTextureInfo>();

		std::shared_ptr<CTexture> _Texture = Texture.lock();

		NewInfo->Texture = _Texture;
		NewInfo->Name = _Texture->GetName();
		NewInfo->Register = Register;
		NewInfo->ShaderBufferType = ShaderBufferType;
		NewInfo->Index = Index;

		mTextureArray.push_back(NewInfo);
	}
}

void CMaterial::AddTextureFullPath(const std::string& Name, std::vector<const TCHAR*> FullPath, int Register, int ShaderBufferType, int Index)
{
	auto TextureMgr = CAssetManager::GetInst()->GetSubManager<CTextureManager>(EAssetType::Texture);

	if (TextureMgr)
	{
		if (!TextureMgr->LoadTexture(Name, FullPath))
		{
			return;
		}

		auto Texture = TextureMgr->FindTexture(Name);

		if (Texture.expired())
		{
			return;
		}

		std::shared_ptr<FMaterialTextureInfo> NewInfo = std::make_shared<FMaterialTextureInfo>();

		std::shared_ptr<CTexture> _Texture = Texture.lock();

		NewInfo->Texture = _Texture;
		NewInfo->Name = _Texture->GetName();
		NewInfo->Register = Register;
		NewInfo->ShaderBufferType = ShaderBufferType;
		NewInfo->Index = Index;

		mTextureArray.push_back(NewInfo);
	}
}

bool CMaterial::SetTexture(int TextureIndex, const std::weak_ptr<class CTexture>& Texture)
{
	//텍스처를 새로받을때 인덱스가 사이즈랑 동일하면 새로운 텍스처 정보를 만들어준다.
	if (TextureIndex == mTextureArray.size())
	{
		std::shared_ptr<FMaterialTextureInfo> NewInfo = std::make_shared<FMaterialTextureInfo>();

		auto _Texture = Texture.lock();

		NewInfo->Texture = _Texture;
		NewInfo->Name = _Texture->GetName();
		NewInfo->Register = 0;
		NewInfo->ShaderBufferType = EShaderBufferType::Pixel;
		NewInfo->Index = 0;

		mTextureArray.push_back(NewInfo);

		return true;
	}
	else if (TextureIndex < 0 || TextureIndex >mTextureArray.size())
	{
		return false;
	}

	mTextureArray[TextureIndex]->Texture = Texture;

	return true;
}

bool CMaterial::Init(const std::string& PixelShader, const std::string& SamplerName)
{
	mCBufferMaterial = std::make_shared<CCBufferMaterial>();

	mCBufferMaterial->Init();

	SetBaseColor(mBaseColor);
	SetOpacity(mOpacity);

	SetPixelShader(PixelShader);
	SetSampler(SamplerName);


	return true;
}

void CMaterial::SetMaterial()
{
	//머티리얼 사용시에 셰이더와 상수버퍼를 레지스터에 등록한다.
	auto Shader = mShader.lock();

	if (Shader)
	{
		Shader->SetShader();
	}

	mCBufferMaterial->UpdateBuffer();

	//머티리얼에 등록된 텍스처들을 레지스터에 등록한다.
	size_t Size = mTextureArray.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Texture = mTextureArray[i]->Texture.lock();

		if (Texture)
		{
			Texture->SetShader(mTextureArray[i]->Register, mTextureArray[i]->ShaderBufferType, mTextureArray[i]->Index);
		}
	}

	//샘플러를 업데이트한다.
	auto ShaderMgr = CAssetManager::GetInst()->GetSubManager<CShaderManager>(EAssetType::Shader);

	if (ShaderMgr)
	{
		ShaderMgr->SetSampler(mSampler, 0);
	}

	//렌더스테이트를 설정해준다.
	auto State = mRenderState.lock();

	if (State)
	{
		State->SetState();
	}
}

void CMaterial::SetMaterial(int TextureIndex)
{
	//머티리얼 사용시에 셰이더와 상수버퍼를 레지스터에 등록한다.
	auto Shader = mShader.lock();

	if (Shader)
	{
		Shader->SetShader();
	}

	mCBufferMaterial->UpdateBuffer();

	//머티리얼에 등록된 텍스처들을 레지스터에 등록한다.
	size_t Size = mTextureArray.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Texture = mTextureArray[i]->Texture.lock();

		if (Texture)
		{
			Texture->SetShader(mTextureArray[i]->Register, mTextureArray[i]->ShaderBufferType, TextureIndex);
		}
	}

	//샘플러를 업데이트한다.
	auto ShaderMgr = CAssetManager::GetInst()->GetSubManager<CShaderManager>(EAssetType::Shader);

	if (ShaderMgr)
	{
		ShaderMgr->SetSampler(mSampler, 0);
	}

	//렌더스테이트를 설정해준다.
	auto State = mRenderState.lock();

	if (State)
	{
		State->SetState();
	}
}

void CMaterial::ResetMaterial()
{
	//사용한 텍스처를 회수한다.
	size_t Size = mTextureArray.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Texture = mTextureArray[i]->Texture.lock();

		if (Texture)
		{
			Texture->ResetShader(mTextureArray[i]->Register, mTextureArray[i]->ShaderBufferType);
		}
	}
	auto State = mRenderState.lock();

	if (State)
	{
		State->ResetState();
	}
}
