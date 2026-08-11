#include "Collision.h"

bool CCollision::CollisionBox2DToBox2D(FVector3& HitPoint, FVector3& Normal, float& Depth, CColliderBox2D* Src, CColliderBox2D* Dest)
{
	if (!Src || !Dest)
	{
		return false;
	}

	//현재 사각형이 회전되있는가에 따라 AABB충돌을 사용할지 말지를 결정짓는다.
	if (Src->GetWorldRot().IsZero() && Dest->GetWorldRot().IsZero())
	{
		return CollisionAABB2DToAABB2D(HitPoint, Normal, Depth, Src->GetInfo(), Dest->GetInfo());
	}

	return CollisionOBB2DToOBB2D(HitPoint, Normal, Depth, Src->GetInfo(), Dest->GetInfo());
}

bool CCollision::CollisionAABB2DToAABB2D(FVector3& HitPoint, FVector3& Normal, float& Depth, const FBox2DInfo& Src, const FBox2DInfo& Dest)
{
	//AABB(Axis Aligned Bounding Box)
	//축이 정렬된 평행한 박스 
	//두개의 사각형이 동일한 축 방향을 가르킨다.
	//두 사각형의 min, max값을 비교해서 충돌했는지 안했는지 확인한다.
	//x좌표를 기준으로 src의 maxX 가 dest의 minX보다 크고,
	//src의 minX가 dest의 maxX보다 작다면 충돌이다.
	//이 두가지 조건이 맞아야 충돌, 아니라면 충돌하지 않은것으로 판단한다.

	//먼저 사각형의 크기부터 지정해준다.
	//중심점을 기준으로 max의 경우엔 각 축에 Halfsize를 곱한값으로 간단하게 min max를 계산할수 있다.
	FVector3 SrcMin = Src.Center - Src.Axis[EAxis::X] * Src.Halfsize.x - Src.Axis[EAxis::Y] * Src.Halfsize.y;
	FVector3 SrcMax = Src.Center + Src.Axis[EAxis::X] * Src.Halfsize.x + Src.Axis[EAxis::Y] * Src.Halfsize.y;

	FVector3 DestMin = Dest.Center - Dest.Axis[EAxis::X] * Dest.Halfsize.x - Dest.Axis[EAxis::Y] * Dest.Halfsize.y;
	FVector3 DestMax = Dest.Center + Dest.Axis[EAxis::X] * Dest.Halfsize.x + Dest.Axis[EAxis::Y] * Dest.Halfsize.y;

	//아래 4가지 조건이 전부다 거짓이면 충돌
	if (SrcMin.x > DestMax.x)
	{
		return false;
	}

	if (SrcMax.x < DestMin.x)
	{
		return false;
	}

	if (SrcMin.y > DestMax.y)
	{
		return false;
	}

	if (SrcMax.y < DestMin.y)
	{
		return false;
	}


	float OverlapX = (std::min)(SrcMax.x, DestMax.x) - (std::max)(SrcMin.x, DestMin.x);
	float OverlapY = (std::min)(SrcMax.y, DestMax.y) - (std::max)(SrcMin.y, DestMin.y);

	if (OverlapX < OverlapY)
	{
		Depth = OverlapX;
		Normal = FVector3(Src.Center.x < Dest.Center.x ? 1.f : -1.f, 0.f, 0.f);
	}
	else
	{
		Depth = OverlapY;
		Normal = FVector3(0.f, Src.Center.y < Dest.Center.y ? 1.f : -1.f, 0.f);
	}

	//충돌 포인트를 구할땐 사각형을 하나 만들어서
	//그 사각형의 중심점을 넣어준다.

	FVector3 IntersectMin, IntersectMax;

	IntersectMin.x = SrcMin.x > DestMin.x ? SrcMin.x : DestMin.x;
	IntersectMax.x = SrcMax.x < DestMax.x ? SrcMax.x : DestMax.x;

	IntersectMin.y = SrcMin.y > DestMin.y ? SrcMin.y : DestMin.y;
	IntersectMax.y = SrcMax.y < DestMax.y ? SrcMax.y : DestMax.y;

	HitPoint = (IntersectMin + IntersectMax) * 0.5f;

	return true;
}

bool CCollision::CollisionOBB2DToOBB2D(FVector3& HitPoint, FVector3& Normal, float& Depth, const FBox2DInfo& Src, const FBox2DInfo& Dest)
{
	//OBB(Oriented Bounding Box)
	//회전된 박스(사각형)
	//사각형끼리의 충돌이 있을때 회전이 있다면 이 충돌 연산으로 충돌 체크를 시도한다.
	//분리축 이론
	//물체를 투영(그림자를 그렸을때) 그림자가 겹치지 않는 축이 단 하나라도 존재하면
	//충돌하지 않은것
	//투영했을때 사각형의 반지름을 이용해서 두 사각형의 원점의 거리가 두사각형의 반지름 합보다 작다면 해당 축에선
	//충돌이다.

	//먼저 중심점의 거리를 구하기 위한 센터라인을 구해준다.
	FVector3 CenterLine = Src.Center - Dest.Center;

	float Overlap[4] = {};
	FVector3 Axes[4] =
	{
		Src.Axis[EAxis::X],
		Src.Axis[EAxis::Y],
		Dest.Axis[EAxis::X],
		Dest.Axis[EAxis::Y]
	};

	//분리축 계산을 통해 분리된 축이 있는지 확인한다.

	if (!AxisProjection(CenterLine, Src.Axis[EAxis::X], Src.Halfsize.x, Dest.Axis, Dest.Halfsize, Overlap[0]))
	{
		return false;
	}

	if (!AxisProjection(CenterLine, Src.Axis[EAxis::Y], Src.Halfsize.y, Dest.Axis, Dest.Halfsize, Overlap[1]))
	{
		return false;
	}

	if (!AxisProjection(CenterLine, Dest.Axis[EAxis::X], Dest.Halfsize.x, Src.Axis, Src.Halfsize, Overlap[2]))
	{
		return false;
	}

	if (!AxisProjection(CenterLine, Dest.Axis[EAxis::Y], Dest.Halfsize.y, Src.Axis, Src.Halfsize, Overlap[3]))
	{
		return false;
	}

	float MinOverlap = FLT_MAX;
	int MinAxisIndex = -1;

	for (int i = 0; i < 4; ++i)
	{
		if (Overlap[i] < MinOverlap)
		{
			MinOverlap = Overlap[i];
			MinAxisIndex = i;
		}
	}

	Depth = MinOverlap;
	Normal = Axes[MinAxisIndex];
	Normal.Normalize();

	FVector3 ToDest = Dest.Center - Src.Center;
	if (Normal.DotProduct(ToDest) < 0.f)
	{
		Normal = -Normal;
	}

	FVector3 SrcMin = Src.Center - Src.Axis[EAxis::X] * Src.Halfsize.x - Src.Axis[EAxis::Y] * Src.Halfsize.y;
	FVector3 SrcMax = Src.Center + Src.Axis[EAxis::X] * Src.Halfsize.x + Src.Axis[EAxis::Y] * Src.Halfsize.y;

	if (SrcMin.x > SrcMax.x)
	{
		std::swap(SrcMin.x, SrcMax.x);
	}
	if (SrcMin.y > SrcMax.y)
	{
		std::swap(SrcMin.y, SrcMax.y);
	}
	FVector3 DestMin = Dest.Center - Dest.Axis[EAxis::X] * Dest.Halfsize.x - Dest.Axis[EAxis::Y] * Dest.Halfsize.y;
	FVector3 DestMax = Dest.Center + Dest.Axis[EAxis::X] * Dest.Halfsize.x + Dest.Axis[EAxis::Y] * Dest.Halfsize.y;

	if (DestMin.x > DestMax.x)
	{
		std::swap(DestMin.x, DestMax.x);
	}
	if (DestMin.y > DestMax.y)
	{
		std::swap(DestMin.y, DestMax.y);
	}

	FVector3 IntersectMin, IntersectMax;

	IntersectMin.x = SrcMin.x > DestMin.x ? SrcMin.x : DestMin.x;
	IntersectMax.x = SrcMax.x < DestMax.x ? SrcMax.x : DestMax.x;

	IntersectMin.y = SrcMin.y > DestMin.y ? SrcMin.y : DestMin.y;
	IntersectMax.y = SrcMax.y < DestMax.y ? SrcMax.y : DestMax.y;

	HitPoint = (IntersectMin + IntersectMax) * 0.5f;

	return true;
}

bool CCollision::CollisionSphere2DToSphere2D(FVector3& HitPoint, FVector3& Normal, float& Depth, CColliderSphere2D* Src, CColliderSphere2D* Dest)
{
	if (!Src || !Dest)
	{
		return false;
	}

	return CollisionSphere2DToSphere2D(HitPoint, Normal, Depth, Src->GetInfo(), Dest->GetInfo());
}

bool CCollision::CollisionSphere2DToSphere2D(FVector3& HitPoint, FVector3& Normal, float& Depth, const FSphere2DInfo& Src, const FSphere2DInfo& Dest)
{
	float Distance = Src.Center.Distance(Dest.Center);
	float TotalRadius = Src.Radius + Dest.Radius;

	//반지름의 합보다 거리가 멀면 충돌 실패
	if (Distance > TotalRadius)
	{
		return false;
	}

	Depth = TotalRadius - Distance;

	if (Distance > 0.f)
	{
		Normal = Dest.Center - Src.Center;
		Normal.Normalize();
	}
	else
	{
		//Distance가 0이면 Normal x축의 +로 정해준다.
		Normal = FVector3(1.f, 0.f, 0.f);
	}

	float Gap = Depth * 0.5f;
	HitPoint = Src.Center + Normal * (Src.Radius - Gap);

	return true;
}

bool CCollision::CollisionBox2DToSphere2D(FVector3& HitPoint, FVector3& Normal, float& Depth, CColliderBox2D* Box, CColliderSphere2D* Sphere)
{
	if (!Box || !Sphere)
	{
		return false;
	}

	return CollisionSphere2DToBox2D(HitPoint, Normal, Depth, Sphere->GetInfo(), Box->GetInfo());
}

bool CCollision::CollisionSphere2DToBox2D(FVector3& HitPoint, FVector3& Normal, float& Depth, CColliderSphere2D* Sphere, CColliderBox2D* Box)
{
	bool Check = CollisionBox2DToSphere2D(HitPoint, Normal, Depth, Box, Sphere);

	if (Check)
	{
		Normal = -Normal;
	}

	return Check;
}

bool CCollision::CollisionSphere2DToBox2D(FVector3& HitPoint, FVector3& Normal, float& Depth, const FSphere2DInfo& Sphere, const FBox2DInfo& Box)
{
	//사각형과 원충돌
	//사각형을 중심점 거리를 이용해 분리축 계산을 시도한다.
	FVector3 CenterLine = Box.Center - Sphere.Center;

	FVector3 Axis = CenterLine;
	Axis.Normalize();

	float Overlap[3] = {};
	FVector3 Axes[3] =
	{
		Axis,
		Box.Axis[EAxis::X],
		Box.Axis[EAxis::Y]
	};

	//분리축 계산은 중심점 거리의 법선벡터를 이용한다.
	if (!AxisProjection(CenterLine, Axis, Sphere.Radius, Box.Axis, Box.Halfsize, Overlap[0]))
	{
		return false;
	}

	float CenterProjX = abs(CenterLine.DotProduct(Box.Axis[EAxis::X]));
	float TotalRadiusX = Sphere.Radius + Box.Halfsize.x;

	if (CenterProjX > TotalRadiusX)
	{
		return false;
	}

	Overlap[1] = TotalRadiusX - CenterProjX;

	float CenterProjY = abs(CenterLine.DotProduct(Box.Axis[EAxis::Y]));
	float TotalRadiusY = Sphere.Radius + Box.Halfsize.y;

	if (CenterProjY > TotalRadiusY)
	{
		return false;
	}

	Overlap[2] = TotalRadiusY - CenterProjY;

	float MinOverlap = FLT_MAX;
	int MinAxisIndex = -1;

	for (int i = 0; i < 3; ++i)
	{
		if (Overlap[i] < MinOverlap)
		{
			MinOverlap = Overlap[i];
			MinAxisIndex = i;
		}
	}

	Depth = MinOverlap;
	Normal = Axes[MinAxisIndex];
	Normal.Normalize();

	FVector3 ToSphere = Sphere.Center - Box.Center;
	if (Normal.DotProduct(ToSphere) < 0.f)
	{
		Normal = -Normal;
	}

	FVector3 BoxMin = Box.Center - Box.Axis[EAxis::X] * Box.Halfsize.x - Box.Axis[EAxis::Y] * Box.Halfsize.y;
	FVector3 BoxMax = Box.Center + Box.Axis[EAxis::X] * Box.Halfsize.x + Box.Axis[EAxis::Y] * Box.Halfsize.y;

	if (BoxMin.x > BoxMax.x)
	{
		std::swap(BoxMin.x, BoxMax.x);
	}
	if (BoxMin.y > BoxMax.y)
	{
		std::swap(BoxMin.y, BoxMax.y);
	}

	FVector3 SphereMin = Sphere.Center - FVector3(Sphere.Radius, Sphere.Radius, 0.f);
	FVector3 SphereMax = Sphere.Center + FVector3(Sphere.Radius, Sphere.Radius, 0.f);

	if (SphereMin.x > SphereMax.x)
	{
		std::swap(SphereMin.x, SphereMax.x);
	}
	if (SphereMin.y > SphereMax.y)
	{
		std::swap(SphereMin.y, SphereMax.y);
	}

	FVector3 IntersectMin, IntersectMax;

	IntersectMin.x = BoxMin.x > SphereMin.x ? BoxMin.x : SphereMin.x;
	IntersectMax.x = BoxMax.x < SphereMax.x ? BoxMax.x : SphereMax.x;

	IntersectMin.y = BoxMin.y > SphereMin.y ? BoxMin.y : SphereMin.y;
	IntersectMax.y = BoxMax.y < SphereMax.y ? BoxMax.y : SphereMax.y;

	HitPoint = (IntersectMin + IntersectMax) * 0.5f;

	return true;
}

bool CCollision::CollisionPointToSphere2D(FVector3& HitPoint, FVector3& Normal, float& Depth, const FSphere2DInfo& Sphere, const FVector3& Point)
{
	//점과 원충돌
	//점과 원의 중심점 거리를 계산한뒤 원의 반지름과 비교한다.
	float Distance = Point.Distance(Sphere.Center);

	if (Distance > Sphere.Radius)
	{
		return false;
	}

	Depth = Sphere.Radius - Distance;

	Normal = Point - Sphere.Center;
	Normal.Normalize();

	HitPoint = Point;

	return true;
}

bool CCollision::CollisionPointToAABB2D(FVector3& HitPoint, FVector3& Normal, float& Depth, const FBox2DInfo& Box, const FVector3& Point)
{
	//점과 AABB 사각형 충돌
	FVector3 Min = Box.Center - Box.Axis[EAxis::X] * Box.Halfsize.x - Box.Axis[EAxis::Y] * Box.Halfsize.y;
	FVector3 Max = Box.Center + Box.Axis[EAxis::X] * Box.Halfsize.x + Box.Axis[EAxis::Y] * Box.Halfsize.y;

	if (Point.x < Min.x || Point.x > Max.x || Point.y < Min.y || Point.y > Max.y)
	{
		return false;
	}

	float DistL = Point.x - Min.x;
	float DistR = Max.x - Point.x;
	float DistB = Point.y - Min.y;
	float DistT = Max.y - Point.y;

	float MinDist = DistL;
	Normal = FVector3(-1.f, 0.f, 0.f);

	if (DistR < MinDist)
	{
		MinDist = DistR;
		Normal = FVector3(1.f, 0.f, 0.f);
	}
	if (DistB < MinDist)
	{
		MinDist = DistB;
		Normal = FVector3(0.f, -1.f, 0.f);
	}
	if (DistT < MinDist)
	{
		MinDist = DistT;
		Normal = FVector3(0.f, 1.f, 0.f);
	}

	Depth = MinDist;
	HitPoint = Point;

	return true;
}

bool CCollision::CollisionPointToOBB2D(FVector3& HitPoint, FVector3& Normal, float& Depth, const FBox2DInfo& Box, const FVector3& Point)
{
	//점과 OBB 충돌
	//점을 OBB의 로컬 좌표계로 변환하여 처리한다.
	//OBB의 중심을 기준으로 점의 상대 위치를 구해준다.
	FVector3 LocalPoint = Point - Box.Center;

	//상대 위치를 이용해서 로컬 x랑 y축에 각각 투영해서 로컬좌표를 얻는다.
	float LocalX = LocalPoint.DotProduct(Box.Axis[EAxis::X]);
	float LocalY = LocalPoint.DotProduct(Box.Axis[EAxis::Y]);

	//로컬 좌표계에서 Halfsize를 기준으로 충돌을 확인한다.
	if (abs(LocalX) > Box.Halfsize.x || abs(LocalY) > Box.Halfsize.y)
	{
		return false;
	}

	float DistL = LocalX + Box.Halfsize.x;
	float DistR = Box.Halfsize.x - LocalX;
	float DistB = LocalY + Box.Halfsize.y;
	float DistT = Box.Halfsize.y - LocalY;

	//로컬 기준의 노말벡터를 구해준다.
	float MinDist = DistL;
	FVector3 LocalNormal = FVector3(-1.f, 0.f, 0.f);

	if (DistR < MinDist)
	{
		MinDist = DistR;
		LocalNormal = FVector3(1.f, 0.f, 0.f);
	}
	if (DistB < MinDist)
	{
		MinDist = DistB;
		LocalNormal = FVector3(0.f, -1.f, 0.f);
	}
	if (DistT < MinDist)
	{
		MinDist = DistT;
		LocalNormal = FVector3(0.f, 1.f, 0.f);
	}

	//로컬 기준이였던 normal벡터를 월드의 normal벡터로 변환해준다.
	Normal = Box.Axis[EAxis::X] * LocalNormal.x + Box.Axis[EAxis::Y] * LocalNormal.y;
	Normal.Normalize();

	Depth = MinDist;
	HitPoint = Point;

	return true;
}

bool CCollision::AxisProjection(const FVector3& CenterLine, const FVector3& Axis, float SrcHalfSize, const FVector3* DestAxis, const FVector2& DestHalfSize, float& OverlapDepth)
{
	//분리축 계산
	//중심점의 거리를 구해준다.
	float CenterProjectionDist = abs(CenterLine.DotProduct(Axis));
	//상대의 반지름을 구해준다.
	float DestProjectionDist = abs(Axis.DotProduct(DestAxis[EAxis::X])) * DestHalfSize.x + abs(Axis.DotProduct(DestAxis[EAxis::Y])) * DestHalfSize.y;

	//두 사각형의 반지름의 합을 구해준다.
	float TotalRadius = SrcHalfSize + DestProjectionDist;

	//반지름이 중심점 끼리의 거리보다 길다면
	//충돌이 일어난것
	if (TotalRadius > CenterProjectionDist)
	{
		OverlapDepth = TotalRadius - CenterProjectionDist;
		return true;
	}

	return false;
}
