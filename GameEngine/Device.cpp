#include "Device.h"

CDevice* CDevice::mInstance = nullptr;

CDevice::CDevice()
{

}

CDevice::~CDevice()
{

}

FVector2 CDevice::GetResolutionRatio() const
{
	//윈도우 클라이언트 영역의 크기를 가져온다.
	RECT WindowRC;
	GetClientRect(mhWnd, &WindowRC);

	//구해진 영역으로 현재 해상도와 윈도우 클라이언트 영역의 비율을 구해준다.
	FVector2 Ratio;
	Ratio.x = mRS.Width / (float)(WindowRC.right - WindowRC.left);
	Ratio.y = mRS.Height / (float)(WindowRC.bottom - WindowRC.top);

	return Ratio;
}

bool CDevice::Init(HWND hWnd, int Width, int Height, bool WindowMode)
{
	mhWnd = hWnd;

	mRS.Width = (float)Width;
	mRS.Height = (float)Height;

	mWindowMode = WindowMode;

	//윈도우는 BGRA 텍스처 포맷을 사용하며, DX는 RGBA 텍스처 포맷을 사용한다.
	//플래그로 DX11이 BGRA 텍스처 포멧을 사용할수 있는 플래그
	UINT Flag = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
	Flag |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL FLevel = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL FLevelResult;

	//첫번째 인자에 nullptr을 넣으면 내장/외장 그래픽카드 장치중 기본적으로 선택된 장치를 사용한다.
	//레벨은 배열로 넣을수 있으며, GPU에 따라 선택할 버전을 여러개 고를수 있다.
	if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, Flag, &FLevel, 1, D3D11_SDK_VERSION, mDevice.GetAddressOf(), &FLevelResult, mContext.GetAddressOf())))
	{
		return false;
	}

	//멀티 샘플링
	//울퉁불퉁한 면을 의도적으로 뭉개어 선으로 보이게 하는것
	//안티앨리어싱의 일종

	UINT SampleCount = 4;
	UINT Check = 0;

	//GPU가 멀티샘플링 레벨을 어디까지 사용할수 있는지 확인하는 함수
	mDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, SampleCount, &Check);

	if (Check < 1)
	{
		SampleCount = 1;
	}

	//스왑체인 설정
	DXGI_SWAP_CHAIN_DESC SwapDesc = {};

	SwapDesc.BufferDesc.Width = Width;
	SwapDesc.BufferDesc.Height = Height;

	//UNORM(unsigned Normalize) 0~1사이 값을 읽겠다
	//UINT
	SwapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//주사율
	// 1 / 60 -> 초당 60프레임
	SwapDesc.BufferDesc.RefreshRate.Numerator = 1;
	SwapDesc.BufferDesc.RefreshRate.Denominator = 0;
	//FLIP_DISCARD를 사용해 주사율을 OS에게 맡긴다.

	//SwapDesc.BufferDesc.RefreshRate.Numerator = 60;
	//SwapDesc.BufferDesc.RefreshRate.Denominator = 1; 

	//지정한 해상도와 모니터해상도가 맞지 않을 경우 어떻게 스케일링 할지 결정한다.
	//DXGI_MODE_SCALING_UNSPECIFIED = 0 드라이버/OS에게 맡긴다.
	//DXGI_MODE_SCALING_CENTERED = 1 작은 해상도 사용할때, 가운데 배치 + 주변은 검은 여백
	//DXGI_MODE_SCALING_STRETCHED = 2 해상도를 화면에 맞게 늘리거나 줄여서 사용
	SwapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//이미지 그릴때 가로 한줄을 어떻게 그릴지 결정하는 것
	//DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED = 0 현재 환경에 따라 맞춰서 그린다.
	//DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE = 1 화면을 위에서 아래로 시작하여 순차적으로 그린다.
	//DXGI_MODE_SCANLINE_ORDER_UPPER_FIELD_FIRST = 2 그릴때 홀수 번째 필드부터 먼저 그린다.
	//DXGI_MODE_SCANLINE_ORDER_LOWER_FIELD_FIRST = 3 그릴떈 짝수 번째 필드부터 먼저 그린다.
	//특별한 경우가 아니면 UNSPECIFIED를 사용한다.
	SwapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	//백버퍼 갯수
	SwapDesc.BufferCount = 2;

	//백버퍼를 출력용으로 지정한다.
	SwapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	SwapDesc.OutputWindow = mhWnd;

	SwapDesc.Windowed = WindowMode;

	//백버퍼에서 화면으로 넘길때 백버퍼를 어떻게 처리할지 결정하는 옵션
	//DXGI_SWAP_EFFECT_DISCARD = 0, 프레임을 화면에 보낸 뒤 버퍼 내용 보장X
	//DXGI_SWAP_EFFECT_SEQUENTIAL = 1 버퍼가 순서대로 화면에 처리된다.
	//위 두가지 방식은 페이지 플리핑X
	//DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL = 3 순차 출력을 하면서 플리핑 처리를 한다.
	//DXGI_SWAP_EFFECT_FLIP_DISCARD = 4 백버퍼를 뒤바꾸면서 백버퍼의 내용을 버린다. 가장 최신 버전의 옵션
	//DXGI_SWAP_EFFECT_FLIP_DISCARD 백버퍼가 최소 2개 이상이어야 사용 가능
	SwapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	//멀티샘플링 퀄리티를 지정한다. FLIP_DISCARD는 MSAA(멀티샘플링) 지원하지 않는다.
	SwapDesc.SampleDesc.Quality = 0;
	SampleCount = SwapDesc.SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD ? 1 : SampleCount;

	SwapDesc.SampleDesc.Count = SampleCount;

	//스왑체인 정보를 이용해서 스왑체인을 생성한다.

	//먼저 디바이스, 어댑터, 팩토리를 순차적으로 가져온다.

	ComPtr<IDXGIDevice> DXGIDevice = nullptr;

	mDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)DXGIDevice.GetAddressOf());

	//물리적인 그래픽카드(어댑터)를 가져온다.
	//DXGI->GetParent()
	ComPtr<IDXGIAdapter> Adapter = nullptr;
	DXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)Adapter.GetAddressOf());

	//어댑터를 통해 최상위 관리자인 팩토리를 가져온다.
	ComPtr<IDXGIFactory> Factory = nullptr;
	Adapter->GetParent(__uuidof(IDXGIFactory), (void**)Factory.GetAddressOf());

	if (FAILED(Factory->CreateSwapChain(mDevice.Get(), &SwapDesc, mSwapChain.GetAddressOf())))
	{
		return false;
	}

	//만들어진 스왑체인으로 백버퍼를 얻어와서 화면에 출력할 렌더타겟을 가져온다.
	ComPtr<ID3D11Texture2D> BackBuffer = nullptr;
	if (FAILED(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)BackBuffer.GetAddressOf())))
	{
		return false;
	}

	//백버퍼를 통해 렌더 타겟을 만들어준다
	if (FAILED(mDevice->CreateRenderTargetView(BackBuffer.Get(), nullptr, mTargetView.GetAddressOf())))
	{
		return false;
	}

	//깊이 버퍼를 생성하기 위한 정보를 만들어준다.
	D3D11_TEXTURE2D_DESC DepthDesc = {};

	DepthDesc.Width = Width;
	DepthDesc.Height = Height;

	//깊이 버퍼 갯수
	//VR같은 경우에는 깊이버퍼가 2개 
	DepthDesc.ArraySize = 1;

	//깊이 버퍼의 포맷을 지정한다.
	//D24 : Depth 24비트를 사용한다.
	//S8 : Stencil 8비트를 사용한다.
	DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	DepthDesc.Usage = D3D11_USAGE_DEFAULT;

	DepthDesc.SampleDesc.Count = 1;
	DepthDesc.SampleDesc.Quality = 0;

	//LOD 지정 깊이버퍼에서는 필요 없으므로 1로 둔다.
	DepthDesc.MipLevels = 1;

	ComPtr<ID3D11Texture2D> DepthBuffer = nullptr;

	if (FAILED(mDevice->CreateTexture2D(&DepthDesc, nullptr, DepthBuffer.GetAddressOf())))
	{
		return false;
	}

	if (FAILED(mDevice->CreateDepthStencilView(DepthBuffer.Get(), nullptr, mDepthStencil.GetAddressOf())))
	{
		return false;
	}

	//뷰포트
	//출력 영역 지정

	D3D11_VIEWPORT VP = {};

	VP.Width = (float)Width;
	VP.Height = (float)Height;

	//깊이값의 최대를 지정한다.
	VP.MaxDepth = 1.f;

	mContext->RSSetViewports(1, &VP);

	//2DFactory를 생성한다.
	if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m2DFactory.GetAddressOf())))
	{
		return false;
	}

	//생성된 3D 백버퍼를 2D 백버퍼에 연결해서
	//2D출력이 3D백버퍼에 출력 될수 있도록 만들어준다.
	ComPtr<IDXGISurface> BackSurface = nullptr;

	mSwapChain->GetBuffer(0, IID_PPV_ARGS(&BackSurface));

	//1. 렌더링의 주체가 어떤 녀석인지
	//2-1. 백퍼버 표면의 픽셀 포멧을 어떤것을 사용할지 결정하는것 DXGI_FORMAT_UNKNOWN: 기존 픽셀 포맷을 사용한다.
	//2-2. 3D장면에 2D 폰트를 그릴때 투명도 처리를 어떻게 할지 결정하는것 PREMULTIPLIED: 투명도가 정상적으로 복합 연산 되도록 사전 곱셈 알파 모드 적용
	D2D1_RENDER_TARGET_PROPERTIES ProPerties = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_HARDWARE, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

	//폰트 출력을 위해 2DRenderTarget을 생성한다.
	//생성한 렌더타겟을 3DRenderTarget에 부착한다.
	if (FAILED(m2DFactory->CreateDxgiSurfaceRenderTarget(BackSurface.Get(), ProPerties, m2DRenderTarget.GetAddressOf())))
	{
		return false;
	}

	return true;
}

void CDevice::BeginRender()
{
	//화면에 그리기전에 렌더타겟과 뎁스뷰를 초기화 한다.
	float Color[4] = {};
	mContext->ClearRenderTargetView(mTargetView.Get(), Color);
	mContext->ClearDepthStencilView(mDepthStencil.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	mContext->OMSetRenderTargets(1, mTargetView.GetAddressOf(), mDepthStencil.Get());
}

void CDevice::EndRender()
{
	//렌더링 파이프라인
	//렌더링을 하기 위한 그리기 단계

	//폴리곤
	//정점(Vertex)
	//정점이 3개 모여서 삼각형을 구성한다.
	//위치, 색상, 텍스처 UV

	//렌더링 파이프라인 단계
	//IA(Input Assembler) 입력 조립기
	//버텍스 셰이더(Vertex Shader) 정점 셰이더
	//테셀레이션(Tessellation) //2D에서 사용잘 안함
	//지오메트리 셰이더(Geometry Shader) // 동일하게 사용 잘 하지 않음
	//레스터라이저(Rasterizer)
	//픽셀 셰이더(Pixel Shader), 프래그먼트 셰이더
	//출력 병합(Output Merger)

	//IA(Input Assembler) 입력 조립기
	//받아온 정점을 삼각형을 구성한다.
	//CPU에서 정점을 계산한다. 이후에 버텍스 셰이더에서 사용할수있또록
	//GPU로 데이터를 넘겨준다.
	//프리미티브(Primitive)는 기본도형이라 생각하면 된다.

	//버텍스 셰이더(Vertex Shader)
	//IA단계에서 받아온 정점데이터를 공간 좌표계(Clip Space)로 변환해준다.

	//월드 변환
	//오브젝트(로컬) 공간-> 월드 공간
	//각자의 로컬공간(본인의 원점)을 월드 공간으로 변경해준다.
	//월드의 원점을 기준으로 오브젝트의 배치를 정해준다.

	//뷰 변환
	//월드 공간-> 뷰 공간
	//카메라를 기준으로 물체를 렌더링 하기전에 원점을 기준으로 변경하여
	//모든 오브젝트의 위치를 카메라가 원점으로 이동한 만큼 이동시켜
	//계산을 쉽게 하게한다.
	//+Z축을 기준으로 오브젝트들의 위치를 변경한다.

	//카메라의 절두체에 들어오지 않은 오브젝트는 컬링(Culling)으로 렌더링 명령을 무시한다.

	//투영 변환
	//뷰 공간-> 공간 좌표계
	//원근감을 적용하기 위해서 오브젝트의 위치를 공간 좌표계로 이동시킨다.
	//직교 투영, 원근 투영
	//직교 투영: 원근감이 없다. 2D환경에서 사용된다.
	//원근 투영: 원근감이 있다. 투영 행렬을 오브젝트의 위치값에 곱해주어 클립 스페이스로 변경한다.
	
	//투영 변환은 4D를 이용해서 계산한다.
	// 
	//--GPU가 알아서 해주는 단계--
	//w값은 카메라와의 거리
	//x,y,z를 w값으로 나누어서 x,y -1 ~ 1 사이의 값, z 0~1 사이의 값으로 변경한다.
	//NDC공간

	//테셀레이션(Tessellation)
	//하나의 삼각형을 여러개의 작은 삼각형으로 잘게 쪼개서 사용한다.
	
	//지오메트리 셰이더(Geometry Shader)
	//정점을 입력받아 정점을 추가 혹은 삭제해서 새로운 프리미티브를 생성할수 있는 단계

	//레스터라이저(Rasterizer)
	//버텍스 셰이더에서 변환된 정점 정보를 화면에 픽셀로 입력해주는 단계
	//NDC -> 2D로 표현하기 위해 스크린 좌표로 옮겨준다.
	//x,y 값을 이용해서 스크린에 픽셀데이터를 저장하고, z값을 깊이에 저장한다.
	//back face culling : 후면을 컬링한다.
	// 
	//--여기까지가 GPU가 알아서 해주는 단계

	//픽셀 셰이더(Pixel Shader)
	//픽셀 하나하나의 색상을 계산한다.
	//래스터라이저가 전달한 오브젝트가 화면에 차지하는 픽셀의 개수만큼 실행된다.
	//GPU가 하나하나 색상을 계산해준다.
	//투명도, 라이팅, 그림자처리 전부가 픽셀 셰이더의 역할
	//색상 값과, 깊이 값 저장한다.

	//출력 병합(Output Merger)
	//렌더링 파이프라인의 마지막 단계
	//최종적으로 알파 값(투명도)와 깊이 값등 여러가지 데이터의 픽셀끼리 경쟁해서 최종적으로 화면에 그려질 픽셀을 정한다.


	mSwapChain->Present(0, 0);
}