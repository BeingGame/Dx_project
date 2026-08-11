#pragma once

#include "../EngineMath.h"

union FMatrix;

struct FVector3
{
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;

	FVector3();
	FVector3(float _x, float _y, float _z);
	FVector3(const FVector3& v);
	FVector3(FVector3&& v) noexcept;
	FVector3(const DirectX::XMVECTOR v);

	//operator=
	const FVector3& operator=(const FVector3& v);
	const FVector3& operator=(float Value);
	const FVector3& operator=(double Value);
	const FVector3& operator=(int Value);

	//operator+
	FVector3 operator+(const FVector3& v) const;
	FVector3 operator+(float Value) const;
	FVector3 operator+(double Value) const;
	FVector3 operator+(int Value) const;

	//operator+=
	const FVector3& operator+=(const FVector3& v);
	const FVector3& operator+=(float Value);
	const FVector3& operator+=(double Value);
	const FVector3& operator+=(int Value);

	//operator++
	const FVector3& operator++();
	FVector3 operator++(int);

	//operator-
	FVector3 operator-(const FVector3& v) const;
	FVector3 operator-(float Value) const;
	FVector3 operator-(double Value) const;
	FVector3 operator-(int Value) const;

	//operator-=
	const FVector3& operator-=(const FVector3& v);
	const FVector3& operator-=(float Value);
	const FVector3& operator-=(double Value);
	const FVector3& operator-=(int Value);

	//operator--
	const FVector3& operator--();
	FVector3 operator--(int);

	//operator*
	FVector3 operator*(const FVector3& v) const;
	FVector3 operator*(float Value) const;
	FVector3 operator*(double Value) const;
	FVector3 operator*(int Value) const;

	//operator*=
	const FVector3& operator*=(const FVector3& v);
	const FVector3& operator*=(float Value);
	const FVector3& operator*=(double Value);
	const FVector3& operator*=(int Value);

	//operator/
	FVector3 operator/(const FVector3& v) const;
	FVector3 operator/(float Value) const;
	FVector3 operator/(double Value) const;
	FVector3 operator/(int Value) const;

	//operator/=
	const FVector3& operator/=(const FVector3& v);
	const FVector3& operator/=(float Value);
	const FVector3& operator/=(double Value);
	const FVector3& operator/=(int Value);

	//operator==
	bool operator==(const FVector3& v);

	//operator!=
	bool operator!=(const FVector3& v);

	//단항 연산
	FVector3 operator-() const;

	//벡터의 길이
	float Length();

	//벡터 정규화
	void Normalize();

	//벡터 내적
	float DotProduct(const FVector3& v) const;

	//벡터 외적
	FVector3 CrossProduct(const FVector3& v) const;

	//벡터 사이의 거리
	float Distance(const FVector3& v) const;

	//행렬 관련된 함수

	bool IsZero() const;

	//행렬에서 방향 변경
	FVector3 TransformNormal(const FMatrix& Mat) const;

	//행렬에서 방향및 위치 변경
	FVector3 TransformCoord(const FMatrix& Mat) const;


	DirectX::XMVECTOR Convert() const
	{
		return DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)this);
	}

	//기본 벡터
	static FVector3 Zero;
	static FVector3 One;
	static FVector3 Axis[EAxis::End];
};

