#include "Texture.h"
#include "PathManager.h"
#include "../Device.h"
#include <fstream>

CTexture::CTexture()
{
	mType = EAssetType::Texture;
}

CTexture::~CTexture()
{
	size_t Size = mTextureList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		SAFE_DELETE(mTextureList[i]);
	}

	if (mIndexTexture)
	{
		mIndexTexture->Release();
	}

	if (mIndexSRV)
	{
		mIndexSRV->Release();
	}


}

bool CTexture::LoadTexture(const TCHAR* FileName, const std::string& PathName)
{
	//FullPath를 만들어준다.

	const TCHAR* BasePath = CPathManager::FindPath(PathName);

	TCHAR FullPath[MAX_PATH] = {};

	lstrcpy(FullPath, BasePath);
	lstrcat(FullPath, FileName);

	return LoadTexture(FullPath);
}

bool CTexture::LoadTexture(const TCHAR* FullPath)
{
	//먼저 텍스처 인포를 생성한다.
	FTextureInfo* Texture = new FTextureInfo;

	//텍스처 리소스의 확장자에 따라 텍스처를 로드 해주는 함수가 달라진다.
	//1. .dds 2. .tga 3. 그외

	//전체 경로에서 파일에 대한 확장자를 가져온다.

	//파일 확장자를 저장할 문자열
	TCHAR FileExt[_MAX_EXT] = {};

	//파일 확장자를 Path에서 분리한다.
	//ex) .png
	//파일 확장자를 분리하고, 대문자 처리해서 일원화 시켜줘야한다.

	//_wsplitpath_s 함수 : 받아온 문자열을 4가지로 구분해서 문자열을 잘라준다.
	//드라이브, 디렉토리 경로, 파일이름, 파일 확장자 4가지로 구분한다.
	//함수 사용할땐 받아올 문자열과, 문자열의 길이를 지정해주면 함수가 처리된다.
	_wsplitpath_s(FullPath, nullptr, 0, nullptr, 0, nullptr, 0, FileExt, _MAX_EXT);

	//wstring으로 반환된 유니코드 문자열을 멀티바이트 문자열로 변환한다.
	char ConvertExt[_MAX_EXT] = {};

	int Count = WideCharToMultiByte(CP_ACP, 0, FileExt, -1, nullptr, 0, nullptr, nullptr);
	WideCharToMultiByte(CP_ACP, 0, FileExt, -1, ConvertExt, Count, nullptr, nullptr);

	//확장자의 문자를 대문자로 만들어준다.
	_strupr_s(ConvertExt);

	//픽셀 정보를 위한 객체를 생성해서, 리소스 파일을 로드해 객체 데이터에 저장한다.
	DirectX::ScratchImage* Image = new DirectX::ScratchImage;

	//파일 확장자를 체크해서 필요한 함수를 호출해준다.
	if (strcmp(ConvertExt, ".DDS") == 0)
	{
		if (FAILED(DirectX::LoadFromDDSFile(FullPath, DirectX::DDS_FLAGS_NONE, nullptr, *Image)))
		{
			SAFE_DELETE(Texture);
			SAFE_DELETE(Image);

			return false;
		}
	}
	else if (strcmp(ConvertExt, ".TGA") == 0)
	{
		if (FAILED(DirectX::LoadFromTGAFile(FullPath, DirectX::TGA_FLAGS_NONE, nullptr, *Image)))
		{
			SAFE_DELETE(Texture);
			SAFE_DELETE(Image);

			return false;
		}
	}
	else
	{
		if (FAILED(DirectX::LoadFromWICFile(FullPath, DirectX::WIC_FLAGS_NONE, nullptr, *Image)))
		{
			SAFE_DELETE(Texture);
			SAFE_DELETE(Image);

			return false;
		}
	}

	//텍스처를 RGBA 포맷으로 강제 변환
	//DXGI_FORMAT_R8G8B8A8_UNORM 32비트 포맷이 아니라면 변환해준다.
	if (Image->GetMetadata().format != DXGI_FORMAT_R8G8B8A8_UNORM)
	{
		DirectX::ScratchImage ConvertedImage;

		if (SUCCEEDED(DirectX::Convert(Image->GetImages(), Image->GetImageCount(), Image->GetMetadata(), DXGI_FORMAT_R8G8B8A8_UNORM, DirectX::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, ConvertedImage)))
		{
			*Image = std::move(ConvertedImage);
		}
	}

	Texture->Image = Image;
	Texture->FullPath = FullPath;

	mTextureList.push_back(Texture);

	//텍스처가 정상적으로 로드되면 SRV를 생성한다.
	return CreateResourceView((int)mTextureList.size() - 1);
}

bool CTexture::LoadTexture(const std::vector<const TCHAR*>& FileName, const std::string& PathName)
{
	size_t Size = FileName.size();

	for (size_t i = 0; i < Size; ++i)
	{
		if (!LoadTexture(FileName[i], PathName))
		{
			return false;
		}
	}

	return true;
}

bool CTexture::LoadTexture(const std::vector<const TCHAR*>& FullPath)
{
	size_t Size = FullPath.size();

	for (size_t i = 0; i < Size; ++i)
	{
		if (!LoadTexture(FullPath[i]))
		{
			return false;
		}
	}

	return true;
}

void CTexture::SetShader(int Register, int ShaderBufferType, int TextureIndex)
{
	if (TextureIndex < 0 || TextureIndex >= mTextureList.size())
	{
		return;
	}

	ID3D11ShaderResourceView* SRV = mTextureList[TextureIndex]->SRV;

	if (!SRV)
	{
		return;
	}

	//SRV가 있다면 텍스처를 레지스터에 등록한다.

	if (ShaderBufferType & EShaderBufferType::Vertex)
	{
		CDevice::GetInst()->GetContext()->VSSetShaderResources(Register, 1, &SRV);
	}
	if (ShaderBufferType & EShaderBufferType::Pixel)
	{
		CDevice::GetInst()->GetContext()->PSSetShaderResources(Register, 1, &SRV);
	}
	if (ShaderBufferType & EShaderBufferType::Domain)
	{
		CDevice::GetInst()->GetContext()->DSSetShaderResources(Register, 1, &SRV);
	}
	if (ShaderBufferType & EShaderBufferType::Hull)
	{
		CDevice::GetInst()->GetContext()->HSSetShaderResources(Register, 1, &SRV);
	}
	if (ShaderBufferType & EShaderBufferType::Geometry)
	{
		CDevice::GetInst()->GetContext()->GSSetShaderResources(Register, 1, &SRV);
	}
	if (ShaderBufferType & EShaderBufferType::Compute)
	{
		CDevice::GetInst()->GetContext()->CSSetShaderResources(Register, 1, &SRV);
	}

	if (mIndexSRV)
	{
		CDevice::GetInst()->GetContext()->PSSetShaderResources(2, 1, &mIndexSRV);
	}

}

void CTexture::ResetShader(int Register, int ShaderBufferType)
{
	ID3D11ShaderResourceView* SRV = nullptr;

	if (ShaderBufferType & EShaderBufferType::Vertex)
	{
		CDevice::GetInst()->GetContext()->VSSetShaderResources(Register, 1, &SRV);
	}
	if (ShaderBufferType & EShaderBufferType::Pixel)
	{
		CDevice::GetInst()->GetContext()->PSSetShaderResources(Register, 1, &SRV);
	}
	if (ShaderBufferType & EShaderBufferType::Domain)
	{
		CDevice::GetInst()->GetContext()->DSSetShaderResources(Register, 1, &SRV);
	}
	if (ShaderBufferType & EShaderBufferType::Hull)
	{
		CDevice::GetInst()->GetContext()->HSSetShaderResources(Register, 1, &SRV);
	}
	if (ShaderBufferType & EShaderBufferType::Geometry)
	{
		CDevice::GetInst()->GetContext()->GSSetShaderResources(Register, 1, &SRV);
	}
	if (ShaderBufferType & EShaderBufferType::Compute)
	{
		CDevice::GetInst()->GetContext()->CSSetShaderResources(Register, 1, &SRV);
	}
}

bool CTexture::CreatePaletteTexture()
{
	if (mTextureList.empty())
	{
		return false;
	}
	else if (mIndexSRV)
	{
		return true;
	}

	MakePaletteIndex();

	D3D11_TEXTURE2D_DESC Desc = {};

	Desc.Height = mTextureList[0]->Height;
	Desc.Width = mTextureList[0]->Width;
	Desc.MipLevels = 1;
	Desc.ArraySize = 1;
	Desc.SampleDesc.Count = 1;
	Desc.Format = DXGI_FORMAT_R8_UINT;
	Desc.Usage = D3D11_USAGE_IMMUTABLE;
	Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA Data = {};

	Data.pSysMem = mPixelIndex.data();
	Data.SysMemPitch = Desc.Width;

	if (FAILED(CDevice::GetInst()->GetDevice()->CreateTexture2D(&Desc, &Data, &mIndexTexture)))
	{
		return false;
	}

	if (FAILED(CDevice::GetInst()->GetDevice()->CreateShaderResourceView(mIndexTexture, nullptr, &mIndexSRV)))
	{
		return false;
	}


	return true;
}

void CTexture::MakePaletteIndex()
{
	//실제로 텍스처에 넣어줄 인덱스 번호랑 기본이 될 컬러를 만들어준다.
	const DirectX::Image Image = mTextureList[0]->Image->GetImages()[0];
	const DirectX::TexMetadata MetaData = mTextureList[0]->Image->GetMetadata();

	//픽셀당 비트 수: bpp
	//bpp구해서 현재 픽셀 1개가 몇 바이트인지 계산한다.
	//format = DXGI_FORMAT_R8G8B8A8_UNORM -> 32비트 -> 4바이트
	size_t bpp = DirectX::BitsPerPixel(MetaData.format);
	size_t Byte = bpp / 8;

	//텍스처의 픽셀 데이터가 저장된 메모리 시작 주소를 받아온다.
	uint8_t* Pixels = Image.pixels;

	size_t Height = Image.height;
	size_t Width = Image.width;

	//먼저 시트의 전체 픽셀 수만큼 팔레트 인덱스를 저장할 벡터를 미리 사이즈 적용
	mPixelIndex.reserve(Height * Width);

	//색상마다 순차적으로 번호를 지정할 변수
	int Index = 0;


	//순차적으로 모든 픽셀을 순회해서 색상을 추출하고 인덱스번호를 넣어준다.
	for (size_t i = 0; i < Height; ++i)
	{
		//rowPitch : 한 줄의 바이트 크기
		//i번째 행의 메모리 시작주소를 계산한다.
		uint8_t* Pixel = Pixels + (i * Image.rowPitch);

		for (size_t j = 0; j < Width; ++j)
		{
			uint32_t Color = 0;

			//픽셀의 4바이트(32비트) 데이터를 32비트 정수로 변환한다.
			Color |= Pixel[(j * Byte)]; //r
			Color = Color << 8;
			Color |= Pixel[(j * Byte) + 1]; //g
			Color = Color << 8;
			Color |= Pixel[(j * Byte) + 2]; //b
			Color = Color << 8;
			Color |= Pixel[(j * Byte) + 3]; //a

			auto Result = mSheetColor.insert(std::make_pair(Color, Index));

			if (Result.second)
			{
				++Index;
			}

			mPixelIndex.push_back((uint8_t)mSheetColor.find(Color)->second);

		}
	}

	std::vector<FVector4> ColorMappingArray;

	auto iter = mSheetColor.begin();
	auto iterEnd = mSheetColor.end();

	ColorMappingArray.resize(256);

	for (; iter != iterEnd; ++iter)
	{
		uint32_t Color = iter->first;

		unsigned int r = 0, g = 0, b = 0, a = 0;

		r = (Color >> 24) & 0xFF;
		g = (Color >> 16) & 0xFF;
		b = (Color >> 8) & 0xFF;
		a = Color & 0xFF;

		FVector4 CalcColor(r / 255.f, g / 255.f, b / 255.f, a / 255.f);

		//SheetColor가 가지고있는 인덱스 번호를 MappingArray의 인덱스 위치로 사용해서
		//컬러를 매핑해준다.
		ColorMappingArray[iter->second] = CalcColor;
	}

	//원본 컬러 팔레트를 넣어준다.
	mPaletteColor.push_back(ColorMappingArray);
}

void CTexture::MakePaletteColor(const FVector2& OriginPos, const FVector2& TargetPos, const FVector2& FrameSize)
{
	const DirectX::Image Image = mTextureList[0]->Image->GetImages()[0];
	const DirectX::TexMetadata MetaData = mTextureList[0]->Image->GetMetadata();

	//픽셀당 비트 수: bpp
	//bpp구해서 현재 픽셀 1개가 몇 바이트인지 계산한다.
	//format = DXGI_FORMAT_R8G8B8A8_UNORM -> 32비트 -> 4바이트
	size_t bpp = DirectX::BitsPerPixel(MetaData.format);
	size_t Byte = bpp / 8;

	//텍스처의 픽셀 데이터가 저장된 메모리 시작 주소를 받아온다.
	uint8_t* Pixels = Image.pixels;

	std::map<uint32_t, int> ColorMap;

	for (size_t y = 0; y < (size_t)FrameSize.y; ++y)
	{
		//원본의 행위치를 구해준다.
		uint8_t* OriginPixel = Pixels + (((size_t)OriginPos.y + y) * Image.rowPitch);
		//타겟의 행위치를 구해준다.
		uint8_t* TargetPixel = Pixels + (((size_t)TargetPos.y + y) * Image.rowPitch);

		for (size_t x = 0; x < (size_t)FrameSize.x; ++x)
		{
			size_t OriginX = (size_t)OriginPos.x + x;
			size_t TargetX = (size_t)TargetPos.x + x;

			uint32_t Color = 0;
			Color |= OriginPixel[(OriginX * Byte)]; //r
			Color = Color << 8;
			Color |= OriginPixel[(OriginX * Byte) + 1]; //g
			Color = Color << 8;
			Color |= OriginPixel[(OriginX * Byte) + 2]; //b
			Color = Color << 8;
			Color |= OriginPixel[(OriginX * Byte) + 3]; //a

			uint32_t TargetColor = 0;
			TargetColor |= TargetPixel[(TargetX * Byte)]; //r
			TargetColor = TargetColor << 8;
			TargetColor |= TargetPixel[(TargetX * Byte) + 1]; //g
			TargetColor = TargetColor << 8;
			TargetColor |= TargetPixel[(TargetX * Byte) + 2]; //b
			TargetColor = TargetColor << 8;
			TargetColor |= TargetPixel[(TargetX * Byte) + 3]; //a

			auto iter = mSheetColor.find(Color);
			if (iter != mSheetColor.end())
			{
				auto iter1 = ColorMap.find(TargetColor);

				if (iter1 == ColorMap.end())
				{
					ColorMap.insert(std::make_pair(TargetColor, iter->second));
				}
			}

		}
	}

	std::vector<FVector4> ColorMappingArray = mPaletteColor[0];

	auto iter = ColorMap.begin();
	auto iterEnd = ColorMap.end();

	for (; iter != iterEnd; ++iter)
	{
		uint32_t Color = iter->first;

		unsigned int r = 0, g = 0, b = 0, a = 0;

		r = (Color >> 24) & 0xFF;
		g = (Color >> 16) & 0xFF;
		b = (Color >> 8) & 0xFF;
		a = Color & 0xFF;

		FVector4 CalcColor(r / 255.f, g / 255.f, b / 255.f, a / 255.f);

		//SheetColor가 가지고있는 인덱스 번호를 MappingArray의 인덱스 위치로 사용해서
		//컬러를 매핑해준다.
		ColorMappingArray[iter->second] = CalcColor;
	}

	//원본 컬러 팔레트를 넣어준다.
	mPaletteColor.push_back(ColorMappingArray);
}

bool CTexture::CreateResourceView(int Index)
{
	//SRV를 통해 레지스터에 텍스처를 등록해준다.
	//다만 텍스처를 레지스터에 등록해놓은뒤에
	//다른 메쉬에서 텍스처를 사용하려고하면 그대로 사용할수 있다.
	//따라서 nullptr로 텍스처를 내려줄수 있다.

	//텍스처 리소스뷰를 생성한다.
	if (FAILED(DirectX::CreateShaderResourceView(
		CDevice::GetInst()->GetDevice().Get(),
		mTextureList[Index]->Image->GetImages(),
		mTextureList[Index]->Image->GetImageCount(),
		mTextureList[Index]->Image->GetMetadata(),
		&mTextureList[Index]->SRV)))
	{
		return false;
	}

	mTextureList[Index]->Width = (unsigned int)mTextureList[Index]->Image->GetImages()[0].width;
	mTextureList[Index]->Height = (unsigned int)mTextureList[Index]->Image->GetImages()[0].height;

	//
	//mTextureList[Index]->Image->GetMetadata().mipLevels;

	return true;
}

void CTexture::Save(std::ofstream& File)
{
	size_t Size = mTextureList.size();

	File.write((char*)(&Size), sizeof(size_t));

	for (size_t i = 0; i < Size; ++i)
	{
		size_t Count = mTextureList[i]->FullPath.length();
		File.write((char*)(&Count), sizeof(size_t));
		File.write((char*)(mTextureList[i]->FullPath.c_str()), sizeof(wchar_t) * Count);
	}

}
