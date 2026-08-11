#include "Mesh.h"
#include "../Device.h"
#include "Material.h"
#include "MaterialManager.h"
#include "AssetManager.h"

CMesh::CMesh()
{
	mType = EAssetType::Mesh;
}

CMesh::~CMesh()
{

}

bool CMesh::CreateMesh(void* VertexData, int VertexSize, int VertexCount, D3D11_USAGE VertexUsage, D3D11_PRIMITIVE_TOPOLOGY Primitive, void* IndexData, int IndexSize, int IndexCount, DXGI_FORMAT Format, D3D11_USAGE IndexUsage)
{
	mVertexBuffer.Size = VertexSize;
	mVertexBuffer.Count = VertexCount;
	mPrimitive = Primitive;

	if (!CreateBuffer(mVertexBuffer.Buffer.GetAddressOf(), D3D11_BIND_VERTEX_BUFFER, VertexData, VertexSize, VertexCount, VertexUsage))
	{
		return false;
	}

	if (IndexData)
	{
		//새로운 메쉬슬롯을 만들고 등록한다.
		std::shared_ptr<FMeshSlot> Slot = std::make_shared<FMeshSlot>();

		Slot->IndexBuffer.Size = IndexSize;
		Slot->IndexBuffer.Count = IndexCount;
		Slot->IndexBuffer.Format = Format;

		auto MaterialMgr = CAssetManager::GetInst()->GetSubManager<CMaterialManager>(EAssetType::Material);

		if (MaterialMgr)
		{
			//기본 머티리얼을 Default로 가져와서 instance로 복제한다.
			Slot->Material = MaterialMgr->CreateMaterialInstance("Default");

			if (!CreateBuffer(Slot->IndexBuffer.Buffer.GetAddressOf(), D3D11_BIND_INDEX_BUFFER, IndexData, IndexSize, IndexCount, IndexUsage))
			{
				return false;
			}
		}

		mMeshSlot.push_back(Slot);
	}

	return true;
}

bool CMesh::CreateBuffer(ID3D11Buffer** Buffer, D3D11_BIND_FLAG Flag, void* Data, int Size, int Count, D3D11_USAGE Usage)
{
	//버퍼를 생성하기 위해 정보 구조체를 선언
	D3D11_BUFFER_DESC Desc = {};

	//버퍼의 전체 메모리 크기
	Desc.ByteWidth = Size * Count;
	Desc.Usage = Usage;
	Desc.BindFlags = Flag;


	if (Usage == D3D11_USAGE_DYNAMIC)
	{
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	else if (Usage == D3D11_USAGE_STAGING)
	{
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	}

	//버퍼의 정보를 담을 구조체를 선언해서 버퍼를 생성한다.
	D3D11_SUBRESOURCE_DATA BufferData = {};
	BufferData.pSysMem = Data;

	if (FAILED(CDevice::GetInst()->GetDevice()->CreateBuffer(&Desc, &BufferData, Buffer)))
	{
		return false;
	}

	return true;
}

void CMesh::Render()
{
	//메쉬 렌더링
	//버텍스버퍼를 출력하기위한 IA를 설정
	//버퍼의 크기랑 시작위치 지정한다.
	UINT Stride = mVertexBuffer.Size;
	UINT Offset = 0;

	CDevice::GetInst()->GetContext()->IASetPrimitiveTopology(mPrimitive);
	CDevice::GetInst()->GetContext()->IASetVertexBuffers(0, 1, mVertexBuffer.Buffer.GetAddressOf(), &Stride, &Offset);

	//인덱스 버퍼가 존재하면 인덱스 버퍼를 이용해서 메쉬를 생성
	//없다면 없을때 렌더링 함수 호출
	if (!mMeshSlot.empty())
	{

		size_t Size = mMeshSlot.size();

		for (size_t i = 0; i < Size; ++i)
		{
			CDevice::GetInst()->GetContext()->IASetIndexBuffer(mMeshSlot[i]->IndexBuffer.Buffer.Get(), mMeshSlot[i]->IndexBuffer.Format, 0);
			CDevice::GetInst()->GetContext()->DrawIndexed(mMeshSlot[i]->IndexBuffer.Count, 0, 0);
		}
	}
	else
	{
		CDevice::GetInst()->GetContext()->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
		CDevice::GetInst()->GetContext()->Draw(mVertexBuffer.Count, 0);
	}

}

void CMesh::Render(int SlotIndex)
{
	//메쉬 렌더링
	//버텍스버퍼를 출력하기위한 IA를 설정
	//버퍼의 크기랑 시작위치 지정한다.
	UINT Stride = mVertexBuffer.Size;
	UINT Offset = 0;

	CDevice::GetInst()->GetContext()->IASetPrimitiveTopology(mPrimitive);
	CDevice::GetInst()->GetContext()->IASetVertexBuffers(0, 1, mVertexBuffer.Buffer.GetAddressOf(), &Stride, &Offset);

	if (SlotIndex >= 0 && SlotIndex < mMeshSlot.size())
	{
		CDevice::GetInst()->GetContext()->IASetIndexBuffer(mMeshSlot[SlotIndex]->IndexBuffer.Buffer.Get(), mMeshSlot[SlotIndex]->IndexBuffer.Format, 0);
		CDevice::GetInst()->GetContext()->DrawIndexed(mMeshSlot[SlotIndex]->IndexBuffer.Count, 0, 0);
	}
	else
	{
		CDevice::GetInst()->GetContext()->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
		CDevice::GetInst()->GetContext()->Draw(mVertexBuffer.Count, 0);
	}
}
