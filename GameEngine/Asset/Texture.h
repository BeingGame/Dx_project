#pragma once

#include "Asset.h"
#include "DirectXTex.h"
#include "CBufferContainer.h"

//텍스처 라이브러리를 선언해준다.

#ifdef _DEBUG

#pragma comment(lib, "DirectXTex_Debug.lib")

#else

#pragma comment(lib, "DirectXTex.lib")

#endif

//텍스처 정보 구조체
struct FTextureInfo
{
	//텍스처의 Pixel 정보를 저장할 객체
	DirectX::ScratchImage* Image = nullptr;

	//텍스처를 셰이더에서 사용할 리소스로 레지스터에 보내줄 SRV객체
	ID3D11ShaderResourceView* SRV = nullptr;

	//이미지 크기
	unsigned int Width = 0;
	unsigned int Height = 0;

	//파일 경로
	std::wstring FullPath;

	~FTextureInfo()
	{
		SAFE_DELETE(Image);
		if (SRV)
		{
			SRV->Release();
		}
	}

};

//텍스처 구성
//텍스처는 동시에 여러장을 들고 있을수도 있다.

class CTexture :
	public CAsset
{
public:
	CTexture();
	virtual ~CTexture();

protected:
	//애니메이션에서 여러장 사용할수 있도록 만들어준다.
	std::vector<FTextureInfo*> mTextureList;

	//스프라이트 시트에서 사용할 팔레트 컬러 스왑에 사용할 SRV
	ID3D11Texture2D* mIndexTexture = nullptr;
	ID3D11ShaderResourceView* mIndexSRV = nullptr;

	std::vector<std::vector<FVector4>> mPaletteColor;
	std::map<uint32_t, int> mSheetColor;
	std::vector<uint8_t> mPixelIndex;

public:
	const FTextureInfo* GetTexture(int Index = 0) const
	{
		if (Index < 0 || Index >= mTextureList.size())
		{
			return nullptr;
		}

		return mTextureList[Index];
	}

	int GetTextureCount() const
	{
		return (int)mTextureList.size();
	}

	std::vector<FVector4> GetPaletteColor(int Index) const
	{
		if (Index < 0 || Index >= mPaletteColor.size())
		{
			static std::vector<FVector4> dummy;
			return dummy;
		}

		return mPaletteColor[Index];
	}

	size_t GetPaletteSize() const
	{
		return mPaletteColor.size();
	}

	std::vector<std::vector<FVector4>> GetPaletteColorAll() const
	{
		return mPaletteColor;
	}


public:
	//실제로 텍스처를 불러올 함수
	bool LoadTexture(const TCHAR* FileName, const std::string& PathName);
	bool LoadTexture(const TCHAR* FullPath);
	bool LoadTexture(const std::vector<const TCHAR*>& FileName, const std::string& PathName);
	bool LoadTexture(const std::vector<const TCHAR*>& FullPath);
	void SetShader(int Register, int ShaderBufferType = EShaderBufferType::Pixel, int TextureIndex = 0);
	void ResetShader(int Register, int ShaderBufferType = EShaderBufferType::Pixel);

	bool CreatePaletteTexture();
	void MakePaletteIndex();
	void MakePaletteColor(const FVector2& OriginPos, const FVector2& TargetPos, const FVector2& FrameSize);

public:
	bool CreateResourceView(int Index = 0);

public:
	void Save(std::ofstream& File);

};

