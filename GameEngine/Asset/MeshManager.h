#pragma once

#include "AssetSubManager.h"
#include "Mesh.h"

class CMeshManager :
    public CAssetSubManager
{
public:
	CMeshManager();
	virtual ~CMeshManager();

private:
	std::unordered_map<std::string, std::shared_ptr<CMesh>> mMeshMap;

public:
	virtual bool Init();
	bool CreateMesh(const std::string& Name, void* VertexData, int VertexSize, int VertexCount, D3D11_USAGE VertexUsage, D3D11_PRIMITIVE_TOPOLOGY Primitive, void* IndexData = nullptr, int IndexSize = 0, int IndexCount = 0, DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN, D3D11_USAGE IndexUsage = D3D11_USAGE_DEFAULT);

	std::weak_ptr<CMesh> FindMesh(const std::string& Name);

	std::vector<std::string> GetMeshNames() const
	{
		static const char* kSkip[] = { "Mesh_UITexRect", "Mesh_LBTexRect", "Mesh_LBFrameRect" };
		std::vector<std::string> Names;
		for (auto& kv : mMeshMap)
		{
			bool Skip = false;
			for (auto& S : kSkip) if (kv.first == S) { Skip = true; break; }
			if (!Skip && kv.first.size() > 5)
				Names.push_back(kv.first.substr(5));
		}
		std::sort(Names.begin(), Names.end());
		return Names;
	}
};

