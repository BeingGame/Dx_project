#include "GlobalCollision.h"

#include "World/CollisionInfoManager.h"

void CGlobalCollision::SetGlobalCollision()
{
	CCollisionInfoManager* InfoManager = CCollisionInfoManager::GetInst();

	if (InfoManager)
	{
		InfoManager->CreateChannel("Skill");

		InfoManager->CreateProfile("Skill", "Skill", true, ECollisionInteraction::Ignore);

		InfoManager->SetProfileInteraction("Static", "Skill", ECollisionInteraction::Overlap);
		InfoManager->SetProfileInteraction("Player", "Skill", ECollisionInteraction::Overlap);
		InfoManager->SetProfileInteraction("Monster", "Skill", ECollisionInteraction::Overlap);

		InfoManager->SetProfileInteraction("Skill", "Static", ECollisionInteraction::Overlap);
		InfoManager->SetProfileInteraction("Skill", "Player", ECollisionInteraction::Overlap);
		InfoManager->SetProfileInteraction("Skill", "Monster", ECollisionInteraction::Overlap);

		InfoManager->CreateChannel("BlockAll");

		InfoManager->CreateProfile("BlockAll", "BlockAll", true, ECollisionInteraction::Block);

		InfoManager->SetProfileInteraction("Static", "BlockAll", ECollisionInteraction::Block);
		InfoManager->SetProfileInteraction("Player", "BlockAll", ECollisionInteraction::Block);
		InfoManager->SetProfileInteraction("Monster", "BlockAll", ECollisionInteraction::Block);

		InfoManager->CreateChannel("OverlapAll");

		InfoManager->CreateProfile("OverlapAll", "OverlapAll", true);
	}
}
