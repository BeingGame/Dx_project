#pragma once


#include "ColliderBox2D.h"
#include "ColliderSphere2D.h"

//충돌 계산
//연산의 비용이 매우 크기 때문에 성능이 좋지 않다.
//충돌 연산의 경우엔 월드에 배치된 모든 액터(오브젝트)에대해서 연산을 시도한다.
//n^2만큼 연산을 시도하는데 비용도 크기 때문에 효율이 떨어진다.

//사각형, 원, 점

class CCollision
{
public:
	static bool CollisionBox2DToBox2D(FVector3& HitPoint, FVector3& Normal, float& Depth, class CColliderBox2D* Src, class CColliderBox2D* Dest);

	//사각형 충돌
	//AABB충돌
	static bool CollisionAABB2DToAABB2D(FVector3& HitPoint, FVector3& Normal, float& Depth, const FBox2DInfo& Src, const FBox2DInfo& Dest);
	//OBB충돌
	static bool CollisionOBB2DToOBB2D(FVector3& HitPoint, FVector3& Normal, float& Depth, const FBox2DInfo& Src, const FBox2DInfo& Dest);

	//원 충돌
	static bool CollisionSphere2DToSphere2D(FVector3& HitPoint, FVector3& Normal, float& Depth, class CColliderSphere2D* Src, class CColliderSphere2D* Dest);
	static bool CollisionSphere2DToSphere2D(FVector3& HitPoint, FVector3& Normal, float& Depth, const FSphere2DInfo& Src, const FSphere2DInfo& Dest);


	static bool CollisionBox2DToSphere2D(FVector3& HitPoint, FVector3& Normal, float& Depth, class CColliderBox2D* Box, class CColliderSphere2D* Sphere);
	static bool CollisionSphere2DToBox2D(FVector3& HitPoint, FVector3& Normal, float& Depth, class CColliderSphere2D* Sphere, class CColliderBox2D* Box);
	static bool CollisionSphere2DToBox2D(FVector3& HitPoint, FVector3& Normal, float& Depth, const FSphere2DInfo& Sphere, const FBox2DInfo& Box);

	//점 충돌
	static bool CollisionPointToSphere2D(FVector3& HitPoint, FVector3& Normal, float& Depth, const FSphere2DInfo& Sphere, const FVector3& Point);
	static bool CollisionPointToAABB2D(FVector3& HitPoint, FVector3& Normal, float& Depth, const FBox2DInfo& Box, const FVector3& Point);
	static bool CollisionPointToOBB2D(FVector3& HitPoint, FVector3& Normal, float& Depth, const FBox2DInfo& Box, const FVector3& Point);

private:
	//분리축 계산을 위한 축 투영 계산
	//사각형의 반지름을 구해준다.
	static bool AxisProjection(const FVector3& CenterLine, const FVector3& Axis, float SrcHalfSize, const FVector3* DestAxis, const FVector2& DestHalfSize, float& OverlapDepth);


};

