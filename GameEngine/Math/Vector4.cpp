#include "Vector4.h"


FVector4 FVector4::Zero;
FVector4 FVector4::One = FVector4(1.f, 1.f, 1.f, 1.f);
FVector4 FVector4::White = FVector4(1.f, 1.f, 1.f, 1.f);
FVector4 FVector4::Black = FVector4(0.f, 0.f, 0.f, 1.f);
FVector4 FVector4::Red = FVector4(1.f, 0.f, 0.f, 1.f);
FVector4 FVector4::Blue = FVector4(0.f, 0.f, 1.f, 1.f);
FVector4 FVector4::Green = FVector4(0.f, 1.f, 0.f, 1.f);

FVector4::FVector4()
	: x(0.f), y(0.f), z(0.f), w(0.f)
{}

FVector4::FVector4(float _x, float _y, float _z, float _w)
	:x(_x), y(_y), z(_z), w(_w)
{}

FVector4::FVector4(const FVector4& v)
	:x(v.x), y(v.y), z(v.z), w(v.w)
{}

FVector4::FVector4(FVector4&& v) noexcept
	:x(v.x), y(v.y), z(v.z), w(v.w)
{}

const FVector4& FVector4::operator=(const FVector4& v)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x = v.x;
	y = v.y;
	z = v.z;
	w = v.w;

	return *this;
}

const FVector4& FVector4::operator=(float Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x = Value;
	y = Value;
	z = Value;
	w = Value;

	return *this;
}

const FVector4& FVector4::operator=(double Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x = (float)Value;
	y = (float)Value;
	z = (float)Value;
	w = (float)Value;

	return *this;
}

const FVector4& FVector4::operator=(int Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x = (float)Value;
	y = (float)Value;
	z = (float)Value;
	w = (float)Value;

	return *this;
}

FVector4 FVector4::operator+(const FVector4& v) const
{
	FVector4 _v;

	_v.x = x + v.x;
	_v.y = y + v.y;
	_v.z = z + v.z;
	_v.w = w + v.w;

	return _v;
}

FVector4 FVector4::operator+(float Value) const
{
	FVector4 _v;

	_v.x = x + Value;
	_v.y = y + Value;
	_v.z = z + Value;
	_v.w = w + Value;

	return _v;
}

FVector4 FVector4::operator+(double Value) const
{
	FVector4 _v;

	_v.x = x + (float)Value;
	_v.y = y + (float)Value;
	_v.z = z + (float)Value;
	_v.w = w + (float)Value;

	return _v;
}

FVector4 FVector4::operator+(int Value) const
{
	FVector4 _v;

	_v.x = x + (float)Value;
	_v.y = y + (float)Value;
	_v.z = z + (float)Value;
	_v.w = w + (float)Value;

	return _v;
}

const FVector4& FVector4::operator+=(const FVector4& v)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;

	return *this;
}

const FVector4& FVector4::operator+=(float Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x += Value;
	y += Value;
	z += Value;
	w += Value;

	return *this;
}

const FVector4& FVector4::operator+=(double Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x += (float)Value;
	y += (float)Value;
	z += (float)Value;
	w += (float)Value;

	return *this;
}

const FVector4& FVector4::operator+=(int Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x += (float)Value;
	y += (float)Value;
	z += (float)Value;
	w += (float)Value;

	return *this;
}

const FVector4& FVector4::operator++()
{
	// TODO: 여기에 return 문을 삽입합니다.
	++x;
	++y;
	++z;
	++w;

	return *this;
}

FVector4 FVector4::operator++(int)
{
	FVector4 v(*this);

	++x;
	++y;
	++z;
	++w;

	return v;
}

FVector4 FVector4::operator-(const FVector4& v) const
{
	FVector4 _v;

	_v.x = x - v.x;
	_v.y = y - v.y;
	_v.z = z - v.z;
	_v.w = w - v.w;

	return _v;
}

FVector4 FVector4::operator-(float Value) const
{
	FVector4 _v;

	_v.x = x - Value;
	_v.y = y - Value;
	_v.z = z - Value;
	_v.w = w - Value;

	return _v;
}

FVector4 FVector4::operator-(double Value) const
{
	FVector4 _v;

	_v.x = x - (float)Value;
	_v.y = y - (float)Value;
	_v.z = z - (float)Value;
	_v.w = w - (float)Value;

	return _v;
}

FVector4 FVector4::operator-(int Value) const
{
	FVector4 _v;

	_v.x = x - (float)Value;
	_v.y = y - (float)Value;
	_v.z = z - (float)Value;
	_v.w = w - (float)Value;

	return _v;
}

const FVector4& FVector4::operator-=(const FVector4& v)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;

	return *this;
}

const FVector4& FVector4::operator-=(float Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x -= Value;
	y -= Value;
	z -= Value;
	w -= Value;

	return *this;
}

const FVector4& FVector4::operator-=(double Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x -= (float)Value;
	y -= (float)Value;
	z -= (float)Value;
	w -= (float)Value;

	return *this;
}

const FVector4& FVector4::operator-=(int Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x -= (float)Value;
	y -= (float)Value;
	z -= (float)Value;
	w -= (float)Value;

	return *this;
}

const FVector4& FVector4::operator--()
{
	// TODO: 여기에 return 문을 삽입합니다.
	--x;
	--y;
	--z;
	--w;

	return *this;
}

FVector4 FVector4::operator--(int)
{
	FVector4 v(*this);

	--x;
	--y;
	--z;
	--w;

	return v;
}

FVector4 FVector4::operator*(const FVector4& v) const
{
	FVector4 _v;

	_v.x = x * v.x;
	_v.y = y * v.y;
	_v.z = z * v.z;
	_v.w = w * v.w;

	return _v;
}

FVector4 FVector4::operator*(float Value) const
{
	FVector4 _v;

	_v.x = x * Value;
	_v.y = y * Value;
	_v.z = z * Value;
	_v.w = w * Value;

	return _v;
}

FVector4 FVector4::operator*(double Value) const
{
	FVector4 _v;

	_v.x = x * (float)Value;
	_v.y = y * (float)Value;
	_v.z = z * (float)Value;
	_v.w = w * (float)Value;

	return _v;
}

FVector4 FVector4::operator*(int Value) const
{
	FVector4 _v;

	_v.x = x * (float)Value;
	_v.y = y * (float)Value;
	_v.z = z * (float)Value;
	_v.w = w * (float)Value;

	return _v;
}

const FVector4& FVector4::operator*=(const FVector4& v)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x *= v.x;
	y *= v.y;
	z *= v.z;
	w *= v.w;

	return *this;
}

const FVector4& FVector4::operator*=(float Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x *= Value;
	y *= Value;
	z *= Value;
	w *= Value;

	return *this;
}

const FVector4& FVector4::operator*=(double Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x *= (float)Value;
	y *= (float)Value;
	z *= (float)Value;
	w *= (float)Value;

	return *this;
}

const FVector4& FVector4::operator*=(int Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x *= (float)Value;
	y *= (float)Value;
	z *= (float)Value;
	w *= (float)Value;

	return *this;
}

FVector4 FVector4::operator/(const FVector4& v) const
{
	FVector4 _v;

	_v.x = x / v.x;
	_v.y = y / v.y;
	_v.z = z / v.z;
	_v.w = w / v.w;

	return _v;
}

FVector4 FVector4::operator/(float Value) const
{
	FVector4 _v;

	_v.x = x / Value;
	_v.y = y / Value;
	_v.z = z / Value;
	_v.w = w / Value;

	return _v;
}

FVector4 FVector4::operator/(double Value) const
{
	FVector4 _v;

	_v.x = x / (float)Value;
	_v.y = y / (float)Value;
	_v.z = z / (float)Value;
	_v.w = w / (float)Value;

	return _v;
}

FVector4 FVector4::operator/(int Value) const
{
	FVector4 _v;

	_v.x = x / (float)Value;
	_v.y = y / (float)Value;
	_v.z = z / (float)Value;
	_v.w = w / (float)Value;

	return _v;
}

const FVector4& FVector4::operator/=(const FVector4& v)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x /= v.x;
	y /= v.y;
	z /= v.z;
	w /= v.w;

	return *this;
}

const FVector4& FVector4::operator/=(float Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x /= Value;
	y /= Value;
	z /= Value;
	w /= Value;

	return *this;
}

const FVector4& FVector4::operator/=(double Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x /= (float)Value;
	y /= (float)Value;
	z /= (float)Value;
	w /= (float)Value;

	return *this;
}

const FVector4& FVector4::operator/=(int Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x /= (float)Value;
	y /= (float)Value;
	z /= (float)Value;
	w /= (float)Value;

	return *this;
}

bool FVector4::operator==(const FVector4& v)
{
	//벡터의 비교

	//FLT_EPSILON : 0.0000001f 매우 작은 실수값

	//FLT_EPSILON 보다 작을경우 0으로 판단한다.
	return fabsf(x - v.x) < FLT_EPSILON && fabsf(y - v.y) < FLT_EPSILON && fabsf(z - v.z) < FLT_EPSILON;
}

bool FVector4::operator!=(const FVector4& v)
{
	return !(*this == v);
}

float& FVector4::operator[](int Index)
{
	// TODO: 여기에 return 문을 삽입합니다.

	switch (Index)
	{
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	case 3:
		return w;
	default:
		return x;
	}
}

