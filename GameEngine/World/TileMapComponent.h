#pragma once

#include "ActorComponent.h"
#include "Tile.h"

//타일맵 컴포넌트
//렌더링될 타일의 정보를 가지고있는 컴포넌트
//정보만 가지고 있기 때문에 액터컴포넌트를 상속받는다.

//타일의 렌더링에 대한 정보도 포함하여 전부 가지고 있게 설계한다.

class CTileMapComponent :
	public CActorComponent
{
public:
	CTileMapComponent();
	CTileMapComponent(const CTileMapComponent& src);
	CTileMapComponent(CTileMapComponent&& src) noexcept;
	virtual ~CTileMapComponent();

protected:
	//컴포넌트가 가지고 있는 타일 정보 배열
	std::vector<std::shared_ptr<class CTile>> mTileList;
	//해당 컴포넌트의 타일 렌더링 타입
	ETileShape mShape = ETileShape::Rect;
	//타일 하나의 크기
	FVector2 mTileSize;
	//타일 컴포넌트가 출력할 타일맵의 크기
	FVector2 mMapSize;

	//타일맵의 x,y축 크기
	int mCountX = 0;
	int mCountY = 0;

	//외곽선 출력을 위한 변수
	bool mTileOutLineRender = false;
	std::weak_ptr<class CMesh> mOutLineMesh;
	std::weak_ptr<class CShader> mOutLineShader;
	//타일맵 텍스처를 출력하기 위한 변수
	std::weak_ptr<class CMesh> mTileMesh;
	std::weak_ptr<class CShader> mTileShader;

	std::shared_ptr<class CCBufferTransform> mCBufferTransform;
	std::shared_ptr<class CCBufferCollider> mCBufferCollider;
	std::shared_ptr<class CCBufferTileMap> mCBufferTileMap;

	//타일 텍스처의 자체적인 크기
	FVector2 mTileTextureSize;

	std::vector<FTileFrame> mTileFrame;

	std::weak_ptr<class CTileMapRender> mTileMapRender;

	//현재 카메라에 들어와있는 구간을 나타낼 위치 변수
	//카메라에 포함된 타일만 렌더링하기 위해서 사용한다.
	int mViewStartX = 0;
	int mViewStartY = 0;
	int mViewEndX = 0;
	int mViewEndY = 0;

public:
	ETileShape GetShape() const
	{
		return mShape;
	}

	const FVector2& GetTileSize() const
	{
		return mTileSize;
	}

	const FVector2& GetMapSize() const
	{
		return mMapSize;
	}

	int GetTileCountX() const
	{
		return mCountX;
	}

	int GetTileCountY() const
	{
		return mCountY;
	}

	const FVector2& GetTileTextureSize() const
	{
		return mTileTextureSize;
	}

	FTileFrame GetTileFrame(int Index) const
	{
		if (Index < 0 || Index >= mTileFrame.size())
		{
			return FTileFrame();
		}

		return mTileFrame[Index];
	}

	int GetViewStartX() const
	{
		return mViewStartX;
	}

	int GetViewStartY() const
	{
		return mViewStartY;
	}

	int GetViewEndX() const
	{
		return mViewEndX;
	}

	int GetViewEndY() const
	{
		return mViewEndY;
	}

	int GetTileFrameCount() const
	{
		return (int)mTileFrame.size();
	}

	ETileType GetTileType(int Index) const;
	int GetTileIndex(const FVector2& Pos) const;
	int GetTileIndex(const FVector3& Pos) const;
	int GetTileIndex(float x, float y) const;

	std::weak_ptr<class CTile> GetTile(const FVector2& Pos) const;
	std::weak_ptr<class CTile> GetTile(const FVector3& Pos) const;
	std::weak_ptr<class CTile> GetTile(float x, float y) const;
	std::weak_ptr<class CTile> GetTile(int Index) const;

	int GetTileIndexX(const FVector2& Pos) const;
	int GetTileIndexY(const FVector2& Pos) const;

	const std::vector<std::shared_ptr<class CTile>>& GetTileList() const
	{
		return mTileList;
	}

	std::weak_ptr<class CMesh> GetTileMesh() const
	{
		return mTileMesh;
	}

	std::weak_ptr<class CShader> GetTileShader() const
	{
		return mTileShader;
	}

	std::weak_ptr<class CMesh> GetOutLineMesh() const
	{
		return mOutLineMesh;
	}

	std::weak_ptr<class CShader> GetOutLineShader() const
	{
		return mOutLineShader;
	}

	std::shared_ptr<class CCBufferTransform> GetCBufferTransform() const
	{
		return mCBufferTransform;
	}

	std::shared_ptr<class CCBufferTileMap> GetCBufferTileMap() const
	{
		return mCBufferTileMap;
	}

	std::shared_ptr<class CCBufferCollider> GetCBufferCollider() const
	{
		return mCBufferCollider;
	}

public:
	void SetTileOutLineRender(bool Enable);
	void SetTileOutLineRender(bool Enable, int Index);

	void SetOutLineMesh(const std::string& Name);
	void SetOutLineShader(const std::string& Name);

	void SetTileMesh(const std::string& Name);
	void SetTileShader(const std::string& Name);

	bool SetTileTexture(ETileTextureType::Type Type, const std::weak_ptr<class CTexture>& Texture);
	bool SetTileTexture(ETileTextureType::Type Type, const std::string& Name);
	bool SetTileTexture(ETileTextureType::Type Type, const std::string& Name, const TCHAR* FileName, const std::string& PathName = "Texture");

	void AddTileFrame(const FVector2& Start, const FVector2& End);
	void AddTileFrame(float StartX, float StartY, float EndX, float EndY);

	void SetTileFrameAll(int FrameIndex);
	void SetTileFrame(int Index, int FrameIndex);

	void SetTileMapRender(const std::weak_ptr<class CTileMapRender>& Render)
	{
		mTileMapRender = Render;
	}

	void SetTileTextureSize(const FVector2& Size)
	{
		mTileTextureSize = Size;
	}

	void SetTileTextureSize(float x, float y)
	{
		mTileTextureSize.x = x;
		mTileTextureSize.y = y;
	}

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void PostUpdate(float DeltaTime);

public:
	virtual CTileMapComponent* Clone() const
	{
		return new CTileMapComponent(*this);
	}

	virtual std::string GetTypeName() const override { return "CTileMapComponent"; }

public:
	//지정된 갯수의 타일을 생성해준다.
	void CreateTile(ETileShape Shape, int CountX, int CountY, const FVector2& TileSize, int TileTextureFrame = -1, bool OutLineRender = false);

public:
	void Save(const TCHAR* FileName, const std::string& PathName = "Asset");
	void SaveFullPath(const TCHAR* FullPath);
	void Load(const TCHAR* FileName, const std::string& PathName = "Asset");
	void LoadFullPath(const TCHAR* FullPath);

private:
	int GetTileRenderIndexX(const FVector2& Pos) const;
	int GetTileRenderIndexY(const FVector2& Pos) const;
};

