

#include "Engine.h"
#include "resource.h"

#include "World/MainWorld.h"
#include "World/StartWorld.h"
#include "World/WorldManager.h"

#include "World/GlobalCollision.h"
#include "World/PrefabManager.h"

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

	CWorldManager::GetInst()->CreateWorld<CMainWorld>(false);

	int Ret = CEngine::GetInst()->Run();

	CPrefabManager::GetInst()->DestroyInst();
	CEngine::GetInst()->DestroyInst();

	return Ret;
}