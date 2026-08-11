#pragma once

#include "CBufferData.h"
#include "CBufferContainer.h"

class CCBufferMaterial :
	public CCBufferData
{
public:
	CCBufferMaterial();
	CCBufferMaterial(const CCBufferMaterial& src);
	CCBufferMaterial(CCBufferMaterial&& src) noexcept;
	virtual ~CCBufferMaterial();

protected:
	FCBufferMaterial mData;

public:
	void SetColor(const FVector4& Color)
	{
		mData.BaseColor = Color;
	}

	void SetOpacity(float Opacity)
	{
		mData.Opacity = Opacity;
	}

	void SetChromaKeyEnable(bool Enable)
	{
		mData.ChromakeyEnable = Enable ? 1 : 0;
	}

	void SetChromaKeyColor(const FVector3& Color)
	{
		mData.ChromaKeyColor = Color;
	}

	void SetChromaKeyThreshold(float Threshold)
	{
		mData.ChromaKeyThreshold = Threshold;
	}

	void SetChromaKey(bool Enable, const FVector3& Color = FVector3(0.f, 1.f, 0.f), float Threshold = 0.1f)
	{
		SetChromaKeyEnable(Enable);
		SetChromaKeyColor(Color);
		SetChromaKeyThreshold(Threshold);
	}

	void SetUVScrollEnable(bool Enable)
	{
		mData.UVScrollEnable = Enable ? 1 : 0;
	}

	void SetUVScrollSpeed(const FVector2& Speed)
	{
		mData.UVScrollSpeed = Speed;
	}

	void SetUVScrollTime(float Time)
	{
		mData.UVScrollTime = Time;
	}

	void SetUVScroll(bool Enable, const FVector2& Speed = FVector2(0.f, 0.f), float Time = 0.1f)
	{
		SetUVScrollEnable(Enable);
		SetUVScrollSpeed(Speed);
		SetUVScrollTime(Time);
	}

	void SetDissolveEnable(bool Enable)
	{
		mData.DissolveEnable = Enable ? 1 : 0;
	}

	void SetDissolveEdgeColor(const FVector4& Color)
	{
		mData.DissolveEdgeColor = Color;
	}

	void SetDissolveProgress(float Progress)
	{
		mData.DissolveProgress = Progress;
	}

	void SetDissolveEdgeWidth(float Width)
	{
		mData.DissolveEdgeWidth = Width;
	}

	void SetDissolve(bool Enable, const FVector4& Color = FVector4::Red, float Progress = 0.f, float Width = 0.04f)
	{
		SetDissolveEnable(Enable);
		SetDissolveEdgeColor(Color);
		SetDissolveProgress(Progress);
		SetDissolveEdgeWidth(Width);
	}

	void SetHitEffectEnable(bool Enable)
	{
		mData.HitEffectEnable = Enable;
	}

	void SetHitIntensity(float Intensity)
	{
		mData.HitIntensity = Intensity;
	}

	void SetHitEffectColor(const FVector4& Color)
	{
		mData.HitColor = Color;
	}

	void SetHitEffect(bool Enable, float Intensity = 0.f, const FVector4& Color = FVector4::Red)
	{
		SetHitEffectEnable(Enable);
		SetHitIntensity(Intensity);
		SetHitEffectColor(Color);
	}


	//실습
	//Hit이펙트 처리하기
	//깜빡거리게 설정하는데, 색상은 자유
	//투명했다가 다시 그려졌다가 OK
	//빨간색으로 변했다가 원본으로 변했다가 OK

public:
	virtual bool Init();
	virtual void UpdateBuffer();
	virtual CCBufferMaterial* Clone();
};

