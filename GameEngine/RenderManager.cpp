#include "RenderManager.h"

#include "World/SceneComponent.h"
#include "World/WorldManager.h"
#include "Asset/AssetManager.h"
#include "Asset/ShaderManager.h"

#include "BlendState.h"
#include "DepthStencilState.h"


CRenderManager* CRenderManager::mInstance = nullptr;

CRenderManager::CRenderManager()
{

}

CRenderManager::~CRenderManager()
{
	ResetState("DepthDisable");
}

bool CRenderManager::CreateLayer(const std::string& Name, int RenderOrder, ERenderSortType SortType)
{
	auto iter = mRenderLayerMap.find(RenderOrder);

	if (iter != mRenderLayerMap.end())
	{
		return false;
	}

	FRenderLayer Layer;

	Layer.Name = Name;
	Layer.SortType = SortType;

	mRenderLayerMap.insert(std::make_pair(RenderOrder, Layer));


	return true;
}

int CRenderManager::GetLayerOrder(const std::string& Name)
{
	auto iter = mRenderLayerMap.begin();
	auto iterEnd = mRenderLayerMap.end();

	for (; iter != iterEnd; ++iter)
	{
		if (iter->second.Name == Name)
		{
			return iter->first;
		}
	}

	//찾는 렌더레이어가 없으면 -1 반환
	return -1;
}

void CRenderManager::AddRenderLayer(const std::weak_ptr<class CSceneComponent>& Component)
{
	auto _Comp = Component.lock();

	if (_Comp)
	{
		int Order = _Comp->GetLayerOrder();

		auto iter = mRenderLayerMap.find(Order);

		if (iter == mRenderLayerMap.end())
		{
			return;
		}

		iter->second.RenderList.push_back(Component);
	}

}

void CRenderManager::RemoveRenderObject(int RenderOrder, const std::weak_ptr<class CSceneComponent>& Component)
{
	auto iter = mRenderLayerMap.find(RenderOrder);

	if (iter == mRenderLayerMap.end())
	{
		return;
	}

	auto iter1 = iter->second.RenderList.begin();
	auto iter1End = iter->second.RenderList.end();

	for (; iter1 != iter1End;)
	{
		if (iter1->lock() == Component.lock())
		{
			iter->second.RenderList.erase(iter1);
			break;
		}

		++iter1;
	}

}

void CRenderManager::ClearRenderLayer(int RenderOrder)
{
	auto iter = mRenderLayerMap.find(RenderOrder);

	if (iter == mRenderLayerMap.end())
	{
		return;
	}

	iter->second.RenderList.clear();
}

void CRenderManager::AddRenderTargetBlendDesc(const std::string& Name, bool BlendEnable, D3D11_BLEND SrcBlend, D3D11_BLEND DestBlend, D3D11_BLEND_OP BlendOp, D3D11_BLEND SrcBlendAlpha, D3D11_BLEND DestBlendAlpha, D3D11_BLEND_OP BlendOpAlpha, UINT8 RenderTargetWriteMask)
{
	auto State = FindRenderState(Name);

	std::shared_ptr<CBlendState> BlendState;

	//스테이트가 없다면 새로운 스테이트를 만들어서 desc를 등록해준다.
	if (State.expired())
	{
		BlendState = std::make_shared<CBlendState>();

		mRenderStateMap.insert(std::make_pair(Name, BlendState));
	}
	else
	{
		BlendState = std::dynamic_pointer_cast<CBlendState>(State.lock());
	}

	BlendState->AddRenderTargetBlendDesc(BlendEnable, SrcBlend, DestBlend, BlendOp, SrcBlendAlpha, DestBlendAlpha, BlendOpAlpha, RenderTargetWriteMask);
}

void CRenderManager::SetBlendFactor(const std::string& Name, float r, float g, float b, float a)
{
	auto State = FindRenderState(Name);

	std::shared_ptr<CBlendState> BlendState;

	if (State.expired())
	{
		BlendState = std::make_shared<CBlendState>();

		mRenderStateMap.insert(std::make_pair(Name, BlendState));
	}
	else
	{
		BlendState = std::dynamic_pointer_cast<CBlendState>(State.lock());
	}

	BlendState->SetBlendFactor(r, g, b, a);
}

void CRenderManager::SetSampleMask(const std::string& Name, UINT SampleMask)
{
	auto State = FindRenderState(Name);

	std::shared_ptr<CBlendState> BlendState;

	if (State.expired())
	{
		BlendState = std::make_shared<CBlendState>();

		mRenderStateMap.insert(std::make_pair(Name, BlendState));
	}
	else
	{
		BlendState = std::dynamic_pointer_cast<CBlendState>(State.lock());
	}

	BlendState->SetSampleMask(SampleMask);
}

bool CRenderManager::CreateBlendState(const std::string& Name, bool AlphaToCoverageEnable, bool IndependentBlendEnable)
{
	auto State = FindRenderState(Name);

	std::shared_ptr<CBlendState> BlendState;

	if (State.expired())
	{
		BlendState = std::make_shared<CBlendState>();

		mRenderStateMap.insert(std::make_pair(Name, BlendState));
	}
	else
	{
		BlendState = std::dynamic_pointer_cast<CBlendState>(State.lock());
	}

	if (!BlendState->CreateState(AlphaToCoverageEnable, IndependentBlendEnable))
	{
		mRenderStateMap.erase(Name);

		return false;
	}

	return true;
}

bool CRenderManager::CreateDepthStencilState(const std::string& Name, bool DepthEnable, D3D11_DEPTH_WRITE_MASK DepthWriteMask, D3D11_COMPARISON_FUNC DepthFunc, bool StencilEnable, UINT8 StencilReadMask, UINT8 StencilWriteMask, D3D11_DEPTH_STENCILOP_DESC FrontFace, D3D11_DEPTH_STENCILOP_DESC BackFace)
{
	auto State = FindRenderState(Name);

	std::shared_ptr<CDepthStencilState> DepthStencilState;

	if (State.expired())
	{
		DepthStencilState = std::make_shared<CDepthStencilState>();

		mRenderStateMap.insert(std::make_pair(Name, DepthStencilState));
	}
	else
	{
		DepthStencilState = std::dynamic_pointer_cast<CDepthStencilState>(State.lock());
	}

	if (!DepthStencilState->CreateState(DepthEnable, DepthWriteMask, DepthFunc, StencilEnable, StencilReadMask, StencilWriteMask, FrontFace, BackFace))
	{
		mRenderStateMap.erase(Name);

		return false;
	}

	return true;
}

void CRenderManager::SetState(const std::string& Name)
{
	auto State = FindRenderState(Name).lock();

	if (State)
	{
		State->SetState();
	}

}

void CRenderManager::ResetState(const std::string& Name)
{
	auto State = FindRenderState(Name).lock();

	if (State)
	{
		State->ResetState();
	}
}

std::weak_ptr<class CRenderState> CRenderManager::FindRenderState(const std::string& Name)
{
	auto iter = mRenderStateMap.find(Name);

	if (iter == mRenderStateMap.end())
	{
		return std::weak_ptr<class CRenderState>();
	}

	return iter->second;
}

bool CRenderManager::Init()
{
	//렌더타겟 desc을 설정한다.

	//D3D11_BLEND_SRC_ALPHA : 알파값을 src의 rgb에 곱해줄값을 Alpha값으로 사용한다.
	//D3D11_BLEND_INV_SRC_ALPHA : 1 - Src.Alpha(1) = 0 -> src의 색상을 그대로 사용할수 있다.
	/*	src.rgba(1.f,1.f,1.f,0.5f) dest.rgba(0.7f,0.7f,0.7f,0.5f)
		계산식 Src.rgb * srcBlend blendOp Dest.rgb * destBlend
		(1.f,1.f,1.f) * 0.5f +  (0.7f,0.7f,0.7f) * 0.5f
		= (0.5f) + (0.35f) = (0.85f)

	*/
	AddRenderTargetBlendDesc("AlphaBlend", true
		, D3D11_BLEND_SRC_ALPHA
		, D3D11_BLEND_INV_SRC_ALPHA
		, D3D11_BLEND_OP_ADD
		, D3D11_BLEND_ONE
		, D3D11_BLEND_INV_SRC_ALPHA);

	CreateBlendState("AlphaBlend");

	CreateDepthStencilState("Depth", true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS_EQUAL);
	CreateDepthStencilState("DepthDisable", false);

	SetState("DepthDisable");

	//레이어를 생성한다.
	CreateLayer("BG", 0, ERenderSortType::Y);
	CreateLayer("Actor", 1, ERenderSortType::Y);

	return true;
}

void CRenderManager::Update(float DeltaTime)
{
	if (mMouseWidget[mMouseState])
	{
		mMouseWidget[mMouseState]->Update(DeltaTime);
	}
}

void CRenderManager::Render()
{
	//렌더 매니저에 등록된 렌더 레이어순서에따라
	//씬컴포넌트들을 렌더링 해준다.

	auto iter = mRenderLayerMap.begin();
	auto iterEnd = mRenderLayerMap.end();

	for (; iter != iterEnd; ++iter)
	{
		auto Comiter = iter->second.RenderList.begin();
		auto ComiterEnd = iter->second.RenderList.end();

		//현재 씬컴포넌트가 유효한지 확인한다.
		for (; Comiter != ComiterEnd;)
		{
			if (Comiter->expired())
			{
				Comiter = iter->second.RenderList.erase(Comiter);
				ComiterEnd = iter->second.RenderList.end();
				continue;
			}
			++Comiter;
		}


		//먼저 렌더리스트를 정렬해준다.
		//2D기준으로 Y정렬을 시도한다.
		switch (iter->second.SortType)
		{
		case ERenderSortType::None:
			break;
		case ERenderSortType::Y:
			if (iter->second.RenderList.size() > 1)
			{
				iter->second.RenderList.sort(SortYRenderList);
			}
			break;
		case ERenderSortType::Alpha:
			break;
		default:
			break;
		}

		Comiter = iter->second.RenderList.begin();
		ComiterEnd = iter->second.RenderList.end();

		for (; Comiter != ComiterEnd; ++Comiter)
		{
			auto _Comp = Comiter->lock();

			if (!_Comp->IsEnable())
			{
				continue;
			}
			else if (!_Comp->IsRenderEnable())
			{
				continue;
			}
			else if (!_Comp->IsAlive())
			{
				continue;
			}

			_Comp->Render();
		}

	}

	//모든 액터들이 렌더링되고 난 이후
	//UI를 렌더링해준다.
	
	auto ShaderMgr = CAssetManager::GetInst()->GetSubManager<CShaderManager>(EAssetType::Shader);

	if (ShaderMgr)
	{
		ShaderMgr->SetSampler("Linear", 0);
	}

	SetState("AlphaBlend");

	CWorldManager::GetInst()->RenderUI();

	if (mMouseWidget[mMouseState])
	{
		mMouseWidget[mMouseState]->Render();
	}

	ResetState("AlphaBlend");

}

void CRenderManager::PostRender()
{
	auto iter = mRenderLayerMap.begin();
	auto iterEnd = mRenderLayerMap.end();

	for (; iter != iterEnd; ++iter)
	{
		auto Comiter = iter->second.RenderList.begin();
		auto ComiterEnd = iter->second.RenderList.end();

		//현재 씬컴포넌트가 유효한지 확인한다.
		for (; Comiter != ComiterEnd;)
		{
			if (Comiter->expired())
			{
				Comiter = iter->second.RenderList.erase(Comiter);
				ComiterEnd = iter->second.RenderList.end();
				continue;
			}

			++Comiter;
		}


		//먼저 렌더리스트를 정렬해준다.
		//2D기준으로 Y정렬을 시도한다.
		switch (iter->second.SortType)
		{
		case ERenderSortType::None:
			break;
		case ERenderSortType::Y:
			if (iter->second.RenderList.size() > 1)
			{
				iter->second.RenderList.sort(SortYRenderList);
			}
			break;
		case ERenderSortType::Alpha:
			break;
		default:
			break;
		}

		Comiter = iter->second.RenderList.begin();
		ComiterEnd = iter->second.RenderList.end();

		for (; Comiter != ComiterEnd; ++Comiter)
		{
			auto _Comp = Comiter->lock();

			if (!_Comp->IsEnable())
			{
				continue;
			}
			else if (!_Comp->IsRenderEnable())
			{
				continue;
			}
			else if (!_Comp->IsAlive())
			{
				continue;
			}

			_Comp->PostRender();
		}

	}

}

bool CRenderManager::SortYRenderList(const std::weak_ptr<CSceneComponent>& Src, const std::weak_ptr<CSceneComponent>& Dest)
{
	//Y좌표를 이용해서 src와 dest를 비교한다.
	//비교할때 src가 더 큰지에 따라 비교를 시도한다.
	auto _Src = Src.lock();
	auto _Dest = Dest.lock();

	float SrcY = 0.f;
	float DestY = 0.f;

	if (!_Src)
	{
		SrcY = _Src->GetWorldPos().y;
	}
	else if (!_Dest)
	{
		DestY = _Dest->GetWorldPos().y;
	}

	//sort함수가 comp함수를 기준으로 정렬되며, Src와Dest를 비교할때 true를 기준으로 위치가 변경될 필요가 있다고 판단한다.
	//따라서 Src의 y좌표가 dest의 y좌표보다 크면 두 요소의 자리가 교체된다.
	return SrcY > DestY;
}
