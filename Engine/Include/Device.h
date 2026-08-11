#pragma once

#include "EngineInfo.h"

class CDevice
{
	Singleton(CDevice)

private:

	//DX와 관련된 리소스를 생성해주는 컴객체
	ComPtr<ID3D11Device> mDevice = nullptr;

	//GPU에 렌더링 명령을 내리는 컴객체
	//렌더링 파이프라인에 리소스를 바인딩해준다.
	ComPtr<ID3D11DeviceContext> mContext = nullptr;

	//스왑체인
	//이중 버퍼링을 이용해 화면의 깜빡거림을 방지한다.
	//정면 버퍼와 후면버퍼를 교체한다.
	ComPtr<IDXGISwapChain> mSwapChain = nullptr;

	//렌더타겟
	//최종적인 결과물(그래픽)을 화면이나 텍스처에 기록하는 View 인터페이스
	ComPtr<ID3D11RenderTargetView> mTargetView = nullptr;

	//뎁스 스텐실(Depth Stencil)
	//3D공간에서 픽셀마다 깊이값과 특정 값을 저장하는 버퍼
	//Depth 깊이값을 저장한다. Stencil 특정값을 저장한다.
	//ex) Masking 픽셀을 그릴지 말지 결정하는 데이터를 Stencil 저장한다.
	ComPtr<ID3D11DepthStencilView> mDepthStencil = nullptr;

	FResolution mRS;

	HWND mhWnd;

	bool mWindowMode = false;

	//폰트 출력을 위한 2D렌더타겟, 2D팩토리-> 폰트, 폰트 컬렉션에 사용될 컴객체
	ComPtr<ID2D1RenderTarget> m2DRenderTarget;
	ComPtr<ID2D1Factory> m2DFactory;

public:
	ComPtr<ID3D11Device> GetDevice() const
	{
		return mDevice;
	}
	ComPtr<ID3D11DeviceContext> GetContext() const
	{
		return mContext;
	}
	ComPtr<IDXGISwapChain> GetSwapChain() const
	{
		return mSwapChain;
	}
	ComPtr<ID3D11RenderTargetView> GetRenderTarget() const
	{
		return mTargetView;
	}
	ComPtr<ID3D11DepthStencilView> GetDepthStencil() const
	{
		return mDepthStencil;
	}

	ComPtr<ID2D1RenderTarget> Get2DRenderTarget() const
	{
		return m2DRenderTarget;
	}

	ComPtr<ID2D1Factory> Get2DFactory() const
	{
		return m2DFactory;
	}

	const FResolution& GetResolution() const
	{
		return mRS;
	}

	FVector2 GetResolutionRatio() const;

	bool GetWindowMode() const
	{
		return mWindowMode;
	}

public:
	bool Init(HWND hWnd, int Width, int Height,bool WindowMode);

	void BeginRender();
	void EndRender();

};

