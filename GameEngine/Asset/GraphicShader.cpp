#include "GraphicShader.h"
#include "../Device.h"
#include "PathManager.h"

CGraphicShader::CGraphicShader()
{}

CGraphicShader::CGraphicShader(const CGraphicShader& src)
	:CShader(src)
{}

CGraphicShader::CGraphicShader(CShader&& src) noexcept
	:CShader(std::move(src))
{}

CGraphicShader::~CGraphicShader()
{}

void CGraphicShader::SetShader()
{
	CDevice::GetInst()->GetContext()->VSSetShader(mVertexShader.Get(), nullptr, 0);
	CDevice::GetInst()->GetContext()->PSSetShader(mPixelShader.Get(), nullptr, 0);

	CDevice::GetInst()->GetContext()->IASetInputLayout(mInputLayout.Get());
}

void CGraphicShader::AddInputDesc(const char* SemanticName, UINT SemanticIndex, DXGI_FORMAT Format, UINT InputSlot, UINT Size, D3D11_INPUT_CLASSIFICATION InputSlotClass, UINT InstanceDataStepRate)
{
	D3D11_INPUT_ELEMENT_DESC Desc = {};
	Desc.SemanticName = SemanticName;
	Desc.SemanticIndex = SemanticIndex;
	Desc.Format = Format;
	Desc.InputSlot = InputSlot;
	Desc.InputSlotClass = InputSlotClass;
	Desc.InstanceDataStepRate = InstanceDataStepRate;
	Desc.AlignedByteOffset = mSize[InputSlot];

	//ex) 0번 버텍스버퍼의 구조체안의 데이터들의 크기를 알려준다.

	//FVertexColor -> 0번주소 -> += sizeof(FVector3) -> 12번주소
	mSize[InputSlot] += Size;

	mInputDesc.push_back(Desc);

}

bool CGraphicShader::CreateInputLayout()
{
	//입력 레이아웃을 만든다.
	//정보의 시작점과 크기를 사용해서 버텍스 셰이더의 바이트코드를 이용해서만든다.
	//셰이더에서 사용하는 구조체의 시맨틱 정보와 현재 레이아웃의 시맨틱 정보가 일치하는지 확인하기 위해서

	if (FAILED(CDevice::GetInst()->GetDevice()->CreateInputLayout(&mInputDesc[0], (UINT)mInputDesc.size(), mVSBlob->GetBufferPointer(), mVSBlob->GetBufferSize(), &mInputLayout)))
	{
		return false;
	}

	return true;
}

bool CGraphicShader::LoadVertexShader(const char* EntryName, const TCHAR* FileName, const std::string& PathName)
{
	//경로를 이용해서 셰이더 코드를 바이트코드로 컴파일하여 객체를 생성한다.
	const TCHAR* BasePath = CPathManager::FindPath(PathName);

	TCHAR FullPath[MAX_PATH] = {};

	lstrcpy(FullPath, BasePath);
	lstrcat(FullPath, FileName);

	UINT Flag = 0;

#ifdef _DEBUG
	Flag = D3DCOMPILE_DEBUG;
#endif

	//컴파일 에러 메세지를 받기 위한 버퍼
	ID3DBlob* ErrorBlob = nullptr;

	/*
	1. Shader파일이 있는 경로
	2. Macro
	3. 셰이더 코드내에서 Include를 어떻게 불러올지 설정, STANDARD_FILE_INCLUDE를 일반적으로 사용하면된다.
	4. 컴파일 하려는 함수 이름
	5. Shader Version 지정
	6,7 추가 옵션 지정
	8. 컴파일된 Shader의 바이트코드
	9. 에러가 있을경우 에러메세지를 얻어오긴 위한 버퍼

	*/
	if (FAILED(D3DCompileFromFile(FullPath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryName, "vs_5_0", Flag, 0, mVSBlob.GetAddressOf(), &ErrorBlob)))
	{
#ifdef _DEBUG
		char ErrorText[128] = {};
		sprintf_s(ErrorText, (const char*)ErrorBlob->GetBufferPointer());
		strcpy_s(ErrorText, "\n");

		OutputDebugStringA(ErrorText);
#endif

		return false;
	}

	//성공적으로 바이트 코드를 생성했으면
	//저장된 바이트코드로 셰이더 객체를 생성한다.
	if (FAILED(CDevice::GetInst()->GetDevice()->CreateVertexShader(mVSBlob->GetBufferPointer()
		, mVSBlob->GetBufferSize(), nullptr, mVertexShader.GetAddressOf())))
	{
		return false;
	}

	return true;
}

bool CGraphicShader::LoadPixelShader(const char* EntryName, const TCHAR* FileName, const std::string& PathName)
{
	//경로를 이용해서 셰이더 코드를 바이트코드로 컴파일하여 객체를 생성한다.
	const TCHAR* BasePath = CPathManager::FindPath(PathName);

	TCHAR FullPath[MAX_PATH] = {};

	lstrcpy(FullPath, BasePath);
	lstrcat(FullPath, FileName);

	UINT Flag = 0;

#ifdef _DEBUG
	Flag = D3DCOMPILE_DEBUG;
#endif

	//컴파일 에러 메세지를 받기 위한 버퍼
	ID3DBlob* ErrorBlob = nullptr;

	/*
	1. Shader파일이 있는 경로
	2. Macro
	3. 셰이더 코드내에서 Include를 어떻게 불러올지 설정, STANDARD_FILE_INCLUDE를 일반적으로 사용하면된다.
	4. 컴파일 하려는 함수 이름
	5. Shader Version 지정
	6,7 추가 옵션 지정
	8. 컴파일된 Shader의 바이트코드
	9. 에러가 있을경우 에러메세지를 얻어오긴 위한 버퍼

	*/
	if (FAILED(D3DCompileFromFile(FullPath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryName, "ps_5_0", Flag, 0, mPSBlob.GetAddressOf(), &ErrorBlob)))
	{
#ifdef _DEBUG
		char ErrorText[128] = {};
		sprintf_s(ErrorText, (const char*)ErrorBlob->GetBufferPointer());
		strcpy_s(ErrorText, "\n");

		OutputDebugStringA(ErrorText);
#endif

		return false;
	}

	//성공적으로 바이트 코드를 생성했으면
	//저장된 바이트코드로 셰이더 객체를 생성한다.
	if (FAILED(CDevice::GetInst()->GetDevice()->CreatePixelShader(mPSBlob->GetBufferPointer()
		, mPSBlob->GetBufferSize(), nullptr, mPixelShader.GetAddressOf())))
	{
		return false;
	}

	return true;
}
