#pragma once

#include "../EngineInfo.h"

class CTile
{
public:
	CTile();
	CTile(const CTile& src);
	CTile(CTile&& src) noexcept;
	~CTile();

private:
	ETileType mType = ETileType::Normal;

	//타일의 위치정보
	FVector2 mPos;
	FVector2 mSize;
	FVector2 mCenter;

	int mIndexX = 0;
	int mIndexY = 0;
	int mIndex = 0;

	//텍스처에서 사용할 프레임 정보
	int mTextureFrame = -1;
	FVector2 mFrameStart;
	FVector2 mFrameEnd;

	//텍스처 렌더링에 대한 불변수
	bool mRender = false;
	//타일의 위치 표시
	bool mOutLineRender = false;
	FVector4 mOutLineColor = FVector4::White;

public:
	ETileType GetType() const
	{
		return mType;
	}

	const FVector2& GetPos() const
	{
		return mPos;
	}

	const FVector2& GetSize() const
	{
		return mSize;
	}

	const FVector2& GetCenter() const
	{
		return mCenter;
	}

	const FVector2& GetFrameStart() const
	{
		return mFrameStart;
	}

	const FVector2& GetFrameEnd() const
	{
		return mFrameEnd;
	}

	bool IsRender() const
	{
		return mRender;
	}

	bool IsOutLineRender() const
	{
		return mOutLineRender;
	}

	int GetIndex() const
	{
		return mIndex;
	}

	const FVector4& GetOutLineColor() const
	{
		return mOutLineColor;
	}

	void SetTileType(ETileType Type)
	{
		mType = Type;
	}

	void SetPos(const FVector2& Pos)
	{
		mPos = Pos;
	}

	void SetPos(float x, float y)
	{
		mPos = FVector2(x, y);
	}

	void SetSize(const FVector2& Size)
	{
		mSize = Size;
	}

	void SetSize(float x, float y)
	{
		mSize = FVector2(x, y);
	}

	void SetCenter(const FVector2& Center)
	{
		mCenter = Center;
	}

	void SetCenter(float x, float y)
	{
		mCenter = FVector2(x, y);
	}

	void SetFrame(const FVector2& Start, const FVector2& End)
	{
		mFrameStart = Start;
		mFrameEnd = End;

		mRender = true;
	}

	void SetFrame(float StartX, float StartY, float EndX, float EndY)
	{
		mFrameStart = FVector2(StartX, StartY);
		mFrameEnd = FVector2(EndX, EndY);

		mRender = true;
	}

	void SetIndex(int x, int y, int Index)
	{
		mIndexX = x;
		mIndexY = y;
		mIndex = Index;
	}

	void SetTextureFrame(int TextureFrame)
	{
		mTextureFrame = TextureFrame;
	}

	void SetOutLineRender(bool Render)
	{
		mOutLineRender = Render;
	}

	void SetOutLineColor(const FVector4& Color)
	{
		mOutLineColor = Color;
	}

	void SetOutLineColor(float r, float g, float b, float a)
	{
		mOutLineColor = FVector4(r, g, b, a);
	}

public:
	void Save(std::ofstream& File);
	void Load(std::ifstream& File);

};

