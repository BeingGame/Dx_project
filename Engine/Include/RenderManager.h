#pragma once

#include "EngineInfo.h"
#include "World/MouseWidget.h"

//렌더 매니저 전체적인 렌더링을 관리하는 시스템

//렌더 레이어
//레이어는 층을 나눠서 영역마다의 렌더 순서를 지정해주고
//렌더링 될 오브젝트(액터)를 정렬해서 최종적으로 레이와 정렬된 오브젝트 순서로 렌더링을 실시한다.
//ex) BGImage와 액터는 각각의 층을 가지고있어야 한다.
//렌더레이어 내부의 오브젝트는 매번 순서가 변할수 있다.

enum class ERenderSortType
{
	None,
	Y, //좌표 x,y중 y값을 이용해서 정렬을 시도한다.
	Alpha //알파 값을 이용해서 정렬을 시도한다.
};

struct FRenderLayer
{
	std::string Name;
	std::list<std::weak_ptr<class CSceneComponent>> RenderList;
	ERenderSortType SortType = ERenderSortType::Y;
};

class CRenderManager
{
	Singleton(CRenderManager);

private:
	std::map<int, FRenderLayer> mRenderLayerMap;

	//렌더스테이트를 관리하기위한 렌더스테이트 맵
	//필요하다면 다른곳에서 사용할수 있게 만든다.
	std::unordered_map<std::string, std::shared_ptr<class CRenderState>> mRenderStateMap;

	//마우스 위젯을 위한 변수선언
	EMouseState::Type mMouseState = EMouseState::Normal;
	std::shared_ptr<CMouseWidget> mMouseWidget[EMouseState::End];

public:
	void SetMouseState(EMouseState::Type State)
	{
		mMouseState = State;
	}

public:
	bool CreateLayer(const std::string& Name, int RenderOrder, ERenderSortType SortType = ERenderSortType::None);
	int GetLayerOrder(const std::string& Name);
	void AddRenderLayer(const std::weak_ptr<class CSceneComponent>& Component);
	void RemoveRenderObject(int RenderOrder, const std::weak_ptr<class CSceneComponent>& Component);
	void ClearRenderLayer(int RenderOrder);

public:
	//렌더매니저에서 렌더스테이트를 생성할수 있도록 만들어준다.
	void AddRenderTargetBlendDesc(const std::string& Name,bool BlendEnable,
		D3D11_BLEND SrcBlend = D3D11_BLEND_ONE,
		D3D11_BLEND DestBlend = D3D11_BLEND_ZERO,
		D3D11_BLEND_OP BlendOp = D3D11_BLEND_OP_ADD,
		D3D11_BLEND SrcBlendAlpha = D3D11_BLEND_ONE,
		D3D11_BLEND DestBlendAlpha = D3D11_BLEND_ZERO,
		D3D11_BLEND_OP BlendOpAlpha = D3D11_BLEND_OP_ADD,
		UINT8 RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL);

	void SetBlendFactor(const std::string& Name, float r, float g, float b, float a);
	void SetSampleMask(const std::string& Name, UINT SampleMask);
	bool CreateBlendState(const std::string& Name, bool AlphaToCoverageEnable = false, bool IndependentBlendEnable = false);
	bool CreateDepthStencilState(const std::string& Name, bool DepthEnable = true,
		D3D11_DEPTH_WRITE_MASK DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
		D3D11_COMPARISON_FUNC DepthFunc = D3D11_COMPARISON_LESS,
		bool StencilEnable = false, UINT8 StencilReadMask = 0xff,
		UINT8 StencilWriteMask = 0xff,
		D3D11_DEPTH_STENCILOP_DESC FrontFace =
		{
			D3D11_STENCIL_OP_KEEP,D3D11_STENCIL_OP_KEEP,
			D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS
		},
		D3D11_DEPTH_STENCILOP_DESC BackFace =
		{
			D3D11_STENCIL_OP_KEEP,D3D11_STENCIL_OP_KEEP,
			D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS
		});

	void SetState(const std::string& Name);
	void ResetState(const std::string& Name);

	std::weak_ptr<class CRenderState> FindRenderState(const std::string& Name);

public:
	bool Init();
	void Update(float DeltaTime);
	void Render();
	void PostRender();

public:
	template<typename T>
	std::weak_ptr<T> SetMouseWidget(EMouseState::Type State, const std::string& Name)
	{
		mMouseWidget[State] = std::make_shared<T>();

		mMouseWidget[State]->SetName(Name);

		if (!mMouseWidget[State]->Init())
		{
			return std::weak_ptr<T>();
		}

		//윈도우 커서를 숨겨주는 함수
		ShowCursor(false);

		return std::dynamic_pointer_cast<T>(mMouseWidget[State]);
	}

public:
	static bool SortYRenderList(const std::weak_ptr<CSceneComponent>& Src, const std::weak_ptr<CSceneComponent>& Dest);
};

