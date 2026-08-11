#include "ShaderFrame.h"

CShaderFrame::CShaderFrame()
{}

CShaderFrame::~CShaderFrame()
{}

bool CShaderFrame::Init(const std::string & PathName)
{
	//셰이더 로드하며 실패시 false를 반환한다.
	if (!LoadVertexShader("FrameVS", TEXT("Collider.hlsl"), PathName))
	{
		return false;
	}

	if (!LoadPixelShader("FramePS", TEXT("Collider.hlsl"), PathName))
	{
		return false;
	}

	//입력 레이아웃 정보를 넣어준다.
	AddInputDesc("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(FVector3), D3D11_INPUT_PER_VERTEX_DATA, 0);


	//만들어진 정보로 입력레이아웃을 생성한다.
	if (!CreateInputLayout())
	{
		return false;
	}

	return true;
}
