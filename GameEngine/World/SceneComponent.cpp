#include "SceneComponent.h"

#include "Actor.h"

#include "../RenderManager.h"

CSceneComponent::CSceneComponent()
{
	mType = EComponentType::Scene;
}

CSceneComponent::CSceneComponent(const CSceneComponent& src)
	:CComponent(src)
{
	//컴포넌트를 복사했다고 해서 부모가 변경될순 없다.
	mParent.reset();
	mChildList.clear();
	
	//트랜스폼 복사
	mRelativePos = src.mRelativePos;
	mRelativeRot = src.mRelativeRot;
	mRelativeScale = src.mRelativeScale;
	mWorldPos = src.mWorldPos;
	mWorldRot = src.mWorldRot;
	mWorldScale = src.mWorldScale;
	mVelocity = src.mVelocity;

	memcpy(&mWorldAxis[0], &src.mWorldAxis[0], sizeof(FVector3) * EAxis::End);

}

CSceneComponent::CSceneComponent(CSceneComponent&& src) noexcept
	:CComponent(std::move(src))
{
	//컴포넌트를 복사했다고 해서 부모가 변경될순 없다.
	mParent = src.mParent;
	mChildList = src.mChildList;

	src.mParent.reset();
	src.mChildList.clear();

	//트랜스폼 복사
	mRelativePos = src.mRelativePos;
	mRelativeRot = src.mRelativeRot;
	mRelativeScale = src.mRelativeScale;
	mWorldPos = src.mWorldPos;
	mWorldRot = src.mWorldRot;
	mWorldScale = src.mWorldScale;
	mVelocity = src.mVelocity;

	memcpy(&mWorldAxis[0], &src.mWorldAxis[0], sizeof(FVector3) * EAxis::End);
}

CSceneComponent::~CSceneComponent()
{
	DetachFromParent();
}

void CSceneComponent::SetRenderLayer(int LayerOrder)
{
	CRenderManager::GetInst()->RemoveRenderObject(GetLayerOrder(), GetThisPtr<CSceneComponent>());

	mRenderLayerOrder = LayerOrder;

	CRenderManager::GetInst()->AddRenderLayer(GetThisPtr<CSceneComponent>());

}

void CSceneComponent::SetRenderLayer(const std::string & Name)
{
	CRenderManager::GetInst()->RemoveRenderObject(GetLayerOrder(), GetThisPtr<CSceneComponent>());

	mRenderLayerOrder = CRenderManager::GetInst()->GetLayerOrder(Name);

	CRenderManager::GetInst()->AddRenderLayer(GetThisPtr<CSceneComponent>());

}

void CSceneComponent::AddChild(std::weak_ptr<CSceneComponent> Child)
{
	auto _Child = Child.lock();

	if (_Child)
	{
		_Child->mParent = GetThisPtr<CSceneComponent>();
		mChildList.push_back(_Child);

		_Child->InheritScale();
		_Child->InheritRotation();
		_Child->InheritPos();
	}
}

void CSceneComponent::UpdateTransform()
{
	mScaleMat.Scaling(mWorldScale);
	mRotMat.Rotation(mWorldRot);
	mTranslateMat.Translation(mWorldPos);

	mWorldMat = mScaleMat * mRotMat * mTranslateMat;

	/*size_t Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Child = mChildList[i].lock();

		if (Child)
		{
			Child->UpdateTransform();
		}
	}*/
}

void CSceneComponent::AttachToActor(const std::weak_ptr<CActor> Actor)
{
	//부모를 변경하기전 이전 부모와의 연결을 끊는다.
	DetachFromParent();

	//액터의 root컴포넌트에 나 자신을 부착한다.
	auto _Actor = Actor.lock();

	if (_Actor)
	{
		auto OtherRoot = _Actor->GetRootComponent().lock();

		if (OtherRoot)
		{
			OtherRoot->AddChild(GetThisPtr<CSceneComponent>());
		}
	}
}

void CSceneComponent::DetachFromParent()
{
	//부모컴포넌트에서 나 자신을 제거한다.
	//ChildList에서 제거
	auto Parent = mParent.lock();

	if (Parent)
	{
		//ChildList로 나 자신을 찾아준다.
		auto iter = Parent->mChildList.begin();
		auto iterEnd = Parent->mChildList.end();

		for (; iter != iterEnd; ++iter)
		{
			//소멸자에서 detachformparent를 호출할떄
			//owenr_before : 현재 메모리 주소를 비교하는 함수
			//weak_ptr을 가져와 사용해준다.
			//enable shared from this를 사용하는 경우엔
			//weak_from_this를 사용해야한다.

			if (!iter->owner_before(weak_from_this()) && !weak_from_this().owner_before(*iter))
			{
				iter = Parent->mChildList.erase(iter);
				break;
			}

			//이후 부모를 nullptr로 변경한다.
			mParent.reset();
		}
	}
}

void CSceneComponent::NormalizeRot(FVector3& Rotation)
{
	//각도를 0~360도로 고정해준다.
	//실제로 작업되는 각도는 -180도 이더라도 우리가 실제로 사용하는 각도는 0~360도 이내

	//나머지 연산을 통해 0~360도로 고정
	Rotation.x = fmodf(Rotation.x, 360.f);
	Rotation.y = fmodf(Rotation.y, 360.f);
	Rotation.z = fmodf(Rotation.z, 360.f);

	//음수를 양수의 각도로 돌려준다.
	if (Rotation.x < 0.f)
	{
		Rotation.x += 360.f;
	}
	if (Rotation.y < 0.f)
	{
		Rotation.y += 360.f;
	}
	if (Rotation.z < 0.f)
	{
		Rotation.z += 360.f;
	}

}

bool CSceneComponent::Init()
{
	CComponent::Init();

	//씬 컴포넌트가 만들어지면 기본적으로 1번 렌더레이어에 등록해준다.
	SetRenderLayer(1);

	return true;
}

void CSceneComponent::Update(float DeltaTime)
{

	//자식 컴포넌트 업데이트를 진행한다.
	/*size_t Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Child = mChildList[i].lock();

		if (Child)
		{
			Child->Update(DeltaTime);
		}
	}*/

}

void CSceneComponent::PostUpdate(float DeltaTime)
{
	/*size_t Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Child = mChildList[i].lock();

		if (Child)
		{
			Child->PostUpdate(DeltaTime);
		}
	}*/
}

void CSceneComponent::Render()
{
	/*size_t Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Child = mChildList[i].lock();

		if (Child)
		{
			Child->Render();
		}
	}*/

	mVelocity = FVector3::Zero;
}

void CSceneComponent::PostRender()
{
	/*size_t Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Child = mChildList[i].lock();

		if (Child)
		{
			Child->PostRender();
		}
	}*/
}

void CSceneComponent::Destroy()
{
	CComponent::Destroy();

	//CRenderManager::GetInst()->RemoveRenderObject(GetLayerOrder(), std::dynamic_pointer_cast<CSceneComponent>(GetThisPtr()));

	/*size_t Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Child = mChildList[i].lock();

		if (Child)
		{
			Child->Destroy();
		}
	}*/

	
}

CSceneComponent* CSceneComponent::Clone() const
{
	// TODO: 여기에 return 문을 삽입합니다.
	return new CSceneComponent(*this);
}

void CSceneComponent::SetInheritScale(bool Scale)
{
	mInheritScale = Scale;
}

void CSceneComponent::SetInheritRot(bool Rot)
{
	mInheritRot = Rot;
}

void CSceneComponent::SetRelativeScale(const FVector3& Scale)
{
	mRelativeScale = Scale;

	InheritScale();
}

void CSceneComponent::SetRelativeScale(const FVector2& Scale)
{
	SetRelativeScale(FVector3(Scale.x, Scale.y, mRelativeScale.z));
}

void CSceneComponent::SetRelativeScale(float x, float y, float z)
{
	SetRelativeScale(FVector3(x, y, z));
}

void CSceneComponent::SetRelativeScale(float x, float y)
{
	SetRelativeScale(FVector3(x, y, mRelativeScale.z));
}

void CSceneComponent::AddRelativeScale(const FVector3& Scale)
{
	SetRelativeScale(mRelativeScale + Scale);
}

void CSceneComponent::AddRelativeScale(const FVector2& Scale)
{
	AddRelativeScale(FVector3(Scale.x, Scale.y, 0.f));
}

void CSceneComponent::AddRelativeScale(float x, float y, float z)
{
	AddRelativeScale(FVector3(x, y, z));
}

void CSceneComponent::AddRelativeScale(float x, float y)
{
	AddRelativeScale(FVector3(x, y, 0.f));
}

void CSceneComponent::SetRelativeRotation(const FVector3& Rot)
{
	FVector3 _Rot = Rot;

	NormalizeRot(_Rot);

	mRelativeRot = _Rot;

	InheritRotation();
}

void CSceneComponent::SetRelativeRotation(const FVector2& Rot)
{
	SetRelativeRotation(FVector3(Rot.x, Rot.y, 0.f));
}

void CSceneComponent::SetRelativeRotation(float x, float y, float z)
{
	SetRelativeRotation(FVector3(x, y, z));
}

void CSceneComponent::SetRelativeRotation(float x, float y)
{
	SetRelativeRotation(FVector3(x, y, 0.f));
}

void CSceneComponent::SetRelativeRotationX(float x)
{
	SetRelativeRotation(FVector3(x, mRelativeRot.y, mRelativeRot.z));
}

void CSceneComponent::SetRelativeRotationY(float y)
{
	SetRelativeRotation(FVector3(mRelativeRot.x, y, mRelativeRot.z));
}

void CSceneComponent::SetRelativeRotationZ(float z)
{
	SetRelativeRotation(FVector3(mRelativeRot.x, mRelativeRot.y, z));
}

void CSceneComponent::AddRelativeRotation(const FVector3& Rot)
{
	SetRelativeRotation(mRelativeRot + Rot);
}

void CSceneComponent::AddRelativeRotation(const FVector2& Rot)
{
	AddRelativeRotation(FVector3(Rot.x, Rot.y, 0.f));
}

void CSceneComponent::AddRelativeRotation(float x, float y, float z)
{
	AddRelativeRotation(FVector3(x, y, z));
}

void CSceneComponent::AddRelativeRotation(float x, float y)
{
	AddRelativeRotation(FVector3(x, y, 0.f));
}

void CSceneComponent::AddRelativeRotationX(float x)
{
	AddRelativeRotation(FVector3(x, 0.f, 0.f));
}

void CSceneComponent::AddRelativeRotationY(float y)
{
	AddRelativeRotation(FVector3(0.f, y, 0.f));
}

void CSceneComponent::AddRelativeRotationZ(float z)
{
	AddRelativeRotation(FVector3(0.f, 0.f, z));
}

void CSceneComponent::SetRelativePos(const FVector3& Pos)
{
	mVelocity += (Pos - mRelativePos);

	mRelativePos = Pos;

	InheritPos();
}

void CSceneComponent::SetRelativePos(const FVector2& Pos)
{
	SetRelativePos(FVector3(Pos.x, Pos.y, 0.f));
}

void CSceneComponent::SetRelativePos(float x, float y, float z)
{
	SetRelativePos(FVector3(x, y, z));
}

void CSceneComponent::SetRelativePos(float x, float y)
{
	SetRelativePos(FVector3(x, y, 0.f));
}

void CSceneComponent::AddRelativePos(const FVector3& Pos)
{
	SetRelativePos(mRelativePos + Pos);
}

void CSceneComponent::AddRelativePos(const FVector2& Pos)
{
	AddRelativePos(FVector3(Pos.x, Pos.y, 0.f));
}

void CSceneComponent::AddRelativePos(float x, float y, float z)
{
	AddRelativePos(FVector3(x, y, z));
}

void CSceneComponent::AddRelativePos(float x, float y)
{
	AddRelativePos(FVector3(x, y, 0.f));
}

//상대 트랜스폼을 변경할경우
//부모가 존재하냐에 따라 월드 트랜스폼을 변경하는 방법이 달라진다.
void CSceneComponent::InheritScale()
{
	if (mInheritScale)
	{
		auto Parent = mParent.lock();

		if (Parent)
		{
			//월드 크기는 부모의 크기에 영향을 받는다.
			mWorldScale = mRelativeScale * Parent->mWorldScale;
		}
		else
		{
			mWorldScale = mRelativeScale;
		}

	}
	else
	{
		mWorldScale = mRelativeScale;
	}

	size_t Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Child = mChildList[i].lock();

		if (Child)
		{
			Child->InheritScale();
		}
	}
}

void CSceneComponent::InheritRotation()
{
	//부모컴포넌트가 회전한만큼 자식컴포넌트에서 더해준다.
	if (mInheritRot)
	{
		auto Parent = mParent.lock();

		if (Parent)
		{
			//월드 크기는 부모의 크기에 영향을 받는다.
			mWorldRot = mRelativeRot + Parent->mWorldRot;
		}
		else
		{
			mWorldRot = mRelativeRot;
		}

	}
	else
	{
		mWorldRot = mRelativeRot;
	}

	//회전했다면 월드의 축계산도 다시해준다.

	FMatrix RotMat;

	RotMat.Rotation(mWorldRot);

	for (int i = 0; i < EAxis::End; ++i)
	{
		/*
		1 0 0 0 * Xx Xy Xz 0 = Xx Xy Xz 0
				  Yx Yy Yz 0
				  Zx Zy Zz 0
				  0  0  0  0

		0 1 0 0 * Xx Xy Xz 0 = Yx Yy Yz 0
				  Yx Yy Yz 0
				  Zx Zy Zz 0
				  0  0  0  0

		0 0 1 0 * Xx Xy Xz 0 = Zx Zy Zz 0
				  Yx Yy Yz 0
				  Zx Zy Zz 0
				  0  0  0  0

		*/


		mWorldAxis[i] = FVector3::Axis[i].TransformNormal(RotMat);

		mWorldAxis[i].Normalize();
	}

	size_t Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Child = mChildList[i].lock();

		if (Child)
		{
			Child->InheritRotation();
		}
	}

	//부모가 회전했으므로 자식의 위치도 변경된다.
	for (size_t i = 0; i < Size; ++i)
	{
		auto Child = mChildList[i].lock();

		if (Child)
		{
			Child->InheritPos();
		}
	}
}

void CSceneComponent::InheritPos()
{
	auto Parent = mParent.lock();

	if (Parent)
	{
		//부모행렬로 먼저 회전한 방향의 위치값을 구해준다.
		FMatrix ParentMatrix;

		//만약 부모 회전에 영향을 받지 않으면 회전을 0으로 두고 계산한다.
		if (mInheritRot)
		{
			FVector3 ParentRot = Parent->GetWorldRot();
			ParentMatrix.Rotation(ParentRot);
		}

		//위치값을 부모행렬에 넣어준다.
		FVector3 ParentPos = Parent->GetWorldPos();
		memcpy(&ParentMatrix._41, &ParentPos, sizeof(FVector3));

		//월드 위치
		//부모의 위치 + 부모의 회전까지 포함해서 계산한다.
		mWorldPos = mRelativePos.TransformCoord(ParentMatrix);
	}
	else
	{
		mWorldPos = mRelativePos;
	}

	size_t Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Child = mChildList[i].lock();

		if (Child)
		{
			Child->InheritPos();
		}
	}
}

void CSceneComponent::SetWorldScale(const FVector3& Scale)
{
	mWorldScale = Scale;

	InheritWorldScale();
}

void CSceneComponent::SetWorldScale(const FVector2& Scale)
{
	SetWorldScale(FVector3(Scale.x, Scale.y, mWorldScale.z));
}

void CSceneComponent::SetWorldScale(float x, float y, float z)
{
	SetWorldScale(FVector3(x, y, z));
}

void CSceneComponent::SetWorldScale(float x, float y)
{
	SetWorldScale(FVector3(x, y, mWorldScale.z));
}

void CSceneComponent::AddWorldScale(const FVector3& Scale)
{
	SetWorldScale(mWorldScale + Scale);
}

void CSceneComponent::AddWorldScale(const FVector2& Scale)
{
	AddWorldScale(FVector3(Scale.x, Scale.y, 0.f));
}

void CSceneComponent::AddWorldScale(float x, float y, float z)
{
	AddWorldScale(FVector3(x, y, z));
}

void CSceneComponent::AddWorldScale(float x, float y)
{
	AddWorldScale(FVector3(x, y, 0.f));
}

void CSceneComponent::SetWorldRotation(const FVector3& Rot)
{
	FVector3 _Rot = Rot;

	NormalizeRot(_Rot);

	mWorldRot = _Rot;

	InheritWorldRotation();
}

void CSceneComponent::SetWorldRotation(const FVector2& Rot)
{
	SetWorldRotation(FVector3(Rot.x, Rot.y, 0.f));
}

void CSceneComponent::SetWorldRotation(float x, float y, float z)
{
	SetWorldRotation(FVector3(x, y, z));
}

void CSceneComponent::SetWorldRotation(float x, float y)
{
	SetWorldRotation(FVector3(x, y, 0.f));
}

void CSceneComponent::SetWorldRotationX(float x)
{
	SetWorldRotation(FVector3(x, mWorldRot.y, mWorldRot.z));
}

void CSceneComponent::SetWorldRotationY(float y)
{
	SetWorldRotation(FVector3(mWorldRot.x, y, mWorldRot.z));
}

void CSceneComponent::SetWorldRotationZ(float z)
{
	SetWorldRotation(FVector3(mWorldRot.x, mWorldRot.y, z));
}

void CSceneComponent::AddWorldRotation(const FVector3& Rot)
{
	SetWorldRotation(mWorldRot + Rot);
}

void CSceneComponent::AddWorldRotation(const FVector2& Rot)
{
	AddWorldRotation(FVector3(Rot.x, Rot.y, 0.f));
}

void CSceneComponent::AddWorldRotation(float x, float y, float z)
{
	AddWorldRotation(FVector3(x, y, z));
}

void CSceneComponent::AddWorldRotation(float x, float y)
{
	AddWorldRotation(FVector3(x, y, 0.f));
}

void CSceneComponent::AddWorldRotationX(float x)
{
	AddWorldRotation(FVector3(x, 0.f, 0.f));
}

void CSceneComponent::AddWorldRotationY(float y)
{
	AddWorldRotation(FVector3(0.f, y, 0.f));
}

void CSceneComponent::AddWorldRotationZ(float z)
{
	AddWorldRotation(FVector3(0.f, 0.f, z));
}

void CSceneComponent::SetWorldPos(const FVector3& Pos)
{
	mVelocity += (Pos - mWorldPos);

	mWorldPos = Pos;

	InheritWorldPos();
}

void CSceneComponent::SetWorldPos(const FVector2& Pos)
{
	SetWorldPos(FVector3(Pos.x, Pos.y, 0.f));
}

void CSceneComponent::SetWorldPos(float x, float y, float z)
{
	SetWorldPos(FVector3(x, y, z));
}

void CSceneComponent::SetWorldPos(float x, float y)
{
	SetWorldPos(FVector3(x, y, 0.f));
}

void CSceneComponent::AddWorldPos(const FVector3& Pos)
{
	SetWorldPos(mWorldPos + Pos);
}

void CSceneComponent::AddWorldPos(const FVector2& Pos)
{
	AddWorldPos(FVector3(Pos.x, Pos.y, 0.f));
}

void CSceneComponent::AddWorldPos(float x, float y, float z)
{
	AddWorldPos(FVector3(x, y, z));
}

void CSceneComponent::AddWorldPos(float x, float y)
{
	AddWorldPos(FVector3(x, y, 0.f));
}

void CSceneComponent::InheritWorldScale()
{
	if (mInheritScale)
	{
		auto Parent = mParent.lock();

		if (Parent)
		{
			//월드 크기는 부모의 크기에 영향을 받는다.
			mRelativeScale = mWorldScale / Parent->mWorldScale;
		}
		else
		{
			mRelativeScale = mWorldScale;
		}

	}
	else
	{
		mRelativeScale = mWorldScale;
	}

	size_t Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Child = mChildList[i].lock();

		if (Child)
		{
			Child->InheritScale();
		}
	}
}

void CSceneComponent::InheritWorldRotation()
{
	//부모컴포넌트가 회전한만큼 자식컴포넌트에서 더해준다.
	if (mInheritRot)
	{
		auto Parent = mParent.lock();

		if (Parent)
		{
			//월드 크기는 부모의 크기에 영향을 받는다.
			mRelativeRot = mWorldRot - Parent->mWorldRot;
		}
		else
		{
			mRelativeRot = mWorldRot;
		}

	}
	else
	{
		mRelativeRot = mWorldRot;
	}

	//회전했다면 월드의 축계산도 다시해준다.

	FMatrix RotMat;

	RotMat.Rotation(mWorldRot);

	for (int i = 0; i < EAxis::End; ++i)
	{
		/*
		1 0 0 0 * Xx Xy Xz 0 = Xx Xy Xz 0
				  Yx Yy Yz 0
				  Zx Zy Zz 0
				  0  0  0  0

		0 1 0 0 * Xx Xy Xz 0 = Yx Yy Yz 0
				  Yx Yy Yz 0
				  Zx Zy Zz 0
				  0  0  0  0

		0 0 1 0 * Xx Xy Xz 0 = Zx Zy Zz 0
				  Yx Yy Yz 0
				  Zx Zy Zz 0
				  0  0  0  0

		*/


		mWorldAxis[i] = FVector3::Axis[i].TransformNormal(RotMat);

		mWorldAxis[i].Normalize();
	}

	size_t Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Child = mChildList[i].lock();

		if (Child)
		{
			Child->InheritRotation();
		}
	}

	//부모가 회전했으므로 자식의 위치도 변경된다.
	for (size_t i = 0; i < Size; ++i)
	{
		auto Child = mChildList[i].lock();

		if (Child)
		{
			Child->InheritPos();
		}
	}
}

void CSceneComponent::InheritWorldPos()
{
	auto Parent = mParent.lock();

	if (Parent)
	{
		//부모행렬로 먼저 회전한 방향의 위치값을 구해준다.
		FMatrix ParentMatrix;

		//만약 부모 회전에 영향을 받지 않으면 회전을 0으로 두고 계산한다.
		if (mInheritRot)
		{
			FVector3 ParentRot = Parent->GetWorldRot();
			ParentMatrix.Rotation(ParentRot);
		}

		//위치값을 부모행렬에 넣어준다.
		FVector3 ParentPos = Parent->GetWorldPos();
		memcpy(&ParentMatrix._41, &ParentPos, sizeof(FVector3));

		//역행렬로 본인위치 - 부모의 월드위치를 해서 상대적인 위치를 구한다.

		ParentMatrix.Inverse();

		//월드 위치
		//부모의 위치 + 부모의 회전까지 포함해서 계산한다.
		mRelativePos = mWorldPos.TransformCoord(ParentMatrix);
	}
	else
	{
		mRelativePos = mWorldPos;
	}

	size_t Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Child = mChildList[i].lock();

		if (Child)
		{
			Child->InheritPos();
		}
	}
}

void CSceneComponent::UpdateChildWorldPos(const FVector3& Dist)
{
	size_t Size = mChildList.size();

	for (size_t i = 0; i < Size; ++i)
	{
		auto Child = mChildList[i].lock();

		if (Child)
		{
			Child->mWorldPos += Dist;
			Child->UpdateChildWorldPos(Dist);
		}
	}
}

void CSceneComponent::Save(std::ofstream& File) const
{
	CComponent::Save(File);
	auto Parent = mParent.lock();
	File << "Parent=" << (Parent ? Parent->GetName() : "") << "\n";
	File << "RelPos=" << mRelativePos.x << " " << mRelativePos.y << " " << mRelativePos.z << "\n";
	File << "RelScale=" << mRelativeScale.x << " " << mRelativeScale.y << " " << mRelativeScale.z << "\n";
	File << "RelRot=" << mRelativeRot.x << " " << mRelativeRot.y << " " << mRelativeRot.z << "\n";
	File << "Layer=" << mRenderLayerOrder << "\n";
}

void CSceneComponent::Load(const std::unordered_map<std::string, std::string>& Props)
{
	{
		auto it = Props.find("RelPos");
		if (it != Props.end())
		{
			float x = 0.f, y = 0.f, z = 0.f;
			sscanf_s(it->second.c_str(), "%f %f %f", &x, &y, &z);
			SetRelativePos(x, y, z);
		}
	}
	{
		auto it = Props.find("RelScale");
		if (it != Props.end())
		{
			float x = 1.f, y = 1.f, z = 1.f;
			sscanf_s(it->second.c_str(), "%f %f %f", &x, &y, &z);
			SetRelativeScale(x, y, z);
		}
	}
	{
		auto it = Props.find("RelRot");
		if (it != Props.end())
		{
			float x = 0.f, y = 0.f, z = 0.f;
			sscanf_s(it->second.c_str(), "%f %f %f", &x, &y, &z);
			SetRelativeRotation(x, y, z);
		}
	}
	{
		auto it = Props.find("Layer");
		if (it != Props.end())
			SetRenderLayer(std::stoi(it->second));
	}
}
