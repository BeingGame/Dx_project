

#include "Engine.h"
#include "resource.h"

#include "World/MainWorld.h"
#include "World/StartWorld.h"
#include "World/WorldManager.h"

#include "World/GlobalCollision.h"
#include "World/PrefabManager.h"

// Actor types for scene save/load
#include "World/Player.h"
#include "World/Monster.h"
#include "World/Bullet.h"
#include "World/Wall.h"
#include "World/Chaser.h"
#include "World/Turret.h"
#include "World/BlackHole.h"
#include "World/OneWayWall.h"
#include "World/ReflectBullet.h"
#include "World/SlowArea.h"
#include "World/Shield.h"

// Component types for scene save/load
#include "World/Component/StatusComponent.h"
#include "World/Component/TurretSkillComp.h"
#include "World/Component/DirectionInputComponent.h"
#include "World/Component/ActionStateComponent.h"
#include "World/Component/HeightComponent.h"

#include "World/World.h"

#ifdef _DEBUG

#pragma comment(lib,"GameEngine_Debug.lib")

#else

#pragma comment(lib,"GameEngine.lib")

#endif


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	//UNREFERENCED_PARAMETER : 인자 미사용 경고를 제거하는데 사용되는 매크로
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (!CEngine::GetInst()->Init(hInstance, TEXT("MyGame"), IDI_ICON1, IDI_ICON1, 1280, 720))
	{
		CEngine::GetInst()->DestroyInst();
		return 0;
	}

	CGlobalCollision::SetGlobalCollision();

	CPrefabManager::GetInst()->Init();

	// Register actor types
	CWorld::RegisterActorType<CPlayer>("CPlayer");
	CWorld::RegisterActorType<CMonster>("CMonster");
	CWorld::RegisterActorType<CBullet>("CBullet");
	CWorld::RegisterActorType<CWall>("CWall");
	CWorld::RegisterActorType<CChaser>("CChaser");
	CWorld::RegisterActorType<CTurret>("CTurret");
	CWorld::RegisterActorType<CBlackHole>("CBlackHole");
	CWorld::RegisterActorType<COneWayWall>("COneWayWall");
	CWorld::RegisterActorType<CReflectBullet>("CReflectBullet");
	CWorld::RegisterActorType<CSlowArea>("CSlowArea");
	CWorld::RegisterActorType<CShield>("CShield");

	// Register client component types
	CWorld::RegisterComponentType<CStatusComponent>("CStatusComponent");
	CWorld::RegisterComponentType<CTurretSkillComp>("CTurretSkillComp");
	CWorld::RegisterComponentType<CDirectionInputComponent>("CDirectionInputComponent");
	CWorld::RegisterComponentType<CActionStateComponent>("CActionStateComponent");
	CWorld::RegisterComponentType<CHeightComponent>("CHeightComponent");

	CWorldManager::GetInst()->CreateWorld<CMainWorld>(false);

	int ExitCode = CEngine::GetInst()->Run();

	CPrefabManager::GetInst()->DestroyInst();
	CEngine::GetInst()->DestroyInst();

	return ExitCode;
}