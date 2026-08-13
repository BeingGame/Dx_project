#include "Engine.h"

#include "Device.h"
#include "TimeManager.h"

#include "Asset/AssetManager.h"
#include "Asset/PathManager.h"

#include "Asset/MeshManager.h"
#include "Asset/ShaderManager.h"
#include "Asset/Mesh.h"
#include "Asset/Shader.h"

#include "Asset/CBufferTransform.h"

#include "RenderManager.h"

#include "World/WorldManager.h"
#include "World/CollisionInfoManager.h"
#include "World/Input.h"

#include "LogManager.h"

CEngine* CEngine::mInstance = nullptr;

bool CEngine::mLoop = true;

CEngine::CEngine()
{

}

CEngine::~CEngine()
{
	CWorldManager::DestroyInst();

	CRenderManager::DestroyInst();

	CCollisionInfoManager::DestroyInst();

	CAssetManager::DestroyInst();

	CPathManager::Clear();

	CLogManager::DestroyInst();

	CDevice::DestroyInst();
}

bool CEngine::Init(HINSTANCE Inst, const TCHAR* WindowName, int IconID, int SmallIconID, int Width, int Height, bool WindowMode)
{
	mhInst = Inst;

	InitRegisterClass(WindowName, IconID, SmallIconID);

	//창 생성
	if (!Create(WindowName, Width, Height))
	{
		//창 실패하면 종료
		return false;
	}

	if (!CDevice::GetInst()->Init(mhWnd, Width, Height, WindowMode))
	{
		return false;
	}

	CTimeManager::Init();

	if (!CPathManager::Init())
	{
		return false;
	}

	if (!CLogManager::GetInst()->Init())
	{
		return false;
	}

	if (!CAssetManager::GetInst()->Init())
	{
		return false;
	}

	if (!CCollisionInfoManager::GetInst()->Init())
	{
		return false;
	}

	if (!CRenderManager::GetInst()->Init())
	{
		return false;
	}

	if (!CWorldManager::GetInst()->Init())
	{
		return false;
	}

	return true;
}

int CEngine::Run()
{
	//MyRegisterClass(hInstance);

	//// 애플리케이션 초기화를 수행합니다:
	//if (!InitInstance(hInstance, nCmdShow))
	//{
	//	return FALSE;
	//}

	//메뉴 리소스에서 윈도우 키입력에 대한 단축키를 설정
	//입력이 발생했을때 해당 단축키에 대한 메세지를 가진 테이블
	//게임 만들떈 독자적으로 키입력을 처리하기때문에 사용하지 않는다.
   //HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINAPI));


	//메세지를 받을 변수
	MSG msg = {};

	// 기본 메시지 루프입니다:
	// 메시지 큐 기반 처리 시스템
	// OS가 이벤트를 감지하면, 해당 이벤트에 대한 메시지를 메시지 큐에서 저장
	// 메시지는 클릭, 키입력, 창 이동들 여러가지 이벤트들을 말한다.
	// 메시지 큐에서 메시지를 받아와 순차적으로 처리한다.
	// 
	// GetMessage 메시지 큐에서 메시지를 가져오는 함수
	// 동기방식으로 이루어져 있다.
	// 함수가 호출되면 함수 내부에서 블락을 건다.
	// 메시지큐에 메시지가 들어올때 까지 getMessage함수는 프로그램을 대기시킨다.
	// 게임을 제작할때 GetMessage는 사용하지 않는다.

	//PeekMessage함수 비동기방식으로 이루어져있다.
	//메시지 큐에 메시지가 없을떄는 넘어가고
	//메시지가 있으면 메시지에 대한 처리를 진행한다.

	//PM_REMOVE 메시지를 가져온후 메시지 큐에서 메시지를 제거한다.
	while (mLoop)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			//키보드 입력이 발생했을때 WM_KEYDOWN이 발생된다.
			//이 발생한 이벤트를 WM_CHAR로 변환해주는 함수
			//이벤트가 발생하지않으면 넘어간다.
			TranslateMessage(&msg);

			//메시지를 윈도우 프로시저 함수로 넘겨주는 매크로 함수
			//WndProc 함수를 호출한다.
			DispatchMessage(&msg);

		}
		else
		{
			Logic();
		}
	}


	return (int)msg.wParam;
}

void CEngine::Destroy()
{
	SAFE_DELETE(mInstance);
}

//게임 로직이 실행될 공간
void CEngine::Logic()
{
	CTimeManager::Update();

	Update(CTimeManager::GetDeltaTime());
	Render();
}

void CEngine::Update(float DeltaTime)
{
	CAssetManager::GetInst()->Update();

	CWorldManager::GetInst()->Update(DeltaTime);

	CWorldManager::GetInst()->PostUpdate(DeltaTime);

	CRenderManager::GetInst()->Update(DeltaTime);
}

void CEngine::Render()
{
	//beginRender와 endRender사이에 그림을 그려준다.
	CDevice::GetInst()->BeginRender();

	CRenderManager::GetInst()->Render();

	CWorldManager::GetInst()->Render();

	CRenderManager::GetInst()->PostRender();

	CWorldManager::GetInst()->PostRender();

	CDevice::GetInst()->EndRender();
}

void CEngine::InitRegisterClass(const TCHAR* WindowName, int IConID, int SmallIconID)
{
	WNDCLASSEXW wcex;

	//창이 정상적으로 생성되도록 WNDCLASSEX 구조체의 크기를 넣어준다.
	wcex.cbSize = sizeof(WNDCLASSEX);

	//윈도우에서 그려지는 영역,클라이언트 영역 이라고 한다.
	//원도우를 다시 그린다. Horizontal, Vertical-> 전체를 다시 그린다.
	//윈도우의 그려지는 영역이 변경될때 다시 그린다.
	wcex.style = CS_HREDRAW | CS_VREDRAW;

	//메세지 발생시 호출할 프로시저 함수를 등록한다.
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;

	//레지스터에 등록할 프로그램의 ID(식별번호)를 넣는다.
	wcex.hInstance = mhInst;

	//실행 파일의 아이콘 설정
	wcex.hIcon = LoadIcon(mhInst, MAKEINTRESOURCE(IConID));
	//커서 이미지 설정
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);

	//창의 벼경색상을 설정한다.
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	//윈도우의 메뉴, 드롭다운 메뉴를 말하는데
	//필요하다면 메뉴를 만들어서 추가해줄수 있다.
	wcex.lpszMenuName = 0;

	//레지스터에 등록할 윈도우 클래스의 이름을 지정
	//TEXT("") 매크로를 이용해서 유니코드로 변환하여 넣어줄수도 있다.
	wcex.lpszClassName = WindowName;

	//아이콘을 추가한다.
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(SmallIconID));

	//윈도우 클래스 정보를 등록하는 함수
	RegisterClassExW(&wcex);
}

bool CEngine::Create(const TCHAR* WindowName, int Width, int Height)
{

	//CreateWindowW 등록된 레지스터 클래스 정보를 바탕으로 창을 생성한다.
	/*
	1. 윈도우 클래스 이름(식별번호)
	2. 윈도우 타이틀바에 출력할 이름
	3. 윈도우 창 스타일을 지정한다. WS_OVERLAPPEDWINDOW: 제목 표시줄과 테두리가있는 일반창
	4. 윈도우 창 시작 X좌표를 지정한다.
	5. 윈도우 창 시작 Y좌표를 지정한다.
	6. 가로 크기
	7. 세로 크기
	8. 부모 윈도우 핸들
	9. 메뉴 핸들
	10. 식별번호
	*/
	//함수가 정상적으로 실행되면 창을 컨트롤할수 있는 핸들이 반환된다.
	mhWnd = CreateWindowW(WindowName, TEXT("Game"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, Width, Height, nullptr, nullptr, mhInst, nullptr);

	//hWnd가 없으면 실패를 반환한다.
	if (!mhWnd)
	{
		return FALSE;
	}

	RECT WindowRect = { 0,0,Width,Height };

	//실제 화면의 크기를 불러오는 함수
	AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, false);

	//구해준 화면의 크기만큼 윈도우의 크기를 지정한다.
	SetWindowPos(mhWnd, HWND_TOPMOST, 0, 0, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top,
		SWP_NOMOVE | SWP_NOZORDER);

	//윈도우 창을 보여줄지 말지 결정한다.
	ShowWindow(mhWnd, SW_SHOW);

	//창을 새로 그린다.
	UpdateWindow(mhWnd);

	return TRUE;
}

LRESULT CEngine::WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hWnd, &ps);
		// TODO: 여기에 그리기 코드를 추가합니다...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		mLoop = false;
		PostQuitMessage(0);
		break;
	case WM_MOUSEWHEEL:
		CInput::AddWheelDelta((int)(short)HIWORD(wParam));
		break;
	case WM_SETFOCUS:
	case WM_ACTIVATE:
		// 파일 다이얼로그가 닫힌 후 DirectInput DISCL_FOREGROUND 재획득
		if (Message == WM_SETFOCUS ||
			(Message == WM_ACTIVATE && LOWORD(wParam) != WA_INACTIVE))
		{
			auto World = CWorldManager::GetInst()->GetWorld().lock();
			if (World)
			{
				if (auto Input = World->GetInput().lock())
					Input->DeviceAcquire();
			}
		}
		return DefWindowProc(hWnd, Message, wParam, lParam);
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
	}
	return 0;
}
