#pragma once

#include "AssetSubManager.h"
#include "Shader.h"
#include "CBuffer.h"
#include "CBufferContainer.h"

class CShaderManager :
	public CAssetSubManager
{
public:
	CShaderManager();
	virtual ~CShaderManager();

protected:
	std::unordered_map<std::string, std::shared_ptr<CShader>> mShaderMap;
	std::unordered_map<std::string, std::shared_ptr<CCBuffer>> mBufferMap;
	
	//샘플러를 저장할 Map
	std::map<std::string, ComPtr<ID3D11SamplerState>> mSamplerMap;

public:
	virtual bool Init();

	bool CreateCBuffer(const std::string& Name, int Size, int Register, int ShaderBuffer = EShaderBufferType::VP);


	//샘플러 생성함수
	/*
		텍스처 좌표의 범위가 0~1을 넘어갔을때 좌표를 어떻게 처리할지에 대한 열거형
		D3D11_TEXTURE_ADDRESS_WRAP	= 1, 텍스처가 반복되는 형태 0~1을 넘어가면 1.01부터는 다시 0 ~ 1로 반복된다. 앞자리를 버리고 소수점만 사용한다고 생각하면 된다.
        D3D11_TEXTURE_ADDRESS_MIRROR	= 2, 범위를 넘어가면 텍스처가 좌우가 뒤집힌 형태로 반복된다. uv 0~1 -> 1~2 : 1~0
        D3D11_TEXTURE_ADDRESS_CLAMP	= 3, 범위를 넘어가면 가장자리의 픽셀의 색상으로 채워지는 형태
        D3D11_TEXTURE_ADDRESS_BORDER	= 4, 범위를 넘어가면 지정한 border color로 채워지는 형태
        D3D11_TEXTURE_ADDRESS_MIRROR_ONCE	= 5 범위를 넘어가면 딱 한번만 미러링이 적용되고 그 이후에는 clamp처럼 색상을 고정하는 형태가 된다.


		D3D11_COMPARISON_FUNC: 비교함수 샘플러가 텍스처의 색상을 기준값과 비교해서 필터링된 0또는 1의 결과를 반환하는 함수
		선형필터링은 0~1사이의 값으로 반환된다.
		3D에서 사용된다. 셰이더에서 그림자처리할때
		2D에선 미사용
	*/

	bool CreateSampler(const std::string& Name, D3D11_FILTER Filter,
		D3D11_TEXTURE_ADDRESS_MODE u = D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_MODE v = D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_MODE w = D3D11_TEXTURE_ADDRESS_WRAP,
		UINT MaxAnisotropy = 0,
		D3D11_COMPARISON_FUNC Func = D3D11_COMPARISON_NEVER);
	void SetSampler(const std::string& Name, int Register, int ShaderBufferType = EShaderBufferType::Pixel);

public:
	std::weak_ptr<CShader> FindShader(const std::string& Name);
	std::weak_ptr<CCBuffer> FindCBuffer(const std::string& Name);
	ComPtr<ID3D11SamplerState> FindSampler(const std::string& Name);

public:
	template<typename T>
	bool CreateShader(const std::string& Name, const std::string& PathName = "Shader")
	{
		//먼저 셰이더 있는지 확인
		std::weak_ptr<CShader> Check = FindShader(Name);

		//있으면 true 반환
		if (!Check.expired())
		{
			return true;
		}

		std::shared_ptr<T> NewShader = std::make_shared<T>();

		if (!NewShader->Init(PathName))
		{
			return false;
		}

		NewShader->SetName(Name);

		mShaderMap.insert(std::make_pair(Name, NewShader));

		return true;
	}


};

