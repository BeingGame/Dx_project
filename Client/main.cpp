
#include "Engine.h"
#include "resource.h"

#include "World/MainWorld.h"
#include "World/WorldManager.h"

#include "World/GlobalCollision.h"

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
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (!CEngine::GetInst()->Init(hInstance, TEXT("MyGame"), IDI_ICON1, IDI_ICON1, 1280, 720))
	{
		CEngine::GetInst()->DestroyInst();
		return 0;
	}

	CGlobalCollision::SetGlobalCollision();

	CWorldManager::GetInst()->CreateWorld<CMainWorld>(false);

	int Ret = CEngine::GetInst()->Run();

	CEngine::GetInst()->DestroyInst();

	return Ret;
}