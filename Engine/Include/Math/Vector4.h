#pragma once

#include "../EngineMath.h"

struct FVector4
{
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;

	FVector4();
	FVector4(float _x, float _y, float _z, float _w);
	FVector4(const FVector4& v);
	FVector4(FVector4&& v) noexcept;

	//operator=
	const FVector4& operator=(const FVector4& v);
	const FVector4& operator=(float Value);
	const FVector4& operator=(double Value);
	const FVector4& operator=(int Value);

	//operator+
	FVector4 operator+(const FVector4& v) const;
	FVector4 operator+(float Value) const;
	FVector4 operator+(double Value) const;
	FVector4 operator+(int Value) const;

	//operator+=
	const FVector4& operator+=(const FVector4& v);
	const FVector4& operator+=(float Value);
	const FVector4& operator+=(double Value);
	const FVector4& operator+=(int Value);

	//operator++
	const FVector4& operator++();
	FVector4 operator++(int);

	//operator-
	FVector4 operator-(const FVector4& v) const;
	FVector4 operator-(float Value) const;
	FVector4 operator-(double Value) const;
	FVector4 operator-(int Value) const;

	//operator-=
	const FVector4& operator-=(const FVector4& v);
	const FVector4& operator-=(float Value);
	const FVector4& operator-=(double Value);
	const FVector4& operator-=(int Value);

	//operator--
	const FVector4& operator--();
	FVector4 operator--(int);

	//operator*
	FVector4 operator*(const FVector4& v) const;
	FVector4 operator*(float Value) const;
	FVector4 operator*(double Value) const;
	FVector4 operator*(int Value) const;

	//operator*=
	const FVector4& operator*=(const FVector4& v);
	const FVector4& operator*=(float Value);
	const FVector4& operator*=(double Value);
	const FVector4& operator*=(int Value);

	//operator/
	FVector4 operator/(const FVector4& v) const;
	FVector4 operator/(float Value) const;
	FVector4 operator/(double Value) const;
	FVector4 operator/(int Value) const;

	//operator/=
	const FVector4& operator/=(const FVector4& v);
	const FVector4& operator/=(float Value);
	const FVector4& operator/=(double Value);
	const FVector4& operator/=(int Value);

	//operator==
	bool operator==(const FVector4& v);

	//operator!=
	bool operator!=(const FVector4& v);

	//operator[]
	float& operator[](int Index);

	//기본 벡터
	static FVector4 Zero;
	static FVector4 One;
	static FVector4 White;
	static FVector4 Black;
	static FVector4 Red;
	static FVector4 Blue;
	static FVector4 Green;

};

