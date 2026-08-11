#include "StatusInterface.h"

void CStatusInterface::AddHP(float HP)
{
	mHP += HP;

	if (mHP <= 0.f)
	{
		if (mDeathFunc)
		{
			mDeathFunc();
		}
	}
	else if (mHP > mHPMax)
	{
		mHP = mHPMax;
	}
}
