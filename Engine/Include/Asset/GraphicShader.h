#pragma once

#include "Shader.h"

//실제로 렌더링 파이프라인을 실행할 셰이더들이 상속받을 추상클래스

class CGraphicShader :
	public CShader
{
public:
	CGraphicShader();
	CGraphicShader(const CGraphicShader& src);
	CGraphicShader(CShader&& src) noexcept;
	virtual ~CGraphicShader() = 0;

protected:
	//Vertex, Pixel, Domain, Hull, Geometry 셰이더 객체를 만드는데 사용된다.

	//Shader 데이터를 해석해서 사용하기 위해 입력 레이아웃 객체를 만들어준다.
	//입력 레지스터에 저장된 버텍스정보를 우리가 설정한것에 맞게 설정하면
	//HLSL에서 입력 레지스터에서 받아온 데이터를 형식에 맞게 사용할수 있도록 해준다.
	//따라서 정점정보가 변경되면 그에 맞는 새로운 입력레이아웃을 생성해야된다.
	ComPtr<ID3D11InputLayout> mInputLayout = nullptr;

	//입력 레지스터에 넣을 요소의 정보를 받을 구조체 배열을 만들어준다.
	//배열로 만드는 이유는 입력 레지스터에 넣는 정보가 여러가지 일수 있다.
	//각각의 정보가 한개의 구조체로 되어있다.
	std::vector<D3D11_INPUT_ELEMENT_DESC> mInputDesc;

	//입력 레이아웃 크기
	UINT mSize[2] = {};

	//Shader 객체, D3DCompileFromFile로 컴파일된 셰이더 객체를 저장한다.
	ComPtr<ID3D11VertexShader> mVertexShader = nullptr;

	//GPU가 읽을수 있는 바이트코드로 변환하여 저장하는 컴객체
	//최종적으로 Blob에 저장된 바이트코드를 셰이더 객체 코드에 저장한다.
	ComPtr<ID3DBlob> mVSBlob = nullptr;

	ComPtr<ID3D11PixelShader> mPixelShader = nullptr;

	ComPtr<ID3DBlob> mPSBlob = nullptr;


public:
	virtual bool Init(const std::string& PathName) = 0;
	//DX에 현재 무슨 셰이더를 사용할지 결정해서 출력하는 용도로 사용된다.
	virtual void SetShader();

public:
	/*
	LPCSTR SemanticName 레지스터 이름
	UINT SemanticIndex 레지스터 번호
	DXGI_FORMAT Format 몇바이트 포멧인지 저장
	UINT InputSlot 버텍스버퍼 번호, 3D환경에선 애니메이션의 가중치 계산을 위해서 1번 버텍스버퍼까지 사용한다.
	UINT Size 여기서 지정한 요소가 구조체의 몇바이트 째에 있는지
	D3D11_INPUT_CLASSIFICATION 정점데이터인지 아니면 인스턴싱
	UINT InstanceDataStepRate 인스턴싱일때만 사용
	*/

	void AddInputDesc(const char* SemanticName, UINT SemanticIndex, DXGI_FORMAT Format, UINT InputSlot,UINT Size,
		D3D11_INPUT_CLASSIFICATION InputSlotClass, UINT InstanceDataStepRate);
	bool CreateInputLayout();
	bool LoadVertexShader(const char* EntryName, const TCHAR* FileName, const std::string& PathName);
	bool LoadPixelShader(const char* EntryName, const TCHAR* FileName, const std::string& PathName);

};

