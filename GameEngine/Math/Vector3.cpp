#include "Vector3.h"
#include "Matrix.h"

FVector3 FVector3::Zero;
FVector3 FVector3::One = FVector3(1.f, 1.f, 1.f);
FVector3 FVector3::Axis[EAxis::End] =
{
	FVector3(1.f,0.f,0.f),
	FVector3(0.f,1.f,0.f),
	FVector3(0.f,0.f,1.f)
};

FVector3::FVector3()
	: x(0.f), y(0.f), z(0.f)
{}

FVector3::FVector3(float _x, float _y, float _z)
	:x(_x), y(_y), z(_z)
{}

FVector3::FVector3(const FVector3& v)
	:x(v.x), y(v.y), z(v.z)
{}

FVector3::FVector3(FVector3&& v) noexcept
	:x(v.x), y(v.y), z(v.z)
{}

FVector3::FVector3(const DirectX::XMVECTOR v)
{
	DirectX::XMStoreFloat3((DirectX::XMFLOAT3*)this, v);
}

const FVector3& FVector3::operator=(const FVector3& v)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x = v.x;
	y = v.y;
	z = v.z;

	return *this;
}

const FVector3& FVector3::operator=(float Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x = Value;
	y = Value;
	z = Value;

	return *this;
}

const FVector3& FVector3::operator=(double Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x = (float)Value;
	y = (float)Value;
	z = (float)Value;

	return *this;
}

const FVector3& FVector3::operator=(int Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x = (float)Value;
	y = (float)Value;
	z = (float)Value;

	return *this;
}

FVector3 FVector3::operator+(const FVector3& v) const
{
	FVector3 _v;

	_v.x = x + v.x;
	_v.y = y + v.y;
	_v.z = z + v.z;

	return _v;
}

FVector3 FVector3::operator+(float Value) const
{
	FVector3 _v;

	_v.x = x + Value;
	_v.y = y + Value;
	_v.z = z + Value;

	return _v;
}

FVector3 FVector3::operator+(double Value) const
{
	FVector3 _v;

	_v.x = x + (float)Value;
	_v.y = y + (float)Value;
	_v.z = z + (float)Value;

	return _v;
}

FVector3 FVector3::operator+(int Value) const
{
	FVector3 _v;

	_v.x = x + (float)Value;
	_v.y = y + (float)Value;
	_v.z = z + (float)Value;

	return _v;
}

const FVector3& FVector3::operator+=(const FVector3& v)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x += v.x;
	y += v.y;
	z += v.z;

	return *this;
}

const FVector3& FVector3::operator+=(float Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x += Value;
	y += Value;
	z += Value;

	return *this;
}

const FVector3& FVector3::operator+=(double Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x += (float)Value;
	y += (float)Value;
	z += (float)Value;

	return *this;
}

const FVector3& FVector3::operator+=(int Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x += (float)Value;
	y += (float)Value;
	z += (float)Value;

	return *this;
}

const FVector3& FVector3::operator++()
{
	// TODO: 여기에 return 문을 삽입합니다.
	++x;
	++y;
	++z;

	return *this;
}

FVector3 FVector3::operator++(int)
{
	FVector3 v(*this);

	++x;
	++y;
	++z;

	return v;
}

FVector3 FVector3::operator-(const FVector3& v) const
{
	FVector3 _v;

	_v.x = x - v.x;
	_v.y = y - v.y;
	_v.z = z- v.z;

	return _v;
}

FVector3 FVector3::operator-(float Value) const
{
	FVector3 _v;

	_v.x = x - Value;
	_v.y = y - Value;
	_v.z = z - Value;

	return _v;
}

FVector3 FVector3::operator-(double Value) const
{
	FVector3 _v;

	_v.x = x - (float)Value;
	_v.y = y - (float)Value;
	_v.z = z - (float)Value;

	return _v;
}

FVector3 FVector3::operator-(int Value) const
{
	FVector3 _v;

	_v.x = x - (float)Value;
	_v.y = y - (float)Value;
	_v.z = z - (float)Value;

	return _v;
}

const FVector3& FVector3::operator-=(const FVector3& v)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x -= v.x;
	y -= v.y;
	z -= v.z;

	return *this;
}

const FVector3& FVector3::operator-=(float Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x -= Value;
	y -= Value;
	z -= Value;

	return *this;
}

const FVector3& FVector3::operator-=(double Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x -= (float)Value;
	y -= (float)Value;
	z -= (float)Value;

	return *this;
}

const FVector3& FVector3::operator-=(int Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x -= (float)Value;
	y -= (float)Value;
	z -= (float)Value;

	return *this;
}

const FVector3& FVector3::operator--()
{
	// TODO: 여기에 return 문을 삽입합니다.
	--x;
	--y;
	--z;

	return *this;
}

FVector3 FVector3::operator--(int)
{
	FVector3 v(*this);

	--x;
	--y;
	--z;

	return v;
}

FVector3 FVector3::operator*(const FVector3& v) const
{
	FVector3 _v;

	_v.x = x * v.x;
	_v.y = y * v.y;
	_v.z = z * v.z;

	return _v;
}

FVector3 FVector3::operator*(float Value) const
{
	FVector3 _v;

	_v.x = x * Value;
	_v.y = y * Value;
	_v.z = z * Value;

	return _v;
}

FVector3 FVector3::operator*(double Value) const
{
	FVector3 _v;

	_v.x = x * (float)Value;
	_v.y = y * (float)Value;
	_v.z = z * (float)Value;

	return _v;
}

FVector3 FVector3::operator*(int Value) const
{
	FVector3 _v;

	_v.x = x * (float)Value;
	_v.y = y * (float)Value;
	_v.z = z * (float)Value;

	return _v;
}

const FVector3& FVector3::operator*=(const FVector3& v)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x *= v.x;
	y *= v.y;
	z *= v.z;

	return *this;
}

const FVector3& FVector3::operator*=(float Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x *= Value;
	y *= Value;
	z *= Value;

	return *this;
}

const FVector3& FVector3::operator*=(double Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x *= (float)Value;
	y *= (float)Value;
	z *= (float)Value;

	return *this;
}

const FVector3& FVector3::operator*=(int Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x *= (float)Value;
	y *= (float)Value;
	z *= (float)Value;

	return *this;
}

FVector3 FVector3::operator/(const FVector3& v) const
{
	FVector3 _v;

	_v.x = x / v.x;
	_v.y = y / v.y;
	_v.z = z / v.z;

	return _v;
}

FVector3 FVector3::operator/(float Value) const
{
	FVector3 _v;

	_v.x = x / Value;
	_v.y = y / Value;
	_v.z = z / Value;

	return _v;
}

FVector3 FVector3::operator/(double Value) const
{
	FVector3 _v;

	_v.x = x / (float)Value;
	_v.y = y / (float)Value;
	_v.z = z / (float)Value;

	return _v;
}

FVector3 FVector3::operator/(int Value) const
{
	FVector3 _v;

	_v.x = x / (float)Value;
	_v.y = y / (float)Value;
	_v.z = z / (float)Value;

	return _v;
}

const FVector3& FVector3::operator/=(const FVector3& v)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x /= v.x;
	y /= v.y;
	z /= v.z;

	return *this;
}

const FVector3& FVector3::operator/=(float Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x /= Value;
	y /= Value;
	z /= Value;

	return *this;
}

const FVector3& FVector3::operator/=(double Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x /= (float)Value;
	y /= (float)Value;
	z /= (float)Value;

	return *this;
}

const FVector3& FVector3::operator/=(int Value)
{
	// TODO: 여기에 return 문을 삽입합니다.
	x /= (float)Value;
	y /= (float)Value;
	z /= (float)Value;

	return *this;
}

bool FVector3::operator==(const FVector3& v)
{
	//벡터의 비교

	//FLT_EPSILON : 0.0000001f 매우 작은 실수값

	//FLT_EPSILON 보다 작을경우 0으로 판단한다.
	return fabsf(x - v.x) < FLT_EPSILON && fabsf(y - v.y) < FLT_EPSILON && fabsf(z - v.z) < FLT_EPSILON;
}

bool FVector3::operator!=(const FVector3& v)
{
	return !(*this == v);
}

FVector3 FVector3::operator-() const
{
	return FVector3(-x, -y, -z);
}

float FVector3::Length()
{
	//피타고라스의 정리
	return sqrtf(x * x + y * y + z * z);
}

void FVector3::Normalize()
{
	float Len = Length();

	if (Len == 0.f)
	{
		return;
	}

	x = x / Len;
	y = y / Len;
	z = z / Len;
}

float FVector3::DotProduct(const FVector3& v) const
{
	float Dot = x * v.x + y * v.y + z * v.z;

	return Dot;
}

FVector3 FVector3::CrossProduct(const FVector3& v) const
{
	//외적
	return FVector3(y * v.z - z * v.y, z * v.x - x * v.x, x * v.y - y * v.x);
}

float FVector3::Distance(const FVector3& v) const
{
	FVector3 _v = *this - v;

	return _v.Length();
}

bool FVector3::IsZero() const
{
	return x == 0.f && y == 0.f && z == 0.f;
}

// w == 0
FVector3 FVector3::TransformNormal(const FMatrix& Mat) const
{
	//직사각형일때 빗면이 늘어난다.
	//역전치행렬을 사용하여 변경된 기울기에 따라 법선벡터를 정상적으로 수직이게 연산한다.

	//역행열의 역할은 비균등한 스케일 값을 상쇄시켜준다.
	//전치행렬 다시 시도해서 스케일이 상쇄된 행렬을 만들어준다.
	DirectX::XMMATRIX InverseMat = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, Mat.m));

	DirectX::XMVECTOR Vec = DirectX::XMVector3TransformNormal(Convert(), InverseMat);

	return Vec;
}

// w == 1
FVector3 FVector3::TransformCoord(const FMatrix& Mat) const
{
	DirectX::XMVECTOR Vec = DirectX::XMVector3TransformCoord(Convert(), Mat.m);

	return Vec;
}
