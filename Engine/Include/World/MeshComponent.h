#pragma once

#include "SceneComponent.h"
#include "../Asset/CBufferContainer.h"

class CMeshComponent :
	public CSceneComponent
{
public:
	CMeshComponent();
	CMeshComponent(const CMeshComponent& src);
	CMeshComponent(CMeshComponent&& src) noexcept;
	virtual ~CMeshComponent();

protected:
	//메쉬 렌더링을 위한 메쉬와 셰이더
	std::weak_ptr<class CMesh> mMesh;
	std::weak_ptr<class CShader> mShader;
	std::shared_ptr<class CCBufferTransform> mCBufferTransform;

	//메쉬컴포넌트가 사용할 머티리얼
	std::vector<std::shared_ptr<class CMaterial>> mMaterialSlot;

	//애니메이션 컴포넌트
	std::weak_ptr<class CAnimation2DComponent> mAnimComp;

public:
	void SetAnimComp(const std::weak_ptr<class CAnimation2DComponent>& AnimComp)
	{
		mAnimComp = AnimComp;
	}

	void SetMesh(const std::weak_ptr<class CMesh> Mesh);
	void SetMesh(const std::string& Name);

	void SetShader(const std::weak_ptr<class CShader> Shader);
	void SetShader(const std::string& Name);

	void SetBlendState(int SlotIndex, const std::string& Name);

	void AddTexture(int SlotIndex, const std::weak_ptr<class CTexture>& Texture, int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel, int Index = 0);
	void AddTexture(int SlotIndex, const std::string& Name, int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel, int Index = 0);
	void AddTexture(int SlotIndex, const std::string& Name, const TCHAR* FileName, const std::string& PathName = "Texture", int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel, int Index = 0);
	void AddTextureFullPath(int SlotIndex, const std::string& Name, const TCHAR* FullPath, int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel, int Index = 0);
	void AddTexture(int SlotIndex, const std::string& Name, std::vector<const TCHAR*> FileName, const std::string& PathName = "Texture", int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel, int Index = 0);
	void AddTextureFullPath(int SlotIndex, const std::string& Name, std::vector<const TCHAR*> FullPath, int Register = 0, int ShaderBufferType = EShaderBufferType::Pixel, int Index = 0);

	bool SetTexture(int SlotIndex, int TextureIndex, const std::weak_ptr<class CTexture>& Texture);

	void SetBaseColor(int SlotIndex, const FVector4& Color);
	//float은 0~1 정규화된 값, int는 0~255로 표현된 값을 받아서 처리한다.
	void SetBaseColor(int SlotIndex, float r, float g, float b, float a);
	void SetBaseColor(int SlotIndex, int r, int g, int b, int a);
	void SetOpacity(int SlotIndex, float Opacity);

	void SetChromaKeyEnable(int SlotIndex, bool Enable);
	void SetChromaKeyColor(int SlotIndex, const FVector3& Color);
	void SetChromaKeyThreshold(int SlotIndex, float Threshold);
	void SetChromaKey(int SlotIndex, bool Enable, const FVector3& Color = FVector3(0.f, 1.f, 0.f), float Threshold = 0.1f);

	void SetUVScrollEnable(int SlotIndex, bool Enable);
	void SetUVScrollSpeed(int SlotIndex, const FVector2& Speed);
	void SetUVScrollTime(int SlotIndex, float Time);
	void SetUVScroll(int SlotIndex, bool Enable, const FVector2& Speed = FVector2(0.f, 0.f), float Time = 0.f);

	void SetDissolveEnable(int SlotIndex, bool Enable);
	void SetDissolveEdgeColor(int SlotIndex, const FVector4& Color);
	void SetDissolveProgress(int SlotIndex, float Progress);
	void SetDissolveEdgeWidth(int SlotIndex, float Width);
	void SetDissolve(int SlotIndex, bool Enable, const FVector4& Color, float Progress, float Width);

	void SetHitEffectEnable(int SlotIndex, bool Enable);
	void SetHitIntensity(int SlotIndex, float Intensity);
	void SetHitEffectColor(int SlotIndex, const FVector4& Color);
	void SetHitEffect(int SlotIndex, bool Enable, float Intensity, const FVector4& Color);


public:
	virtual bool Init();
	virtual void Render();
	virtual void Destroy();
};

