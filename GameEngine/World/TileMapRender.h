#pragma once

#include "SceneComponent.h"

//타일 맵 렌더
//타일맵 컴포넌트에게 정보를 받아서 타일을 출력해준다.
//그외의 백그라운드 이미지나, 블렌드스테이드, 텍스처 종류를 들고있게한다.
//렌더링을 주로 해줄 컴포넌트

class CTileMapRender :
    public CSceneComponent
{
public:
	CTileMapRender();
	CTileMapRender(const CTileMapRender& src);
	CTileMapRender(CTileMapRender&& src) noexcept;
	virtual ~CTileMapRender();

protected:
	std::weak_ptr<class CMesh> mBackMesh;
	std::weak_ptr<class CShader> mBackShader;
	std::weak_ptr<class CTexture> mTexture[ETileTextureType::End];
	//백 메쉬
	std::weak_ptr<class CRenderState> mAlphaBlend;
	//타일 메쉬
	std::weak_ptr<class CRenderState> mTileAlphaBlend;
	std::weak_ptr<class CTileMapComponent> mTileMap;

	std::shared_ptr<class CCBufferTransform> mCBufferTransform;
	std::shared_ptr<class CCBufferMaterial> mCBufferMaterial;

public:
	std::weak_ptr<class CTexture> GetTexture(ETileTextureType::Type Type) const
	{
		return mTexture[Type];
	}

	void SetTileMapComponent(const std::weak_ptr<class CTileMapComponent>& TileMap);
	void EnableAlphaBlend();
	void EnableTileAlphaBlend();

	bool SetTexture(ETileTextureType::Type Type, const std::weak_ptr<class CTexture>& Texture);
	bool SetTexture(ETileTextureType::Type Type, const std::string& Name);
	bool SetTexture(ETileTextureType::Type Type, const std::string& Name, const TCHAR* FileName, const std::string& PathName = "Texture");
	bool SetTextureFullPath(ETileTextureType::Type Type, const std::string& Name, const TCHAR* FullPath);
	bool SetTextureFullPath(ETileTextureType::Type Type, const std::string& Name, const std::vector<const TCHAR*>& FullPath);

	void SetBackMesh(const std::string& Name);
	void SetBackMesh(const std::weak_ptr<class CMesh>& Mesh);
	void SetBackShader(const std::string& Name);
	void SetBackShader(const std::weak_ptr<class CShader>& Shader);

	void SetTileFrameSize(float x, float y);
	void AddTileFrame(const FVector2& Start, const FVector2& End);
	void AddTileFrame(float StartX , float StartY, float EndX, float EndY);

public:
	virtual bool Init();
	virtual void Render();

public:
	virtual CTileMapRender* Clone() const;

public:
	void Save(std::ofstream& File);
	void Load(std::ifstream& File);

};

