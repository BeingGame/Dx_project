#pragma once

#include "../EngineMath.h"

//벡터
//크기랑 방향을가진 물리량
//게임 프로그래밍 방향, 크기, 위치를 나타낸다.

//벡터의 크기
//피타고라스의 정리를 이용해서 구한다.

//단위 벡터
//크기가 1인 벡터, 사실상 방향을 나타낸다.

//법선 벡터(Normal)
//한점에서 수직인 벡터

//벡터의 합
//벡터의 합은 각 성분끼리 더해준다. 새로운 벡터를 만들어낸다.
//ex) Vector(x1,y1) + Vector(x2,y2) = Vector(x1+x2, y1+y2)

//벡터의 차
//벡터의 차는 각 성분끼리 빼준다.
//ex) Vector(x1,y1) - Vector(x2,y2) = Vector(x1-x2, y1-y2)
//v2 벡터를 기준으로 v2가 v1을 가르키는 벡터가 나온다.

//벡터의 정규화
//벡터의 성분을 합쳤을때 1이라는 값이 나와야한다.
//√(X^2 + Y^2) = 1
//ex) Vector(3,4) = √(9 + 16) = √25-> 5
//3/5, 4/5 = 0.6, 0.8
//√0.6^2 + 0.8^2 = √0.36 + 0.64 = √1 = 1

//벡터 곱
//외적 -> 3D개념에서 사용한다.

//내적(DotProduct)
//벡터를 곱해서 스칼라값을 만들어낸다.
//스칼라값: 크기를 가지고 있는 어떠한 값
//두 벡터를 곱해서 두벡터의 방향이 얼마나 일치하는지 알아낼수 있다.
//값이 1이 나오면 같은 방향의 벡터, 0이라면 서로가 수직인 벡터
//B라는 벡터의 A라는 벡터를 투영한 값
//ex)Vector(x1,y2) * Vector(x2,y2) = x1*x2 + y1*y2

//|v1| <- v1 벡터의 크기
//A⋅B = |A||B|cos n
//A랑 B가 단위벡터일때 : A⋅B = cos n * arccos n -> 각도가 나온다.

//외적
//2D내적은 사실 벡터가 시계방향인지 반시계방향인지 확인할수 있다.
//두 벡터를 외적하면 두 벡터가 동시에 수직인 벡터가 생성된다.
//벡터의 외적은 교환법칙이 성립하지 않는다.
//ex)Vector(x1,y1,z1) * Vector(x2,y2,z2) = Vector(y1*z2 - z1*y2,z1*x3 - x1*z3, x1*y2 - y1*x2)

//2D
//Vector(x1,y1) * Vector(x2,y2) = x1*y2 - x2*y1

//반사 벡터
//V: 입사벡터(벽을 향하는 벡터)
//N: 법선벡터(벽이 바라보는 정면벡터)
//입사 벡터를 법선벡터의 내적만큼 2번 곱해서 그만큼 돌려주면 된다.
//V - 2(V*N)N


struct FVector2
{
	float x = 0.f;
	float y = 0.f;

	FVector2();
	FVector2(float _x, float _y);
	FVector2(const FVector2& v);
	FVector2(FVector2&& v) noexcept;

	//operator=
	const FVector2& operator=(const FVector2& v);
	const FVector2& operator=(float Value);
	const FVector2& operator=(double Value);
	const FVector2& operator=(int Value);

	//operator+
	FVector2 operator+(const FVector2& v) const;
	FVector2 operator+(float Value) const;
	FVector2 operator+(double Value) const;
	FVector2 operator+(int Value) const;

	//operator+=
	const FVector2& operator+=(const FVector2& v);
	const FVector2& operator+=(float Value);
	const FVector2& operator+=(double Value);
	const FVector2& operator+=(int Value);

	//operator++
	const FVector2& operator++();
	FVector2 operator++(int);

	//operator-
	FVector2 operator-(const FVector2& v) const;
	FVector2 operator-(float Value) const;
	FVector2 operator-(double Value) const;
	FVector2 operator-(int Value) const;

	//operator-=
	const FVector2& operator-=(const FVector2& v);
	const FVector2& operator-=(float Value);
	const FVector2& operator-=(double Value);
	const FVector2& operator-=(int Value);

	//operator--
	const FVector2& operator--();
	FVector2 operator--(int);

	//operator*
	FVector2 operator*(const FVector2& v) const;
	FVector2 operator*(float Value) const;
	FVector2 operator*(double Value) const;
	FVector2 operator*(int Value) const;

	//operator*=
	const FVector2& operator*=(const FVector2& v);
	const FVector2& operator*=(float Value);
	const FVector2& operator*=(double Value);
	const FVector2& operator*=(int Value);

	//operator/
	FVector2 operator/(const FVector2& v) const;
	FVector2 operator/(float Value) const;
	FVector2 operator/(double Value) const;
	FVector2 operator/(int Value) const;

	//operator/=
	const FVector2& operator/=(const FVector2& v);
	const FVector2& operator/=(float Value);
	const FVector2& operator/=(double Value);
	const FVector2& operator/=(int Value);

	//operator==
	bool operator==(const FVector2& v);

	//operator!=
	bool operator!=(const FVector2& v);

	//벡터의 길이
	float Length();

	//벡터 정규화
	void Normalize();

	//벡터 내적
	float DotProduct(const FVector2& v) const;

	//벡터 사이의 거리
	float Distance(const FVector2& v) const;

	//기본 벡터
	static FVector2 Zero;
	static FVector2 One;
};