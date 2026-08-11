#pragma once


enum class EAssetType
{
	None =-1,
	Mesh,
	Shader,
	Texture,
	Material,
	Animation2D,
	Sound,
	Font,
	FontCollection
};

enum class EColliderType
{
	Box2D,
	Sphere2D
};

//콜리전 채널 종류
namespace ECollisionChannel
{
	enum Type
	{
		Static,
		Player,
		Monster,
		Custom1,
		Custom2,
		Custom3,
		Custom4,
		Custom5,
		Custom6,
		Custom7,
		Custom8,
		Custom9,
		End
	};
}

namespace ECollisionInteraction
{
	enum Type
	{
		Ignore,
		Overlap,
		Block,
		End
	};
}

//애니메이션의 텍스처 타입
enum class EAnimation2DTextureType
{
	None = -1,
	SpriteSheet,
	Frame
};

//사운드의 차원
enum class ESoundDimension
{
	Mode2D,
	Mode3D,
	Mode3D_Linear
};

//마우스의 상태
//State라는 이름을 붙여서 게임에서 어떤 상황에서 커서를 사용할지 모르기 때문에
//포괄적인 열거형 이름을 지어준다.
namespace EMouseState
{
	enum Type
	{
		Normal,
		State1,
		State2,
		State3,
		State4,
		State5,
		State6,
		State7,
		State8,
		State9,
		End
	};
}

//타일의 타입
//현재 타일이 이동가능한 타일인지, 불가능한 타일인지 결정하는 타입
enum class ETileType
{
	Normal,
	Wall,
	End
};

namespace ETileTextureType
{
	enum Type
	{
		Back,
		Tile,
		End
	};
};

enum class ETileShape
{
	Rect
};
