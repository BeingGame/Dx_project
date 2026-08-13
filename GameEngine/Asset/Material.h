#pragma once

#include "Asset.h"
#include "CBufferContainer.h"

//구조체로 머티리얼 텍스처 정보를 설정한다.
struct FMaterialTextureInfo
{
	std::string Name;
	std::weak_ptr<class CTexture> Texture;

	//텍스처가 등록될 레지스터와 텍스처 인덱스
	int Register = 0;
	int ShaderBufferType = EShaderBufferType::Pixel;
	int Index = 0;
};

class CMaterial :
	public CAsset
{
public:
	CMaterial();
	CMaterial(const CMaterial& src);
	virtual ~CMaterial();

protected:
	std::vector<std::shared_ptr<FMaterialTextureInfo>> mTextureArray;
	FVector4 mBaseColor = FVector4::White;
	float mOpacity = 1.f;

	//머티리얼 상수버퍼
	std::shared_ptr<class CCBufferMaterial> mCBufferMaterial;

	//머티리얼이 사용할 픽셀셰이더
	std::weak_ptr<class CShader> mShader;
	//머티리얼이 사용할 샘플러이름
	std::string mSampler = "Linear";

	//머티리얼이 사용할 렌더스테이트
	std::weak_ptr<class CRenderState> mRenderState;

public:
	const FVector4& GetBaseColor() const
	{
		return mBaseColor;
	}

	float GetOpacity() const
	{
		return mOpacity;
	}

	std::string GetShaderName() const;

	const std::string& GetSamplerName() const { return mSampler; }

	std::string GetTextureName(int Index = 0) const
	{
		if (Index >= 0 && Index < (int)mTextureArray.size() && mTextureArray[Index])
			return mTextureArray[Index]->Name;
		return "";
	}

	std::weak_ptr<class CRenderState> GetBlendState() const
	{
		return mRenderState;
	}

	void SetBaseColor(const FVector4& Color);
	//float은 0~1 정규화된 값, int는 0~255로 표현된 값을 받아서 처리한다.
	void SetBaseColor(float r, float g, float b, float a);
	void SetBaseColor(int r, int g, int b, int a);
	void SetOpacity(float Opacity);

	//크로마키
	void SetChromaKeyEnable(bool Enable);
	void SetChromaKeyColor(const FVector3& Color);
	void SetChromaKeyThreshold(float Threshold);
	void SetChromaKey(bool Enable, const FVector3& Color = FVector3(0.f, 1.f, 0.f), float Threshold = 0.1f);

	//UV스크롤링
	void SetUVScrollEnable(bool Enable);
	void SetUVScrollSpeed(const FVector2& Speed);
	void SetUVScrollTime(float Time);
	void SetUVScroll(bool Enable, const FVector2& Speed, float Time);

	//디졸브
	void SetDissolveEnable(bool Enable);
	void SetDissolveEdgeColor(const FVector4& Color);
	void SetDissolveProgress(float Progress);
	void SetDissolveEdgeWidth(float Width);
	void SetDissolve(bool Enable, const FVector4& Color, float Progress, float Width);

	void SetHitEffectEnable(bool Enable);
	void SetHitIntensity(float Intensity);
	void SetHitEffectColor(const FVector4& Color);
	void SetHitEffect(bool Enable, float Intensity, const FVector4& Color);

	void SetPixelShader(const std::string& Name);
	void SetSampler(const std::string& Name);

	void SetRenderState(const std::weak_ptr<class CRenderState> State)
	{
		mRenderState = State;
	}
	void SetRenderState(const std::string& Name);

public:
	void AddTexture(const std::weak_ptr<class CTexture>& Texture, int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel, int Index = 0);
	void AddTexture(const std::string& Name, int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel, int Index = 0);
	void AddTexture(const std::string& Name, const TCHAR* FileName, const std::string& PathName = "Texture", int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel, int Index = 0);
	void AddTextureFullPath(const std::string& Name, const TCHAR* FullPath, int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel, int Index = 0);
	void AddTexture(const std::string& Name, std::vector<const TCHAR*> FileName, const std::string& PathName = "Texture", int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel, int Index = 0);
	void AddTextureFullPath(const std::string& Name, std::vector<const TCHAR*> FullPath, int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel, int Index = 0);

	bool SetTexture(int TextureIndex, const std::weak_ptr<class CTexture>& Texture);

public:
	bool Init(const std::string& PixelShader = "Material", const std::string& SamplerName = "Linear");
	void SetMaterial();
	void SetMaterial(int TextureIndex);
	void ResetMaterial();

public:
	virtual CMaterial* Clone()
	{
		return new CMaterial(*this);
	}
};