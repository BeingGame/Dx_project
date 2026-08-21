#pragma once

//상수버퍼의 Shader타입

namespace EShaderBufferType
{
	enum Type
	{
		Vertex = 0x1,
		Pixel = 0x2,
		Domain = 0x4,
		Hull = 0x8,
		Geometry = 0x10,
		Compute = 0x20,
		VP = Vertex | Pixel,
		Graphic = Vertex | Pixel | Domain | Hull | Geometry,
		All = Graphic | Compute,
	};
}

//상수버퍼의 구조체 선언

//행렬을 받는 상수버퍼 구조체
struct FCBufferTransform
{
	FMatrix World;
	FMatrix View;
	FMatrix Proj;
	FMatrix WV; //World * View
	FMatrix WVP; //World * View * Proj
	FMatrix VP; //View * Proj
	FVector3 PivotSize;
	float Empty;
};

//컬러만 들어가는 상수버퍼 데이터
struct FCBufferCollider
{
	FVector4 Color;
};

struct FCBufferMaterial
{
	//패딩을 위해 FVector4를 앞으로
	FVector4 BaseColor;
	FVector4 DissolveEdgeColor = FVector4::Red;
	FVector4 HitColor = FVector4::Red;

	//UV 스크롤링
	int UVScrollEnable = false;
	FVector2 UVScrollSpeed;
	float UVScrollTime = 0.f;

	//크로마키
	int ChromakeyEnable = false;
	FVector3 ChromaKeyColor = FVector3(0.f, 1.f, 0.f);
	float ChromaKeyThreshold = 0.1f;

	//디졸브
	int DissolveEnable = false;
	float DissolveProgress = 0.f;
	float DissolveEdgeWidth = 0.04f;

	//히트 이펙트
	int HitEffectEnable = false;
	float HitIntensity = 0.f;

	float Opacity = 1.f;
	float Empty;
};

struct FCBufferAnimation2D
{
	FVector2 LTUV;
	//우측 아래 UV는 특별한일이 없으면 무조건 1.f,1.f
	FVector2 RBUV = FVector2(1.f, 1.f);
	int Animation2DEnable = 0;
	int Animation2DTextureType = -1;
	int TextureSymmetry = 0;
	int Animation2DFrame = 0;
	FVector2 Ratio = FVector2(1.f, 1.f);
	FVector2 Offset = FVector2(0.f, 0.f);

	//스프라이트 쿼드를 돌리는 회전각(라디안). 0이면 그대로다.
	//뒤의 Empty는 16바이트 정렬용 패딩이다. (셰이더 cbuffer와 크기가 같아야 한다)
	float Rotation = 0.f;
	FVector3 AnimEmpty = FVector3(0.f, 0.f, 0.f);
};

//상수버퍼 제한
//65536 바이트 = 64KB
struct FCBufferPalette
{
	FVector4 PaletteColor[256];
	int PaletteEnable;
	FVector3 Empty;
};

struct FCBufferUIDefault
{
	//위젯이 가지는 색상에 부가적으로 추가해줄 컬러
	FVector4 BrushTint;
	FVector4 WidgetColor;
	FVector2 BrushLTUV;
	FVector2 BrushRBUV;
	int BrushAnimationEnable = 0;
	int BrushTextureEnable = 0;
	int BrushPercentEnable = 0;
	float BrushPercent = 1.f;
	int BrushBarDir = 0;
	FVector3 Empty;
};

struct FCBufferTileMap
{
	FVector2 LTUV;
	FVector2 RBUV = FVector2(1.f, 1.f);
};