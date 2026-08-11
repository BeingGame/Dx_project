
#include "Matrix.h"
#include "../EngineInfo.h"

FMatrix FMatrix::IdentityMatrix;

FMatrix::FMatrix()
{
	m = DirectX::XMMatrixIdentity();
}

FMatrix::FMatrix(const FMatrix& _m)
{
	m = _m.m;
}

FMatrix::FMatrix(FMatrix&& _m) noexcept
{
	m = _m.m;
}

FMatrix::FMatrix(const DirectX::XMMATRIX& _m)
{
	m = _m;
}

FMatrix::FMatrix(DirectX::XMMATRIX&& _m) noexcept
{
	m = _m;
}

FMatrix::FMatrix(const FVector4 _m[4])
{
	memcpy(v, _m, sizeof(FVector4) * 4);
}

FVector4& FMatrix::operator[](int Index)
{
	if (Index < 0 || Index > 3)
	{
		return v[0];
	}

	return v[Index];

	// TODO: 여기에 return 문을 삽입합니다.
}

const FMatrix& FMatrix::operator=(const FMatrix& _m)
{
	// TODO: 여기에 return 문을 삽입합니다.
	m = _m.m;

	return *this;
}

const FMatrix& FMatrix::operator=(const DirectX::XMMATRIX& _m)
{
	// TODO: 여기에 return 문을 삽입합니다.

	m = _m;

	return *this;
}

const FMatrix& FMatrix::operator=(const FVector4 _m[4])
{
	// TODO: 여기에 return 문을 삽입합니다.

	memcpy(v, _m, sizeof(FVector4) * 4);

	return *this;
}

FMatrix FMatrix::operator*(const FMatrix& _m) const
{
	return FMatrix(m * _m.m);
}

FMatrix FMatrix::operator*(const DirectX::XMMATRIX& _m) const
{
	return FMatrix(m * _m);
}

//단위 행렬로 변경한다.
void FMatrix::Identity()
{
	m = DirectX::XMMatrixIdentity();
}

//전치행렬
void FMatrix::Transpose()
{
	m = DirectX::XMMatrixTranspose(m);
}

void FMatrix::Inverse()
{
	//역행렬에는 행렬식이 필요하다.
	//DX가 역행렬만들때 행렬식을 이용해서 역행렬을 계산한다.
	DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(m);

	m = DirectX::XMMatrixInverse(&det, m);
}

void FMatrix::Scaling(const FVector3& v)
{
	m = DirectX::XMMatrixScaling(v.x, v.y, v.z);
}

void FMatrix::Scaling(float x, float y, float z)
{
	m = DirectX::XMMatrixScaling(x, y, z);
}

void FMatrix::Scaling(const FVector2& v)
{
	m = DirectX::XMMatrixScaling(v.x, v.y, 1.f);
}

void FMatrix::Scaling(float x, float y)
{
	m = DirectX::XMMatrixScaling(x, y, 1.f);
}

void FMatrix::Rotation(const FVector3& v)
{
	float X = DirectX::XMConvertToRadians(v.x);
	float Y = DirectX::XMConvertToRadians(v.y);
	float Z = DirectX::XMConvertToRadians(v.z);

	DirectX::XMVECTOR Quat = DirectX::XMQuaternionRotationRollPitchYaw(X, Y, Z);

	m = DirectX::XMMatrixRotationQuaternion(Quat);
}

void FMatrix::Rotation(float x, float y, float z)
{
	float X = DirectX::XMConvertToRadians(x);
	float Y = DirectX::XMConvertToRadians(y);
	float Z = DirectX::XMConvertToRadians(z);

	DirectX::XMVECTOR Quat = DirectX::XMQuaternionRotationRollPitchYaw(X, Y, Z);

	m = DirectX::XMMatrixRotationQuaternion(Quat);
}

//각 요소를 기준으로 회전행렬을 생성한다.
void FMatrix::RotationX(float x)
{
	float X = DirectX::XMConvertToRadians(x);

	m = DirectX::XMMatrixRotationX(X);
}

void FMatrix::RotationY(float y)
{
	float Y = DirectX::XMConvertToRadians(y);

	m = DirectX::XMMatrixRotationY(Y);
}

void FMatrix::RotationZ(float z)
{
	float Z = DirectX::XMConvertToRadians(z);

	m = DirectX::XMMatrixRotationZ(Z);
}

void FMatrix::RotationAxis(const FVector3& Axis, float Angle)
{
	//축을 기준으로 회전행렬을 생성한다.
	float AngleValue = DirectX::XMConvertToRadians(Angle);
	DirectX::XMVECTOR AxisValue = DirectX::XMLoadFloat3((DirectX::XMFLOAT3*)&Axis);

	m = DirectX::XMMatrixRotationAxis(AxisValue, AngleValue);
}

void FMatrix::Translation(const FVector3& v)
{
	m = DirectX::XMMatrixTranslation(v.x, v.y, v.z);
}

void FMatrix::Translation(float x, float y, float z)
{
	m = DirectX::XMMatrixTranslation(x, y, z);

}

void FMatrix::Translation(const FVector2& v)
{
	m = DirectX::XMMatrixTranslation(v.x, v.y, 1.f);

}

void FMatrix::Translation(float x, float y)
{

	m = DirectX::XMMatrixTranslation(x, y, 1.f);
}

void FMatrix::ExtractScale(FVector3& RetVal) const
{
	//월드 행렬에서 크기를 추출하는 방법
	//벡터의 크기를 계산하는것과 동일
	RetVal.x = sqrtf(_11 * _11 + _12 * _12 + _13 * _13);
	RetVal.y = sqrtf(_21 * _21 + _22 * _22 + _23 * _23);
	RetVal.z = sqrtf(_31 * _31 + _32 * _32 + _33 * _33);
}

void FMatrix::ExtractEuler(FVector3& RetVal) const
{
	//DX에서 회전행렬이 곱해지는 순서 Roll->Pitch->Yaw 순서 ZXY
	//DX는 우리가 행렬 곱한거랑 반대로 곱한다.
	FVector3 Scale;
	ExtractScale(Scale);

	float r13 = _13 / Scale.x;
	float r21 = _21 / Scale.y;
	float r22 = _22 / Scale.y;
	float r23 = _23 / Scale.y;
	float r33 = _33 / Scale.z;

	if (fabsf(r23) < 0.999f)
	{
		RetVal.x = DirectX::XMConvertToDegrees(asinf(-r23));
		RetVal.y = DirectX::XMConvertToDegrees(atan2f(r13, r33));
		RetVal.z = DirectX::XMConvertToDegrees(atan2f(r21, r22));
	}
	else
	{
		//짐벌락이 발생하면 Yaw에 회전량을 몰아준다.

		float r11 = _11 / Scale.x;
		float r12 = _12 / Scale.x;

		RetVal.x = DirectX::XMConvertToDegrees(asinf(-r23));
		RetVal.y = DirectX::XMConvertToDegrees(atan2f(-r12, r11));
		RetVal.z = 0.f;
	}

}

void FMatrix::ExtractPos(FVector3& RetVal) const
{
	//이동은 4번째행에 존재한다.
	RetVal.x = _41;
	RetVal.y = _42;
	RetVal.z = _43;

}
