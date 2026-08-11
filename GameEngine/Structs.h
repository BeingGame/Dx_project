#pragma once

#include "Math/Matrix.h"

struct FResolution
{
	float Width = 0.f;
	float Height = 0.f;
};

struct FVertexBuffer
{
	//gpu에 있는 버텍스 버퍼
	ComPtr<ID3D11Buffer> Buffer = nullptr;

	//정점 한개당 크기
	int Size = 0;

	//정점의 개수
	int Count = 0;
};

struct FIndexBuffer
{
	ComPtr<ID3D11Buffer> Buffer = nullptr;

	//인덱스 1개의 메모리 크기
	int Size = 0;

	//인덱스의 개수
	int Count = 0;

	//인덱스를 이용해서 출력할때 인덱스의 타입을 지정하는데 사용하는 포맷
	DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
};

struct FVertexColor
{
	FVector3 Pos;
	FVector4 Color;

	FVertexColor()
	{

	}

	FVertexColor(float x, float y, float z, float r, float g, float b, float a)
		:Pos(x, y, z), Color(r, g, b, a)
	{

	}
};

struct FVertexTexture
{
	FVector3 Pos;
	//텍스처의 좌표로 사용될 UV를 넣어준다.e
	FVector2 UV;

	FVertexTexture()
	{

	}

	//텍스처 좌표인 UV는 왼쪽위부터 오른쪽아래까지 윈도우 좌표계랑 동일하게 되어있다.
	//값을 0~1으로 되어있다.
	//텍스처마다 크기가 달라서 텍스처 크기 * UV를 통해 텍스처의 픽셀을 가져올수 있다.
	//텍스처의 픽셀을 줄임말로 텍셀이라고 한다.
	FVertexTexture(float x, float y, float z ,float u, float v)
	{
		Pos.x = x;
		Pos.y = y;
		Pos.z = z;
		UV.x = u;
		UV.y = v;
	}

};

//사각형 충돌 구조체
struct FBox2DInfo
{
	FVector3 Center;
	FVector3 Axis[2] =
	{
		FVector3::Axis[EAxis::X],
		FVector3::Axis[EAxis::Y]
	};
	//사각형의 반지름의 역할을 할 벡터
	FVector2 Halfsize = FVector2(1.f, 1.f);
};

//원 충돌 구조체
struct FSphere2DInfo
{
	FVector3 Center;
	float Radius;
};

struct FCollisionChannel
{
	//채널 이름
	std::string Name;
	ECollisionChannel::Type Channel = ECollisionChannel::Static;
};


struct FCollisionProfile
{
	std::string Name;

	//프로파일이 사용하는 콜리전 채널
	std::shared_ptr<FCollisionChannel> Channel = nullptr;

	//콜리전이 활성화 상태인지 체크하는 변수
	bool Enable = true;

	//다른 콜리전 채널과 어떤 상호작용할지를 저장
	ECollisionInteraction::Type Interaction[ECollisionChannel::End] = {};
};

//실질적으로 충돌에서 사용하는것은 콜리전 프로파일을 이용한다.
//두개의 충돌체가 서로의 채널에서 같은 상호작용이 아니라면 충돌하지 않는것으로 판단한다.


//텍스처의 UV 시작좌표와 크기를 저장하는 구조체
struct FTextureFrame
{
	//Start시작부터 시작해서 Size만큼 UV를 읽는다.
	FVector2 Start;
	FVector2 Size;
	//최대 텍스처 크기 대비 비율
	FVector2 Ratio = FVector2(1.f, 1.f);
	//피벗 위치 오프셋
	FVector2 Offset = FVector2(0.f, 0.f);

};

struct FTileFrame
{
	FVector2 Start;
	FVector2 End;
};