#include "BlendState.h"

#include "Device.h"

CBlendState::CBlendState()
{}

CBlendState::~CBlendState()
{}

void CBlendState::AddRenderTargetBlendDesc(bool BlendEnable, D3D11_BLEND SrcBlend, D3D11_BLEND DestBlend, D3D11_BLEND_OP BlendOp, D3D11_BLEND SrcBlendAlpha, D3D11_BLEND DestBlendAlpha, D3D11_BLEND_OP BlendOpAlpha, UINT8 RenderTargetWriteMask)
{
	D3D11_RENDER_TARGET_BLEND_DESC Desc = {};

	Desc.BlendEnable = BlendEnable;
	Desc.SrcBlend = SrcBlend;
	Desc.DestBlend = DestBlend;
	Desc.BlendOp = BlendOp;
	Desc.SrcBlendAlpha = SrcBlendAlpha;
	Desc.DestBlendAlpha = DestBlendAlpha;
	Desc.BlendOpAlpha = BlendOpAlpha;
	Desc.RenderTargetWriteMask = RenderTargetWriteMask;

	mTargetDesc.push_back(Desc);
}

void CBlendState::SetBlendFactor(float r, float g, float b, float a)
{
	mBlendFactor[0] = r;
	mBlendFactor[1] = g;
	mBlendFactor[2] = b;
	mBlendFactor[3] = a;
}

void CBlendState::SetSampleMask(UINT SampleMask)
{
	mSampleMask = SampleMask;
}

bool CBlendState::CreateState(bool AlphaToCoverageEnable, bool IndependentBlendEnable)
{
	//스테이트 생성할때 desc가 없다면 적용할 블렌드 정보가 없으므로 생성이 불가능
	if (mTargetDesc.empty())
	{
		return false;
	}

	D3D11_BLEND_DESC Desc = {};

	Desc.AlphaToCoverageEnable = AlphaToCoverageEnable;
	Desc.IndependentBlendEnable = IndependentBlendEnable;

	//설정한 TargetDesc를 전부 Desc에 복사해준다.
	size_t Size = mTargetDesc.size();

	memcpy(Desc.RenderTarget, &mTargetDesc[0], sizeof(D3D11_RENDER_TARGET_BLEND_DESC) * Size);

	if (FAILED(CDevice::GetInst()->GetDevice()->CreateBlendState(&Desc, (ID3D11BlendState**)mState.GetAddressOf())))
	{
		return false;
	}

	return true;
}

void CBlendState::SetState()
{
	//set됬던 prevstate를 가져오고, 현재 스테이트를 등록한다.
	CDevice::GetInst()->GetContext()->OMGetBlendState((ID3D11BlendState**)mPrevState.GetAddressOf(), mPrevBlendFactor, &mPrevSampleMask);

	CDevice::GetInst()->GetContext()->OMSetBlendState((ID3D11BlendState*)mState.Get(), mBlendFactor, mSampleMask);
}

void CBlendState::ResetState()
{
	//이전에 저장했던 prevstate를 다시 돌려준다.
	CDevice::GetInst()->GetContext()->OMSetBlendState((ID3D11BlendState*)mPrevState.Get(), mPrevBlendFactor, mPrevSampleMask);

	mPrevState.Reset();
}
