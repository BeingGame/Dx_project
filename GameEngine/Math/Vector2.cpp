#include "Vector2.h"

FVector2 FVector2::Zero;
FVector2 FVector2::One = FVector2(1.f, 1.f);

FVector2::FVector2()
	: x(0.f), y(0.f)
{}

FVector2::FVector2(float _x, float _y)
	:x(_x), y(_y)
{}

FVector2::FVector2(const FVector2& v)
	:x(v.x), y(v.y)
{}

FVector2::FVector2(FVector2&& v) noexcept
	:x(v.x), y(v.y)
{}

const FVector2& FVector2::operator=(const FVector2& v)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x = v.x;
	y = v.y;

	return *this;
}

const FVector2& FVector2::operator=(float Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x = Value;
	y = Value;

	return *this;
}

const FVector2& FVector2::operator=(double Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x = (float)Value;
	y = (float)Value;

	return *this;
}

const FVector2& FVector2::operator=(int Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x = (float)Value;
	y = (float)Value;

	return *this;
}

FVector2 FVector2::operator+(const FVector2& v) const
{
	FVector2 _v;

	_v.x = x + v.x;
	_v.y = y + v.y;

	return _v;
}

FVector2 FVector2::operator+(float Value) const
{
	FVector2 _v;

	_v.x = x + Value;
	_v.y = y + Value;

	return _v;
}

FVector2 FVector2::operator+(double Value) const
{
	FVector2 _v;

	_v.x = x + (float)Value;
	_v.y = y + (float)Value;

	return _v;
}

FVector2 FVector2::operator+(int Value) const
{
	FVector2 _v;

	_v.x = x + (float)Value;
	_v.y = y + (float)Value;

	return _v;
}

const FVector2& FVector2::operator+=(const FVector2& v)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x += v.x;
	y += v.y;

	return *this;
}

const FVector2& FVector2::operator+=(float Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x += Value;
	y += Value;

	return *this;
}

const FVector2& FVector2::operator+=(double Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x += (float)Value;
	y += (float)Value;

	return *this;
}

const FVector2& FVector2::operator+=(int Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x += (float)Value;
	y += (float)Value;

	return *this;
}

const FVector2& FVector2::operator++()
{
	// TODO: 여기에 return 문을 삽입합니다.
	++x;
	++y;

	return *this;
}

FVector2 FVector2::operator++(int)
{
	FVector2 v(*this);

	++x;
	++y;

	return v;
}

FVector2 FVector2::operator-(const FVector2& v) const
{
	FVector2 _v;

	_v.x = x - v.x;
	_v.y = y - v.y;

	return _v;
}

FVector2 FVector2::operator-(float Value) const
{
	FVector2 _v;

	_v.x = x - Value;
	_v.y = y - Value;

	return _v;
}

FVector2 FVector2::operator-(double Value) const
{
	FVector2 _v;

	_v.x = x - (float)Value;
	_v.y = y - (float)Value;

	return _v;
}

FVector2 FVector2::operator-(int Value) const
{
	FVector2 _v;

	_v.x = x - (float)Value;
	_v.y = y - (float)Value;

	return _v;
}

const FVector2& FVector2::operator-=(const FVector2& v)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x -= v.x;
	y -= v.y;

	return *this;
}

const FVector2& FVector2::operator-=(float Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x -= Value;
	y -= Value;

	return *this;
}

const FVector2& FVector2::operator-=(double Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x -= (float)Value;
	y -= (float)Value;

	return *this;
}

const FVector2& FVector2::operator-=(int Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x -= (float)Value;
	y -= (float)Value;

	return *this;
}

const FVector2& FVector2::operator--()
{
	// TODO: 여기에 return 문을 삽입합니다.
	--x;
	--y;

	return *this;
}

FVector2 FVector2::operator--(int)
{
	FVector2 v(*this);

	--x;
	--y;

	return v;
}

FVector2 FVector2::operator*(const FVector2& v) const
{
	FVector2 _v;

	_v.x = x * v.x;
	_v.y = y * v.y;

	return _v;
}

FVector2 FVector2::operator*(float Value) const
{
	FVector2 _v;

	_v.x = x * Value;
	_v.y = y * Value;

	return _v;
}

FVector2 FVector2::operator*(double Value) const
{
	FVector2 _v;

	_v.x = x * (float)Value;
	_v.y = y * (float)Value;

	return _v;
}

FVector2 FVector2::operator*(int Value) const
{
	FVector2 _v;

	_v.x = x * (float)Value;
	_v.y = y * (float)Value;

	return _v;
}

const FVector2& FVector2::operator*=(const FVector2& v)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x *= v.x;
	y *= v.y;

	return *this;
}

const FVector2& FVector2::operator*=(float Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x *= Value;
	y *= Value;

	return *this;
}

const FVector2& FVector2::operator*=(double Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x *= (float)Value;
	y *= (float)Value;

	return *this;
}

const FVector2& FVector2::operator*=(int Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x *= (float)Value;
	y *= (float)Value;

	return *this;
}

FVector2 FVector2::operator/(const FVector2& v) const
{
	FVector2 _v;

	_v.x = x / v.x;
	_v.y = y / v.y;

	return _v;
}

FVector2 FVector2::operator/(float Value) const
{
	FVector2 _v;

	_v.x = x / Value;
	_v.y = y / Value;

	return _v;
}

FVector2 FVector2::operator/(double Value) const
{
	FVector2 _v;

	_v.x = x / (float)Value;
	_v.y = y / (float)Value;

	return _v;
}

FVector2 FVector2::operator/(int Value) const
{
	FVector2 _v;

	_v.x = x / (float)Value;
	_v.y = y / (float)Value;

	return _v;
}

const FVector2& FVector2::operator/=(const FVector2& v)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x /= v.x;
	y /= v.y;

	return *this;
}

const FVector2& FVector2::operator/=(float Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x /= Value;
	y /= Value;

	return *this;
}

const FVector2& FVector2::operator/=(double Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x /= (float)Value;
	y /= (float)Value;

	return *this;
}

const FVector2& FVector2::operator/=(int Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x /= (float)Value;
	y /= (float)Value;

	return *this;
}

bool FVector2::operator==(const FVector2& v)
{
	//벡터의 비교

	//FLT_EPSILON : 0.0000001f 매우 작은 실수값

	//FLT_EPSILON 보다 작을경우 0으로 판단한다.
	return fabsf(x - v.x) < FLT_EPSILON && fabsf(y - v.y) < FLT_EPSILON;
}

bool FVector2::operator!=(const FVector2& v)
{
	return !(*this == v);
}

float FVector2::Length()
{
	//피타고라스의 정리
	return sqrtf(x * x + y * y);
}

void FVector2::Normalize()
{
	float Len = Length();

	if (Len == 0.f)
	{
		return;
	}

	x = x / Len;
	y = y / Len;
}

float FVector2::DotProduct(const FVector2& v) const
{
	float Dot = x * v.x + y * v.y;

	return Dot;
}

float FVector2::Distance(const FVector2& v) const
{
	FVector2 _v = *this - v;

	return _v.Length();
}
