#include "CBuffer.h"
#include "../Device.h"

CCBuffer::CCBuffer()
{}

CCBuffer::CCBuffer(const CCBuffer & src)
{
	mBuffer = src.mBuffer;
}

CCBuffer::CCBuffer(CCBuffer && src) noexcept
{
	mBuffer = src.mBuffer;
	src.mBuffer.Reset();
}

CCBuffer::~CCBuffer()
{}

bool CCBuffer::Init(int Size, int Register, int ShaderBuffer)
{
	mSize = Size;
	mRegister = Register;
	mShaderBuffer = ShaderBuffer;

	D3D11_BUFFER_DESC Desc = {};

	//상수버퍼는 갱신이 빈번하기 때문에 Dynamic으로 생성한다.
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.ByteWidth = mSize;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(CDevice::GetInst()->GetDevice()->CreateBuffer(&Desc, nullptr, mBuffer.GetAddressOf())))
	{
		return false;
	}

	return true;
}

void CCBuffer::Update(void* Data)
{
	//인자로 들어온 void 포인터 데이터를 버퍼에 갱신해준다.
	//데이터를 넣을때는 map과 unmap 함수를 사용한다.
	//안전한게 GPU에 데이터를 넣기 위함

	//CPU랑 GPU랑 연산속도 차이로 동기화 문제가 발생
	//CPU가 데이터를 고치는 동안 GPU가 데이터를 사용하는 문제를 막기위함

	D3D11_MAPPED_SUBRESOURCE Map = {};

	//먼저 Map안에 버퍼의 시작주소를 얻어오고, map의 용도를 기입한다.
	//D3D11_MAP_WRITE_DISCARD : 기다리지 말고 복사해라
	//GPU가 다른 버퍼를 읽고있을때 임시로 버퍼 하나 생성해 복사한뒤, 작업이 끝나면 해당 복사된 버퍼 스위칭을
	//GPU가 알아서 처리한다.

	//Map함수는 GPU와의 연결을 잠깐 끊어주는 역할
	CDevice::GetInst()->GetContext()->Map(mBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &Map);

	//데이터를 Map에 복사하여 버퍼에 데이터를 채운다.
	memcpy(Map.pData, Data, mSize);

	//끊어진 연결을 다시 UnMap이용해 연결해서 버퍼를 GPU와 연결한다.
	CDevice::GetInst()->GetContext()->Unmap(mBuffer.Get(), 0);

	//저장된 상수버퍼를 Shader에 보내준다.

	if (mShaderBuffer & EShaderBufferType::Vertex)
	{
		CDevice::GetInst()->GetContext()->VSSetConstantBuffers(mRegister, 1, mBuffer.GetAddressOf());
	}
	if (mShaderBuffer & EShaderBufferType::Pixel)
	{
		CDevice::GetInst()->GetContext()->PSSetConstantBuffers(mRegister, 1, mBuffer.GetAddressOf());
	}
	if (mShaderBuffer & EShaderBufferType::Domain)
	{
		CDevice::GetInst()->GetContext()->DSSetConstantBuffers(mRegister, 1, mBuffer.GetAddressOf());
	}
	if (mShaderBuffer & EShaderBufferType::Hull)
	{
		CDevice::GetInst()->GetContext()->HSSetConstantBuffers(mRegister, 1, mBuffer.GetAddressOf());
	}
	if (mShaderBuffer & EShaderBufferType::Geometry)
	{
		CDevice::GetInst()->GetContext()->GSSetConstantBuffers(mRegister, 1, mBuffer.GetAddressOf());
	}
	if (mShaderBuffer & EShaderBufferType::Compute)
	{
		CDevice::GetInst()->GetContext()->CSSetConstantBuffers(mRegister, 1, mBuffer.GetAddressOf());
	}

}