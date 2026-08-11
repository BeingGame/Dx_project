#pragma once

#include "EngineInfo.h"

class CEngine
{
	//엔진 클래스의 경우 여러개가 필요없어서 싱글톤으로 생성한다.
	Singleton(CEngine);

private:
	HINSTANCE mhInst = nullptr;
	HWND mhWnd = nullptr;
	static bool mLoop;

public:
	HINSTANCE GetWindowInstance() const
	{
		return mhInst;
	}

	HWND GetWindowHandle() const
	{
		return mhWnd;
	}

public:
	bool Init(HINSTANCE Inst, const TCHAR* WindowName, int IconID, int SmallIconID, int Width, int Height, bool WindowMode = true);

	int Run();

	void Destroy();

	//
	void Logic();

	//오브젝트 갱신 함수
	void Update(float DeltaTime);

	//갱신된 오브젝트들을 화면에 그려주는 함수
	void Render();

public:
	void InitRegisterClass(const TCHAR* WindowName, int IConID, int SmallIconID);

	bool Create(const TCHAR* WindowName, int Width, int Height);

	static LRESULT WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
};

