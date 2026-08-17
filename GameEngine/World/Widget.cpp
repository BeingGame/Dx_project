#include "Widget.h"

#include "../Asset/CBufferTransform.h"
#include "../Asset/CBufferUIDefault.h"

#include "../Asset/AssetManager.h"
#include "../Asset/Shader.h"
#include "../Asset/ShaderManager.h"

#include "../Asset/Texture.h"
#include "../Asset/TextureManager.h"

#include "../Asset/Mesh.h"
#include "../Asset/MeshManager.h"

#include "Collision.h"

FMatrix CWidget::mUIProjMat;
unsigned long long CWidget::mCreateSeqCounter = 0;

CWidget::CWidget()
{
	mCreateSeq = ++mCreateSeqCounter;
}

CWidget::CWidget(const CWidget& src)
{
	//복제본은 원본과 같은 순번을 쓰면 안 된다. (동률 → 정렬 흔들림)
	mCreateSeq = ++mCreateSeqCounter;

	mEnable = src.mEnable;
	mAlive = src.mAlive;
	mWorld = src.mWorld;
	mUIManager = src.mUIManager;
	mMesh = src.mMesh;
	mShader = src.mShader;
	mName = src.mName;
	mPos = src.mPos;
	mRenderPos = src.mRenderPos;
	mSize = src.mSize;
	mPivot = src.mPivot;
	mAngle = src.mAngle;
	mZOrder = src.mZOrder;
	mMouseOn = src.mMouseOn;
	mWidgetColor = src.mWidgetColor;
	mScrollTarget = src.mScrollTarget;
	mScrollOffset = src.mScrollOffset;

	mCBufferTransform = std::shared_ptr<CCBufferTransform>(src.mCBufferTransform->Clone());
	mCBufferTransform->Init();

	mCBufferUIDefault = std::shared_ptr<CCBufferUIDefault>(src.mCBufferUIDefault->Clone());
	mCBufferUIDefault->Init();
}

CWidget::~CWidget()
{}

void CWidget::RemoveChild(std::shared_ptr<CWidget> Child)
{}

void CWidget::SetParentAll()
{}

void CWidget::SetShader(const std::weak_ptr<class CShader>& Shader)
{
	mShader = Shader;
}

void CWidget::SetShader(const std::string& Name)
{
	auto ShaderMgr = CAssetManager::GetInst()->GetSubManager<CShaderManager>(EAssetType::Shader);

	if (ShaderMgr)
	{
		mShader = ShaderMgr->FindShader(Name);
	}
}

bool CWidget::Init()
{
	auto MeshMgr = CAssetManager::GetInst()->GetSubManager<CMeshManager>(EAssetType::Mesh);

	if (MeshMgr)
	{
		mMesh = MeshMgr->FindMesh("UITexRect");

		if (mMesh.expired())
		{
			return false;
		}
	}
	SetShader("UIDefault");

	mCBufferTransform = std::make_shared<CCBufferTransform>();

	if (!mCBufferTransform->Init())
	{
		return false;
	}

	mCBufferUIDefault = std::make_shared<CCBufferUIDefault>();

	if (!mCBufferUIDefault->Init())
	{
		return false;
	}

	return true;
}

void CWidget::Update(float DeltaTime)
{}

void CWidget::Render()
{
	auto Parent = mParent.lock();

	//mScrollOffset은 스크롤 대상 위젯에만 부모가 채워준다. 그 외에는 항상 0이다.
	if (Parent)
	{
		mRenderPos = Parent->mRenderPos + mPos + mScrollOffset;
	}
	else
	{
		mRenderPos = mPos + mScrollOffset;
	}

	mCBufferUIDefault->SetWidgetColor(mWidgetColor);
}

void CWidget::Destory()
{
	mAlive = false;
}

bool CWidget::CollisionMouse(std::weak_ptr<CWidget>& Result, const FVector2& MousePos)
{
	//AABB랑 OBB충돌을 점충돌로 받아온다.

	//Update에서 변경된 위치 갱신
	auto Parent = mParent.lock();

	if (Parent)
	{
		mRenderPos = Parent->mRenderPos + mPos + mScrollOffset;
	}
	else
	{
		mRenderPos = mPos + mScrollOffset;
	}

	FVector3 HitPoint, Normal, LocalMousePos;
	float Depth = 0.f;

	LocalMousePos.x = MousePos.x;
	LocalMousePos.y = MousePos.y;

	//AABB충돌을 진행한다.
	if (mAngle == 0.f)
	{
		FBox2DInfo WidgetBox;

		//중심점은 0,0을 기준으로 좌측상단이 0.5,0.5가 된다.
		//RenderPos == 좌측상단
		//중심점 = RenderPos +(mSize * 0.5f);
		//ex)RenderPos(100,100) Size(200,200) -> Center(200,200)
		//pivot이 있을땐 달라진다
		//중심점 = RenderPos +(mSize * 0.5f);
		//ex)RenderPos(100,100) Size(200,200) Pivot(1,1)(정점데이터)
		//중심점 = RenderPos + (mSize * 0.5f) - (mSize * mPivot)
		//중심점 = RenderPos +mSize(0.5f - mPivot)
		WidgetBox.Center = mRenderPos + mSize * (FVector3(0.5f, 0.5f, 0.f) - mPivot);

		FVector2 Size;
		Size.x = mSize.x * 0.5f;
		Size.y = mSize.y * 0.5f;
		WidgetBox.Halfsize = Size;

		if (CCollision::CollisionPointToAABB2D(HitPoint, Normal, Depth, WidgetBox, LocalMousePos))
		{
			Result = GetThisPtr<CWidget>();
			mMouseOn = true;
			return true;
		}
		else
		{
			if (mMouseOn)
			{
				mMouseOn = false;
			}

			return false;
		}
	}
	//OBB충돌을 진행한다.
	else
	{
		//회전이 있을경우, Pivot이 중심점인 0.5,0.5 라면 문제없이 계산을 바로 진행할수 있다.
		FVector3 Axis[2];

		FBox2DInfo WidgetBox;

		FVector2 Size;
		Size.x = mSize.x * 0.5f;
		Size.y = mSize.y * 0.5f;
		WidgetBox.Halfsize = Size;

		//축을 위젯의 각도만큼 회전시켜준다.
		WidgetBox.Axis[0] = FVector3::Axis[EAxis::X];
		WidgetBox.Axis[1] = FVector3::Axis[EAxis::Y];

		FMatrix RotMat;
		RotMat.RotationZ(mAngle);

		WidgetBox.Axis[0] = Axis[0].TransformNormal(RotMat);
		WidgetBox.Axis[1] = Axis[1].TransformNormal(RotMat);

		WidgetBox.Axis[0].Normalize();
		WidgetBox.Axis[1].Normalize();

		//변경된 축을 이용해서 Center를 구해준다.
		FVector3 Pivot = FVector3(0.5f, 0.5f, 0.f) - mPivot;

		WidgetBox.Center = mRenderPos + WidgetBox.Axis[0] * (mSize * Pivot.x) + WidgetBox.Axis[1] * (mSize.y * mPivot.y);

		if (CCollision::CollisionPointToOBB2D(HitPoint, Normal, Depth, WidgetBox, LocalMousePos))
		{
			Result = GetThisPtr<CWidget>();
			mMouseOn = true;
			return true;
		}
		else
		{
			if (mMouseOn)
			{
				mMouseOn = false;
			}
			return false;
		}

	}
}

void CWidget::MouseHovered()
{
	if (mWidgetEventFunc[EWidgetEventState::Hovered])
	{
		mWidgetEventFunc[EWidgetEventState::Hovered]();
	}

	SetWidgetState(EWidgetState::Hovered);
}

void CWidget::MouseUnHovered()
{
	SetWidgetState(EWidgetState::Normal);
}

void CWidget::MouseClicked()
{
	if (mWidgetEventFunc[EWidgetEventState::Clicked])
	{
		mWidgetEventFunc[EWidgetEventState::Clicked]();
	}

	SetWidgetState(EWidgetState::Clicked);
}

void CWidget::MouseRelease()
{
	//마우스가 호버된 상태에서 마우스버튼을 눌렀다 땜.
	SetWidgetState(EWidgetState::Release);
}

void CWidget::RenderBrush(const FUIBrush& Brush, const FVector3& RenderPos, const FVector3& Size)
{
	//렌더링
	auto Mesh = mMesh.lock();
	auto Shader = mShader.lock();

	if (!Mesh || !Shader)
	{
		return;
	}

	FMatrix ScaleMat, RotMat, TranslationMat, WorldMat;

	ScaleMat.Scaling(Size);
	RotMat.RotationZ(mAngle);
	TranslationMat.Translation(RenderPos);

	WorldMat = ScaleMat * RotMat * TranslationMat;

	//UI에서는 뷰행렬이 필요없다.

	mCBufferTransform->SetWorldMatrix(WorldMat);
	mCBufferTransform->SetViewMatrix(FMatrix::IdentityMatrix);
	mCBufferTransform->SetProjMatrix(mUIProjMat);

	mCBufferTransform->SetPivotSize(mPivot);

	mCBufferTransform->UpdateBuffer();

	mCBufferUIDefault->SetBrushTint(Brush.Tint);

	FVector2 TexSize;

	if (!Brush.Texture.expired())
	{
		mCBufferUIDefault->SetTextureEnable(true);

		auto Texture = Brush.Texture.lock();

		TexSize.x = (float)Texture->GetTexture()->Width;
		TexSize.y = (float)Texture->GetTexture()->Height;

		if (Brush.AnimationType == EAnimation2DTextureType::Frame)
		{
			Texture->SetShader(0, EShaderBufferType::Pixel, Brush.Frame);
		}
		else
		{
			Texture->SetShader(0, EShaderBufferType::Pixel, 0);
		}
	}
	else
	{
		mCBufferUIDefault->SetTextureEnable(false);
	}

	if (Brush.AnimationEnable)
	{
		mCBufferUIDefault->SetAnimationEnable(true);

		int Frame = Brush.Frame;

		FTextureFrame FrameInfo = Brush.AnimationFrames[Frame];

		mCBufferUIDefault->SetLTUV(FrameInfo.Start / TexSize);

		FVector2 FrameRB;
		FrameRB.x = FrameInfo.Start.x + FrameInfo.Size.x;
		FrameRB.y = FrameInfo.Start.y + FrameInfo.Size.y;

		mCBufferUIDefault->SetRBUV(FrameRB / TexSize);
	}
	else
	{
		mCBufferUIDefault->SetAnimationEnable(false);
	}

	mCBufferUIDefault->UpdateBuffer();

	Shader->SetShader();

	Mesh->Render();
}

void CWidget::RenderBrush(const FUIBrush& Brush, const FVector3& RenderPos, const FVector3& Size, const FVector2& FrameStart, const FVector2& FrameSize)
{
	//렌더링
	auto Mesh = mMesh.lock();
	auto Shader = mShader.lock();

	if (!Mesh || !Shader)
	{
		return;
	}

	FMatrix ScaleMat, RotMat, TranslationMat, WorldMat;

	ScaleMat.Scaling(Size);
	RotMat.RotationZ(mAngle);
	TranslationMat.Translation(RenderPos);

	WorldMat = ScaleMat * RotMat * TranslationMat;

	//UI에서는 뷰행렬이 필요없다.

	mCBufferTransform->SetWorldMatrix(WorldMat);
	mCBufferTransform->SetViewMatrix(FMatrix::IdentityMatrix);
	mCBufferTransform->SetProjMatrix(mUIProjMat);

	mCBufferTransform->SetPivotSize(mPivot);

	mCBufferTransform->UpdateBuffer();

	mCBufferUIDefault->SetBrushTint(Brush.Tint);

	FVector2 TexSize;

	if (!Brush.Texture.expired())
	{
		mCBufferUIDefault->SetTextureEnable(true);

		auto Texture = Brush.Texture.lock();

		TexSize.x = (float)Texture->GetTexture()->Width;
		TexSize.y = (float)Texture->GetTexture()->Height;

		if (Brush.AnimationType == EAnimation2DTextureType::Frame)
		{
			Texture->SetShader(0, EShaderBufferType::Pixel, Brush.Frame);
		}
		else
		{
			Texture->SetShader(0, EShaderBufferType::Pixel, 0);
		}
	}
	else
	{
		mCBufferUIDefault->SetTextureEnable(false);
	}

	mCBufferUIDefault->SetAnimationEnable(true);

	mCBufferUIDefault->SetLTUV(FrameStart / TexSize);

	FVector2 FrameRB;
	FrameRB.x = FrameStart.x + FrameSize.x;
	FrameRB.y = FrameStart.y + FrameSize.y;

	mCBufferUIDefault->SetRBUV(FrameRB / TexSize);

	mCBufferUIDefault->UpdateBuffer();

	Shader->SetShader();

	Mesh->Render();
}

bool CWidget::SortRender(const std::shared_ptr<class CWidget>& Src, const std::shared_ptr<class CWidget>& Dest)
{
	//ZOrder가 같으면 생성 순번으로 순서를 고정한다.
	//타이브레이커가 없으면 불안정 정렬 때문에 매 프레임 순서가 뒤바뀌어
	//같은 ZOrder로 겹쳐놓은 위젯(타이틀 바 위의 버튼 등)이 깜빡인다.
	if (Src->GetZOrder() != Dest->GetZOrder())
	{
		return Src->GetZOrder() < Dest->GetZOrder();
	}

	return Src->GetCreateSeq() < Dest->GetCreateSeq();
}

bool CWidget::SortCollision(const std::shared_ptr<class CWidget>& Src, const std::shared_ptr<class CWidget>& Dest)
{
	//렌더 순서의 정확한 역순이어야 나중에 그려진(위에 보이는) 위젯이 먼저 히트된다.
	if (Src->GetZOrder() != Dest->GetZOrder())
	{
		return Src->GetZOrder() > Dest->GetZOrder();
	}

	return Src->GetCreateSeq() > Dest->GetCreateSeq();
}
