#include "ShaderManager.h"
#include "ShaderColor2D.h"
#include "ShaderFrame.h"
#include "ShaderMaterial.h"
#include "ShaderAnimation2D.h"
#include "ShaderUIDefault.h"
#include "ShaderTileMap.h"
#include "ShaderTexture.h"

#include "../Device.h"

CShaderManager::CShaderManager()
{}

CShaderManager::~CShaderManager()
{}

bool CShaderManager::Init()
{
	//셰이더
	if (!CreateShader<CShaderColor2D>("Color2D", "EngineShader"))
	{
		return false;
	}

	if (!CreateShader<CShaderFrame>("FrameColor2D", "EngineShader"))
	{
		return false;
	}

	if (!CreateShader<CShaderMaterial>("Material", "EngineShader"))
	{
		return false;
	}

	if (!CreateShader<CShaderTexture>("Texture", "EngineShader"))
	{
		return false;
	}

	if (!CreateShader<CShaderAnimation2D>("Animation2D", "EngineShader"))
	{
		return false;
	}

	if (!CreateShader<CShaderUIDefault>("UIDefault", "EngineShader"))
	{
		return false;
	}

	if (!CreateShader<CShaderTileMap>("TileMap", "EngineShader"))
	{
		return false;
	}

	//상수버퍼
	if (!CreateCBuffer("Transform", sizeof(FCBufferTransform), 0, EShaderBufferType::Vertex))
	{
		return false;
	}

	if (!CreateCBuffer("Collider", sizeof(FCBufferCollider), 10, EShaderBufferType::Pixel))
	{
		return false;
	}

	if (!CreateCBuffer("Material", sizeof(FCBufferMaterial), 1, EShaderBufferType::Pixel))
	{
		return false;
	}

	if (!CreateCBuffer("Animation2D", sizeof(FCBufferAnimation2D), 2, EShaderBufferType::VP))
	{
		return false;
	}

	if (!CreateCBuffer("Palette", sizeof(FCBufferPalette), 3, EShaderBufferType::Pixel))
	{
		return false;
	}

	if (!CreateCBuffer("UIDefault", sizeof(FCBufferUIDefault), 10, EShaderBufferType::VP))
	{
		return false;
	}

	if (!CreateCBuffer("TileMap", sizeof(FCBufferTileMap), 10, EShaderBufferType::VP))
	{
		return false;
	}

	return true;
}

bool CShaderManager::CreateCBuffer(const std::string& Name, int Size, int Register, int ShaderBuffer)
{
	std::weak_ptr<CCBuffer> Check = FindCBuffer(Name);

	if (!Check.expired())
	{
		return true;
	}

	std::shared_ptr<CCBuffer> NewCBuffer = std::make_shared<CCBuffer>();

	if (!NewCBuffer->Init(Size, Register, ShaderBuffer))
	{
		return false;
	}

	mBufferMap.insert(std::make_pair(Name, NewCBuffer));

	//점과 선형 필터링 만들어준다.
	//Min : 텍스처가 축소될때
	//Mag : 텍스처가 확대될때
	//Mip : 밈맵레벨이 적용될때 그사이를 어떻게 처리할것인가
	//마지막에 붙은 필터링으로 처리한다.
	CreateSampler("Point", D3D11_FILTER_MIN_MAG_MIP_POINT);
	CreateSampler("Linear", D3D11_FILTER_MIN_MAG_MIP_LINEAR);

	return true;
}

bool CShaderManager::CreateSampler(const std::string& Name, D3D11_FILTER Filter, D3D11_TEXTURE_ADDRESS_MODE u, D3D11_TEXTURE_ADDRESS_MODE v, D3D11_TEXTURE_ADDRESS_MODE w, UINT MaxAnisotropy, D3D11_COMPARISON_FUNC Func)
{
	if (FindSampler(Name))
	{
		return true;
	}

	//샘플러 정보를 입력한다.
	D3D11_SAMPLER_DESC Desc = {};

	Desc.Filter = Filter;

	//UV는 말그대로 우리가 사용하는 UV
	//W는 3D 텍스처에서 사용되는 두께에 대한 좌표
	Desc.AddressU = u;
	Desc.AddressV = v;
	Desc.AddressW = w;

	//비등방성 필터링
	//3D에서 각도에 따라 텍스처가 비스듬해질때 텍스처가 흐릿해지는 현상을 개선하기 위한 필터링
	//2D에선 사용X
	Desc.MaxAnisotropy = MaxAnisotropy;

	//Mip Level을 강제로 설정하는것
	//0이면 하드웨어가 알아서 Level처리를 한다.
	//음수값일땐 하드웨어보다 더 고해상도를, 양수면 저해상도를 선택하게 된다.
	Desc.MipLODBias = 0.f;
	Desc.MinLOD = 0.f;
	//Max로 두는 이유는 하드웨어가 알아서 처리하게 하기 위함
	Desc.MaxLOD = D3D11_FLOAT32_MAX;

	//D3D11_TEXTURE_ADDRESS_BORDER일때 범위를 넘어가면 
	//선택한 컬러로 칠하게 한다.
	//범위를 넘어가면 검은색으로 칠해준다.
	Desc.BorderColor[0] = 0.f;
	Desc.BorderColor[1] = 0.f;
	Desc.BorderColor[2] = 0.f;
	Desc.BorderColor[3] = 1.f;

	ComPtr<ID3D11SamplerState> Sampler = nullptr;

	if (FAILED(CDevice::GetInst()->GetDevice()->CreateSamplerState(&Desc, Sampler.GetAddressOf())))
	{
		return false;
	}

	mSamplerMap.insert(std::make_pair(Name, Sampler));

	return true;
}

void CShaderManager::SetSampler(const std::string& Name, int Register, int ShaderBufferType)
{
	auto Sampler = FindSampler(Name);

	//만들어져 있는 샘플러가 없으면 레지스터에 등록하지 않는다.
	if (!Sampler)
	{
		return;
	}

	if (ShaderBufferType & EShaderBufferType::Vertex)
	{
		CDevice::GetInst()->GetContext()->VSSetSamplers(Register, 1, Sampler.GetAddressOf());
	}
	if (ShaderBufferType & EShaderBufferType::Pixel)
	{
		CDevice::GetInst()->GetContext()->PSSetSamplers(Register, 1, Sampler.GetAddressOf());
	}
	if (ShaderBufferType & EShaderBufferType::Domain)
	{
		CDevice::GetInst()->GetContext()->DSSetSamplers(Register, 1, Sampler.GetAddressOf());
	}
	if (ShaderBufferType & EShaderBufferType::Hull)
	{
		CDevice::GetInst()->GetContext()->HSSetSamplers(Register, 1, Sampler.GetAddressOf());
	}
	if (ShaderBufferType & EShaderBufferType::Geometry)
	{
		CDevice::GetInst()->GetContext()->GSSetSamplers(Register, 1, Sampler.GetAddressOf());
	}
	if (ShaderBufferType & EShaderBufferType::Compute)
	{
		CDevice::GetInst()->GetContext()->CSSetSamplers(Register, 1, Sampler.GetAddressOf());
	}

}

std::weak_ptr<CShader> CShaderManager::FindShader(const std::string& Name)
{
	auto iter = mShaderMap.find(Name);

	if (iter == mShaderMap.end())
	{
		return std::weak_ptr<CShader>();
	}

	return iter->second;
}

std::weak_ptr<CCBuffer> CShaderManager::FindCBuffer(const std::string& Name)
{
	auto iter = mBufferMap.find(Name);

	if (iter == mBufferMap.end())
	{
		return std::weak_ptr<CCBuffer>();
	}

	return iter->second;
}

ComPtr<ID3D11SamplerState> CShaderManager::FindSampler(const std::string& Name)
{
	auto iter = mSamplerMap.find(Name);

	if (iter == mSamplerMap.end())
	{
		return nullptr;
	}

	return iter->second;
}
