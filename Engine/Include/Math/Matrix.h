#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

//행렬
//행과 열을 가지고 있는 식
//가로줄과 세로줄로 배열한것

//행렬의 합
//합치려는 두 행렬의 크기가 완벽히 같아야한다.
//ex) 4x4 행렬 + 4x4 행렬 (o)
//	  4x4 행렬 + 2x4 행렬 (x)

//행렬의 곱
//곱하려는 두행렬의 크기중 앞행렬의 행과 뒤 행렬의 열이 동일하면 계산 가능
//곱한뒤에는 앞행렬의 행 x 뒤행렬의 열로 크기가 정해진다.
//ex) 4x1 행렬 * 1x4행렬 = 4x4 행렬

/* 1x4 행렬 * 4x4 행렬
1 2 3 4 * 1 2 3 4   1*1 + 2*5 + 3*9 + 4*3	50 30 40 50
		  5 6 7 8 = 1*2 + 2*6 + 3*0 + 4*4 = 
		  9 0 1 2	1*3 + 2*7 + 3*1 + 4*5
		  3 4 5 6	1*4 + 2*8 + 3*2 + 4*6
*/

//행렬의 곱은 교환법칙이 절대 성립하지 않는다.

//단위 행렬(항등 행렬)
//행렬을 곱했을때 어떤 행렬에 곱해도 자기자신이 나오는 행렬
/*
4x4 단위 행렬
1 0 0 0
0 1 0 0
0 0 1 0
0 0 0 1

*/

//전치
//주대각선을 기준으로 행과 열을 뒤바꾸는 것
//DX는 열 우선으로 메모리에 저장하기 때문에 전치를 수행해서 계산할수 있게 변경해야한다.

//직교 행렬
//행렬의 성분인 행벡터들이 모두 수직을 이루는 행렬

//역행렬
//어느 한 행렬에 역행렬을 곱하면 항등행렬이 나온다.
//2x2 행렬은 수반행렬 = det(A)
//3x3 이상의 행렬 가우스 조던 소거법
//행렬식 det(A) == 0 역행렬이 존재하지 않는다.

//행렬을 사용하는 이유
//오브젝트에게 있는 위치,회전,크기 값 이거를 한번에 계산하려고 행렬을 사용한다.

/*
4x4 행렬
크기 행렬
x 0 0 0
0 y 0 0
0 0 z 0
0 0 0 1

x축 회전 행렬
1	0	0	0
0 cos -sin	0
0 sin  cos	0
0	0	0	1

y축 회전 행렬
cos	0	sin	0
0	1	0	0
-sin0	cos	0
0	0	0	1

z축 회전 행렬
cos	-sin0	0
sin	cos	0	0
0	0	1	0
0	0	0	1

최종 회전 행렬 : x축 회전 행렬 * y축 회전 행렬 * z축 회전 행렬

이동 행렬
1	0	0	0
0	1	0	0
0	0	1	0
x	y	z	0

월드 행렬 = 크기 * 회전 * 이동 * 부모행렬
부모: 나에게 영향을 줄수 있는 오브젝트

오일러 각
3D회전을 x,y,z 축 각각의 회전으로 값을 표현
짐벌락: 한축이 90도 회전하게 되면 나머지 두축이 겹쳐서 회전이 정상적으로 되지 않는 것.

쿼터니언(4원수)
숫자가 4개
짐벌락없이 부드러운 회전 구현이 가능하다.

x축: pitch, y축: yaw, z축: roll

*/
//__declspec: 컴파일러한테 명령을 내리는 키워드
//align(n) : n바이트 기준으로 정렬한다
//GPU가 16바이트를 기준으로 메모리를 읽는다.

__declspec(align(16))union FMatrix
{
	DirectX::XMMATRIX m;

	struct
	{
		float _11, _12, _13, _14;
		float _21, _22, _23, _24;
		float _31, _32, _33, _34;
		float _41, _42, _43, _44;
	};

	FVector4 v[4];

	FMatrix();
	FMatrix(const FMatrix& _m);
	FMatrix(FMatrix&& _m) noexcept;
	FMatrix(const DirectX::XMMATRIX& _m);
	FMatrix(DirectX::XMMATRIX&& _m) noexcept;
	FMatrix(const FVector4 _m[4]);

	FVector4& operator[](int Index);

	const FMatrix& operator=(const FMatrix& _m);
	const FMatrix& operator=(const DirectX::XMMATRIX& _m);
	const FMatrix& operator=(const FVector4 _m[4]);

	//행렬 곱
	FMatrix operator*(const FMatrix& _m) const;
	FMatrix operator*(const DirectX::XMMATRIX& _m) const;

	void Identity();
	void Transpose();
	void Inverse();

	//크기 행렬
	void Scaling(const FVector3& v);
	void Scaling(float x, float y, float z);
	void Scaling(const FVector2& v);
	void Scaling(float x, float y);

	//회전 행렬
	void Rotation(const FVector3& v);
	void Rotation(float x, float y, float z);
	void RotationX(float x);
	void RotationY(float y);
	void RotationZ(float z);
	void RotationAxis(const FVector3& Axis, float Angle);

	//이동 행렬
	void Translation(const FVector3& v);
	void Translation(float x, float y, float z);
	void Translation(const FVector2& v);
	void Translation(float x, float y);


	//크기 회전 이동을 월드행렬에서 추출한다.
	void ExtractScale(FVector3& RetVal) const;
	void ExtractEuler(FVector3& RetVal) const;
	void ExtractPos(FVector3& RetVal) const;
	

	static FMatrix IdentityMatrix;

};


/*

x축 회전 행렬
1	0	0	0
0 cos -sin	0
0 sin  cos	0
0	0	0	1

y축 회전 행렬
cos	0	sin	0
0	1	0	0
-sin0	cos	0
0	0	0	1

z축 회전 행렬
cos	-sin0	0
sin	cos	0	0
0	0	1	0
0	0	0	1


순서는 Y * X * Z 순으로 행렬곱을 시행한다.


Y * X
cy		sy * sx		sy * cx		0
0		cx			-sx			0
-sy		cy * sx		cy * cx		0
0		0			0			1


Y * X * Z
(cy * cz) + (sy * sx * sz)		(cy * -sz) + (sy * sx * cz)		sy * cx		0
cx * sz							cx * cz							-sx			0
(-sy * cz) + (cy * sx * sz)		(-sy * -sz) + (cy * sx * cz)	cy * cx		0
0								0								0			1

x축 : pitch y축: yaw, z축: roll

_23으로 순수한 pitch값을 구할수 있다.
arcsin(-_23) : pitch 각도
arctan(_11 / _13) : yaw 각도
arctan(_22 / _21) : roll 각도


sin / cos = 높이/빗변 / 밑변/빗변 = 높이 / 밑변


*/