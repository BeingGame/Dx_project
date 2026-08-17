#pragma once

#include "../Object.h"

//UI가 렌더링할때 필요한 데이터를 전부 저장할 구조체
struct FUIBrush
{
	std::weak_ptr<class CTexture> Texture;
	FVector4 Tint = FVector4::White;
	std::vector<FTextureFrame> AnimationFrames;
	bool AnimationEnable = false;
	EAnimation2DTextureType AnimationType = EAnimation2DTextureType::SpriteSheet;
	int Frame = 0;
	float Time = 0.f;
	float FrameTime = 0.f;
	float PlayRate = 1.f;
	float PlayTime = 0.f;

public:
	void PlayAnimation(float DeltaTime)
	{
		if (!AnimationEnable)
		{
			return;
		}

		Time += DeltaTime * PlayRate;

		if (Time >= FrameTime)
		{
			Time -= FrameTime;

			++Frame;

			if (Frame == (int)AnimationFrames.size())
			{
				Frame = 0;
			}
		}

	}
};

namespace EWidgetState
{
	enum Type
	{
		Normal,
		Hovered,
		Clicked,
		Release,
		Disable,
		End
	};
}

namespace EWidgetEventState
{
	enum Type
	{
		Hovered,
		Clicked,
		End
	};
}


//모든 위젯 클래스의 부모가 될 추상클래스
//버튼, 체크박스, 이미지위젯, 프로그레스바등 많은 위젯이 사용하는 속성이 다르기때문에
//상위로 위젯 추상클래스를 사용해서 매니저 클래스에서 관리하기 쉽게 만들어준다.
//다만 위젯 자체가 가지는 속성-> 크기, 위치, 렌더링용 메쉬와 셰이더등을 가진상태로 다른 위젯에 상속시켜준다.

class CWidget :
	public CObject
{
public:
	CWidget();
	CWidget(const CWidget& src);
	virtual ~CWidget() = 0;

protected:
	//모든 위젯이 공유해서 사용하는 행렬을 선언한다.
	//윈도우 좌표계와 동일한 행렬 좌표계를 만들어서
	//액터가 사용하는 행렬과는 다른 행렬을 상수버퍼에 올려서 셰이더를 통해 렌더링한다.
	static FMatrix mUIProjMat;

	//생성되는 모든 위젯에 고유한 순번을 부여하기 위한 카운터
	static unsigned long long mCreateSeqCounter;

public:
	static void CreateUIProjection(float Width, float Height)
	{
		mUIProjMat = DirectX::XMMatrixOrthographicOffCenterLH(0.f, Width, Height, 0.f, 0.f, 1000.f);
	}

	static const FMatrix& GetProjMat()
	{
		return mUIProjMat;
	}

protected:
	//위젯이 사용할 변수들을 선언한다.
	//위젯은 2D 좌표계를 사용해서
	//위치랑 UI가 사용할 셰이더를 따로 처리해야한다.

	//UI는 사각형 메쉬를 사용한다.
	//다만 사각형 메쉬만 사용하진 않지만, 충돌과 같은 연산에서
	//효율이 좋지않기 때문에 다른 도형 메쉬는 사용하지 않는다.

	std::weak_ptr<class CWorld> mWorld;
	std::weak_ptr<class CUIManager> mUIManager;

	//모든 위젯이 부모를 가질순 있지만
	//자식같은 경우에는 위젯마다 필요한 경우가 다르기 때문에
	//각 위젯에서 ChildList를 만들어준다.
	std::weak_ptr<CWidget> mParent;

	std::weak_ptr<class CMesh> mMesh;
	std::weak_ptr<class CShader> mShader;

	std::shared_ptr<class CCBufferTransform> mCBufferTransform;
	std::shared_ptr<class CCBufferUIDefault> mCBufferUIDefault;

	//위젯 속성
	std::string mName;
	FVector3 mPos;
	FVector3 mRenderPos;
	FVector3 mSize;
	FVector3 mPivot;

	//UI는 2D환경이기 때문에 회전은 한축으로만 이뤄진다.
	float mAngle = 0.f;

	//UI의 렌더링순서 값이 클수록 나중에 렌더링
	int mZOrder = 0;

	//위젯 생성 순번. ZOrder가 같은 위젯들의 정렬 순서를 고정하기 위한 타이브레이커.
	//std::sort는 불안정 정렬이라 ZOrder만 비교하면 동일 ZOrder 위젯들의
	//상대 순서가 매 프레임 뒤바뀌어 겹친 위젯이 깜빡인다.
	unsigned long long mCreateSeq = 0;

	//마우스가 현재 UI에 올라가있는지 확인한다.
	bool mMouseOn = false;

	//부모 컨테이너가 스크롤될 때 같이 밀려야 하는 위젯인지 여부.
	//타이틀바/배경처럼 고정되어야 하는 위젯은 false로 둔다.
	bool mScrollTarget = false;

	//부모 컨테이너가 매 프레임 넣어주는 스크롤 이동량.
	//mRenderPos 계산에 그대로 더해진다. (스크롤 대상이 아니면 항상 0)
	FVector3 mScrollOffset;

	FVector4 mWidgetColor = FVector4::White;

	EWidgetState::Type mWidgetState = EWidgetState::Normal;
	std::function<void()> mWidgetEventFunc[EWidgetEventState::End];

public:
	std::weak_ptr<CWidget> GetParent() const
	{
		return mParent;
	}

	const std::string GetName() const
	{
		return mName;
	}

	const FVector3& GetPos() const
	{
		return mPos;
	}

	const FVector3& GetRenderPos() const
	{
		return mRenderPos;
	}

	const FVector3& GetSize() const
	{
		return mSize;
	}

	//── 스크롤 연동 ────────────────────────────────────────────────────────
	//부모 컨테이너의 스크롤을 따라 움직일 위젯으로 표시한다.
	void SetScrollTarget(bool Target)
	{
		mScrollTarget = Target;

		if (!Target)
		{
			mScrollOffset = FVector3();
		}
	}

	bool IsScrollTarget() const
	{
		return mScrollTarget;
	}

	//부모 컨테이너 전용. 스크롤 이동량을 넣어준다.
	void SetScrollOffset(const FVector3& Offset)
	{
		mScrollOffset = Offset;
	}

	const FVector3& GetScrollOffset() const
	{
		return mScrollOffset;
	}

	const FVector3& GetPivot() const
	{
		return mPivot;
	}

	float GetAngle() const
	{
		return mAngle;
	}

	int GetZOrder() const
	{
		return mZOrder;
	}

	unsigned long long GetCreateSeq() const
	{
		return mCreateSeq;
	}

	const FVector4& GetWidgetColor() const
	{
		return mWidgetColor;
	}

	EWidgetState::Type GetWidgetState() const
	{
		return mWidgetState;
	}

	bool IsMouseOn() const
	{
		return mMouseOn;
	}

	void SetWidgetState(EWidgetState::Type State)
	{
		mWidgetState = State;
	}

	void SetWidgetEnable(bool Enable)
	{
		mWidgetState = Enable ? EWidgetState::Normal : EWidgetState::Disable;
	}

	void SetWorld(const std::weak_ptr<class CWorld>& World)
	{
		mWorld = World;
	}

	void SetUIManager(const std::weak_ptr<class CUIManager>& UIManager)
	{
		mUIManager = UIManager;
	}

	virtual void RemoveChild(std::shared_ptr<CWidget> Child);

	void Detach()
	{
		auto Parent = mParent.lock();

		if (Parent)
		{
			Parent->RemoveChild(GetThisPtr<CWidget>());

			mParent.reset();
		}
	}

	void SetParent(const std::weak_ptr<CWidget>& Parent)
	{
		Detach();

		mParent = Parent;
	}

	virtual void SetParentAll();

	void SetName(const std::string& Name)
	{
		mName = Name;
	}

	void AddPos(const FVector2& Move)
	{
		mPos.x += Move.x;
		mPos.y += Move.y;
	}

	void SetPos(const FVector3& Pos)
	{
		mPos.x = Pos.x;
		mPos.y = Pos.y;
	}

	void SetPos(float x, float y)
	{
		mPos.x = x;
		mPos.y = y;
	}

	virtual void SetSize(const FVector3& Size)
	{
		mSize = Size;
	}

	virtual void SetSize(float x, float y)
	{
		mSize.x = x;
		mSize.y = y;
	}

	void SetPivot(const FVector3& Pivot)
	{
		mPivot = Pivot;
	}

	void SetPivot(float x, float y)
	{
		mPivot.x = x;
		mPivot.y = y;
	}

	void SetAngle(float Angle)
	{
		//정규화 한다.
		mAngle = Angle;

		mAngle = fmodf(mAngle, 360.f);

		if (mAngle < 0.f)
		{
			mAngle += 360.f;
		}
	}

	virtual void SetZOrder(int ZOrder)
	{
		mZOrder = ZOrder;
	}

	void SetWidgetColor(const FVector4& Color)
	{
		mWidgetColor = Color;
	}

	void SetWidgetColor(float r, float g, float b, float a)
	{
		mWidgetColor = FVector4(r, g, b, a);
	}

	virtual void SetOpacity(float Opacity)
	{
		mWidgetColor.w = Opacity;
	}

	virtual void SetOpacityAll(float Opacity)
	{
		mWidgetColor.w = Opacity;
	}

public:
	//UI가 필요시 다른 셰이더를 사용할수도 있기 때문에
	//셰이더를 적용할수 있는 함수를 만들어준다.
	void SetShader(const std::weak_ptr<class CShader>& Shader);
	void SetShader(const std::string& Name);

public:
	virtual bool Init();
	virtual void Update(float DeltaTime);
	virtual void Render();
	virtual void Destory();

	//UI충돌
	virtual bool CollisionMouse(std::weak_ptr<CWidget>& Result, const FVector2& MousePos);
	virtual void MouseHovered();
	virtual void MouseUnHovered();
	virtual void MouseClicked();
	virtual void MouseRelease();

	virtual CWidget* Clone() = 0;

protected:
	//렌더링이 불필요한 위젯들은 렌더링할 필요가 없기 때문에
	//함수를 따로 둬서 렌더링해야되는 위젯에서 호출하게 만들어준다.
	void RenderBrush(const FUIBrush& Brush, const FVector3& RenderPos, const FVector3& Size);
	//애니메이션일 경우
	void RenderBrush(const FUIBrush& Brush, const FVector3& RenderPos, const FVector3& Size, const FVector2& FrameStart,
		const FVector2& FrameSize);

public:
	template<typename T>
	void SetWidgetEventFunc(EWidgetEventState::Type Type, T* Object, void(T::* Func)())
	{
		mWidgetEventFunc[Type] = std::bind(Func, Object);
	}

protected:
	//렌더링같은 경우에는 후순위가 화면에 나타나기 때문에
	static bool SortRender(const std::shared_ptr<CWidget>& Src, const std::shared_ptr<CWidget>& Dest);
	//ZOrder가장 큰 위젯이 가장 먼저 충돌연산을 진행한다.
	static bool SortCollision(const std::shared_ptr<CWidget>& Src, const std::shared_ptr<CWidget>& Dest);

};

