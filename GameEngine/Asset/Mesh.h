#pragma once

#include "Asset.h"


//메쉬의 슬롯 정보를 저장할 구조체
struct FMeshSlot
{
	FIndexBuffer IndexBuffer;
	std::shared_ptr<class CMaterial> Material;
};

//메쉬
//그래픽에서 3D로 표현되는 물체
//정점, 부가적인 데이터들(컬러,UV)

class CMesh :
    public CAsset
{
public:
	CMesh();
	virtual ~CMesh();

protected:
	//정점 버퍼가 들고있는 데이터들을 토대로 렌더링할수 있는 메쉬를 생성한다.
	FVertexBuffer mVertexBuffer;


	//메쉬의 인덱스 버퍼도 만들어준다.
	//좀더 효율적으로 삼각형을 그리기 위해 사용된다.
	//FIndexBuffer mIndexBuffer;
	std::vector<std::shared_ptr<FMeshSlot>> mMeshSlot;


	//기본도형(프리미티브)에 대한 정보를 저장한다.
	D3D11_PRIMITIVE_TOPOLOGY mPrimitive = D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

public:
	std::vector<std::shared_ptr<FMeshSlot>> GetMeshSlot() const
	{
		return mMeshSlot;
	}

public:
	//메쉬 생성을 함수
	//버텍스 데이터랑 인덱스데이터를 void 타입으로 받는다.
	//D3D11_USAGE: 
	/*
	 D3D11_USAGE_DEFAULT	= 0, // GPU 전용 읽기, 쓰기 가능 CPU는 읽기 쓰기불가능
	 D3D11_USAGE_IMMUTABLE	= 1, // 불변, GPU읽기 가능, 쓰기는 불가능 정보를 한번 넣고 바꾸지 않을 경우에 사용된다.
	 D3D11_USAGE_DYNAMIC	= 2, //GPU 읽기 가능, CPU 쓰기 가능, 정보를 자주 업데이트 해야되는 경우 사용된다.
	 D3D11_USAGE_STAGING	= 3 // CPU가 읽기 쓰기 가능, GPU 사용하지 않는다. 메모리를 확보하고 데이터를 저장하는 용도로 사용된다.
	
	*/
	bool CreateMesh(void* VertexData, int VertexSize, int VertexCount, D3D11_USAGE VertexUsage, D3D11_PRIMITIVE_TOPOLOGY Primitive, void* IndexData = nullptr, int IndexSize = 0, int IndexCount = 0, DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN, D3D11_USAGE IndexUsage = D3D11_USAGE_DEFAULT);

	bool CreateBuffer(ID3D11Buffer** Buffer, D3D11_BIND_FLAG Flag, void* Data, int Size, int Count, D3D11_USAGE Usage);

	void Render();
	void Render(int SlotIndex);

};

