#include "Input.h"
#include "../Engine.h"
#include "../Device.h"

#include "World.h"
#include "CameraManager.h"

#include "../LogManager.h"

int CInput::sWheelAccum = 0;

CInput::CInput()
{}

CInput::~CInput()
{
	auto iter = mKeyStateMap.begin();
	auto iterEnd = mKeyStateMap.end();

	for (; iter != iterEnd; ++iter)
	{
		SAFE_DELETE(iter->second);
	}
}

void CInput::DeviceAcquire()
{
	if (mKeyboard)
	{
		mKeyboard->Acquire();
	}

	if (mMouse)
	{
		mMouse->Acquire();
	}
}

void CInput::DeviceUnAcquire()
{
	if (mKeyboard)
	{
		mKeyboard->Unacquire();
	}

	if (mMouse)
	{
		mMouse->Unacquire();
	}
}

bool CInput::Init()
{
	mhInst = CEngine::GetInst()->GetWindowInstance();
	mhWnd = CEngine::GetInst()->GetWindowHandle();

	if (FAILED(DirectInput8Create(mhInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&mInput, nullptr)))
	{
		mInputType = EInputSystemType::Window;
	}
	else
	{
		//성공하면 장치를 가져온다
		if (!InitDevice())
		{
			return false;
		}
	}

	return true;
}

void CInput::Update(float DeltaTime)
{
	//키보드와 마우스 입력을 받을때
	//Dinput일때 keyboard랑 mouse에서 입력을 받아온다.

	if (mInputType == EInputSystemType::DInput)
	{
		UpdateKeyboard();
		UpdateMouse();
	}

	//마우스 위치를 갱신한다.
	UpdateMousePos(DeltaTime);

	//입력 상태 갱신
	UpdateInput(DeltaTime);

	//입력된 키에 따라 조합키를 실행한다.
	UpdateBindKey(DeltaTime);
}

bool CInput::InitDevice()
{
	//키보다 장치를 먼저 생성한다.
	if (FAILED(mInput->CreateDevice(GUID_SysKeyboard, &mKeyboard, nullptr)))
	{
		return false;
	}

	//c_dfDIKeyboard포멧은 내부적으로 256바이트의 버퍼를 사용하도록 되어있다.
	mKeyboard->SetDataFormat(&c_dfDIKeyboard);

	//윈도우창이 창모드인지 아니면 전체화면인지 체크해서 키입력을 받도록 한다.

	/*
	DISCL_EXCLUSIVE     : 입력을 독점한다.
	DISCL_NONEXCLUSIVE  : 다른 앱과 입력을 공유한다.
	DISCL_FOREGROUND    : 내 윈도우가 활성화일 때만 입력을 받는다.
	DISCL_BACKGROUND    : 내 윈도우가 비활성이어도 입력을 받는다.
	DISCL_NOWINKEY      : 윈도우 키같은 시스템 키 입력을 막아준다.
	*/

	if (CDevice::GetInst()->GetWindowMode())
	{
		mKeyboard->SetCooperativeLevel(mhWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	}
	else
	{
		mKeyboard->SetCooperativeLevel(mhWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	}

	//시스템에 장치에 대한 제어권을 획득하는 함수
	//제어권이 없으면 키보드 입력을 하더라도 무시된다.
	mKeyboard->Acquire();

	//마우스 장치 생성
	if (FAILED(mInput->CreateDevice(GUID_SysMouse, &mMouse, nullptr)))
	{
		return false;
	}

	mMouse->SetDataFormat(&c_dfDIMouse);

	if (CDevice::GetInst()->GetWindowMode())
	{
		mMouse->SetCooperativeLevel(mhWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	}
	else
	{
		mMouse->SetCooperativeLevel(mhWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	}

	mMouse->Acquire();

	return true;
}

void CInput::UpdateKeyboard()
{
	//DInput으로 얻어온 키보드에서 키입력에 대한 State를 받아오는 함수
	if (!mKeyboard)
	{
		return;
	}

	//GetDeviceState : 키보드 키가 눌렸는지 안눌렸는지를 가져오는 함수
	//0x80이 1이면 눌림, 0이면 떼짐
	HRESULT Result = mKeyboard->GetDeviceState(256, (LPVOID)&mKeyState);

	//실패했다면 먼저 제어권을 다시 가져온다.
	//키상태를 재확인
	if (FAILED(Result))
	{
		// 장치 분리/포커스 손실 시 구버퍼 클리어 → 키 고착 방지
		ZeroMemory(mKeyState, sizeof(mKeyState));

		//장치가 끊기거나, 포커스르 잃었을때 DIERR_INPUTLOST가 나온다.
		//이를 acquire함수로 다시 제어권을 가져오게한다.
		//DIERR_NOTACQUIRED : 장치 획득이 아직 되지 않은 상태
		if (Result == DIERR_INPUTLOST || Result == DIERR_NOTACQUIRED)
		{
			if (SUCCEEDED(mKeyboard->Acquire()))
			{
				mKeyboard->GetDeviceState(256, (LPVOID)&mKeyState);
			}
		}
	}

}

void CInput::UpdateMouse()
{
	if (!mMouse)
	{
		return;
	}

	//GetDeviceState : 키보드 키가 눌렸는지 안눌렸는지를 가져오는 함수
	//0x80이 1이면 눌림, 0이면 떼짐
	HRESULT Result = mMouse->GetDeviceState(sizeof(mMouseState), (LPVOID)&mMouseState);

	//실패했다면 먼저 제어권을 다시 가져온다.
	//키상태를 재확인
	if (FAILED(Result))
	{
		// 장치 분리/포커스 손실 시 구버퍼 클리어 → 버튼 고착 방지
		// (다이얼로그 사용 후 DISCL_FOREGROUND 언어콰이어 타이밍에서
		//  LButton pressed 상태가 남아 게임이 멈추는 현상 수정)
		ZeroMemory(&mMouseState, sizeof(mMouseState));

		//장치가 끊기거나, 포커스르 잃었을때 DIERR_INPUTLOST가 나온다.
		//이를 acquire함수로 다시 제어권을 가져오게한다.
		//DIERR_NOTACQUIRED : 장치 획득이 아직 되지 않은 상태
		if (Result == DIERR_INPUTLOST || Result == DIERR_NOTACQUIRED)
		{
			if (SUCCEEDED(mMouse->Acquire()))
			{
				mMouse->GetDeviceState(sizeof(mMouseState), (LPVOID)&mMouseState);
			}
		}
	}
}

void CInput::UpdateInput(float DeltaTime)
{
	switch (mInputType)
	{
	case EInputSystemType::DInput:
		UpdateDInput(DeltaTime);
		break;
	case EInputSystemType::Window:
		UpdateWindowInput(DeltaTime);
		break;
	default:
		break;
	}
}

void CInput::UpdateDInput(float DeltaTime)
{
	//먼저 시스템키들의 상태를 확인한다.
	//컨트롤이 눌렸을 경우
	if (mKeyState[DIK_LCONTROL] & 0x80)
	{
		//현재 눌린 상태이므로 release를 false로 변경한다.
		mCtrlState[EInputType::Release] = false;

		//이제 막 눌린상태
		if (!mCtrlState[EInputType::Press] && !mCtrlState[EInputType::Hold])
		{
			mCtrlState[EInputType::Press] = true;
			mCtrlState[EInputType::Hold] = true;
		}
		//이전 프레임에서도 press 상태라면
		else
		{
			mCtrlState[EInputType::Press] = false;
		}
	}
	//컨트롤이 떼진경우
	else if (mCtrlState[EInputType::Hold])
	{
		mCtrlState[EInputType::Press] = false;
		mCtrlState[EInputType::Hold] = false;
		mCtrlState[EInputType::Release] = true;
	}
	//컨트롤이 떼진상태고, Release가 true일때
	else if (mCtrlState[EInputType::Release])
	{
		mCtrlState[EInputType::Release] = false;
	}

	//알트
	if (mKeyState[DIK_LALT] & 0x80)
	{
		//현재 눌린 상태이므로 release를 false로 변경한다.
		mAltState[EInputType::Release] = false;

		//이제 막 눌린상태
		if (!mAltState[EInputType::Press] && !mAltState[EInputType::Hold])
		{
			mAltState[EInputType::Press] = true;
			mAltState[EInputType::Hold] = true;
		}
		//이전 프레임에서도 press 상태라면
		else
		{
			mAltState[EInputType::Press] = false;
		}
	}
	else if (mAltState[EInputType::Hold])
	{
		mAltState[EInputType::Press] = false;
		mAltState[EInputType::Hold] = false;
		mAltState[EInputType::Release] = true;
	}
	else if (mAltState[EInputType::Release])
	{
		mAltState[EInputType::Release] = false;
	}

	if (mKeyState[DIK_LSHIFT] & 0x80)
	{
		//현재 눌린 상태이므로 release를 false로 변경한다.
		mShiftState[EInputType::Release] = false;

		//이제 막 눌린상태
		if (!mShiftState[EInputType::Press] && !mShiftState[EInputType::Hold])
		{
			mShiftState[EInputType::Press] = true;
			mShiftState[EInputType::Hold] = true;
		}
		//이전 프레임에서도 press 상태라면
		else
		{
			mShiftState[EInputType::Press] = false;
		}
	}
	else if (mShiftState[EInputType::Hold])
	{
		mShiftState[EInputType::Press] = false;
		mShiftState[EInputType::Hold] = false;
		mShiftState[EInputType::Release] = true;
	}
	else if (mShiftState[EInputType::Release])
	{
		mShiftState[EInputType::Release] = false;
	}

	//시스템키 확인이후 직접적인 키입력을 확인한다.

	//먼저 마우스버튼의 입력을 확인한다.
	for (int i = 0; i < EMouseType::End; ++i)
	{
		//버튼인덱스 0= 왼쪽 1= 오른쪽 2= 휠버튼
		if (mMouseState.rgbButtons[i] & 0x80)
		{
			//현재 눌린 상태이므로 release를 false로 변경한다.
			mMouseButton[i][EInputType::Release] = false;

			//이제 막 눌린상태
			if (!mMouseButton[i][EInputType::Press] && !mMouseButton[i][EInputType::Hold])
			{
				mMouseButton[i][EInputType::Press] = true;
				mMouseButton[i][EInputType::Hold] = true;
			}
			//이전 프레임에서도 press 상태라면
			else
			{
				mMouseButton[i][EInputType::Press] = false;
			}
		}
		else if (mMouseButton[i][EInputType::Hold])
		{
			mMouseButton[i][EInputType::Press] = false;
			mMouseButton[i][EInputType::Hold] = false;
			mMouseButton[i][EInputType::Release] = true;
		}
		else if (mMouseButton[i][EInputType::Release])
		{
			mMouseButton[i][EInputType::Release] = false;
		}
	}

	// DirectInput 마우스 lZ = 휠 누적 델타 (양수=위, 음수=아래)
	mMouseWheelDelta = (float)mMouseState.lZ;

	//인풋시스템에서 바인드된 키를 확인한다.

	auto iter = mKeyStateMap.begin();
	auto iterEnd = mKeyStateMap.end();

	for (; iter != iterEnd; ++iter)
	{
		//인풋시스템에 키가 입력된 상황을 전부 저장해준다.
		switch (iter->second->Key)
		{
		case DIK_MOUSELBUTTON:
			iter->second->Press = mMouseButton[EMouseType::LButton][EInputType::Press];
			iter->second->Release = mMouseButton[EMouseType::LButton][EInputType::Release];
			iter->second->Hold = mMouseButton[EMouseType::LButton][EInputType::Hold];
			break;
		case DIK_MOUSERBUTTON:
			iter->second->Press = mMouseButton[EMouseType::RButton][EInputType::Press];
			iter->second->Release = mMouseButton[EMouseType::RButton][EInputType::Release];
			iter->second->Hold = mMouseButton[EMouseType::RButton][EInputType::Hold];
			break;
		case DIK_MOUSEWHEELBUTTON:
			iter->second->Press = mMouseButton[EMouseType::MButton][EInputType::Press];
			iter->second->Release = mMouseButton[EMouseType::MButton][EInputType::Release];
			iter->second->Hold = mMouseButton[EMouseType::MButton][EInputType::Hold];
			break;
		default:
			//시스템키와 동일하게 먼저 press,hold,release처리를 일괄적으로 한다.

			if (mKeyState[iter->second->Key] & 0x80)
			{
				//현재 눌린 상태이므로 release를 false로 변경한다.
				iter->second->Release = false;

				//이제 막 눌린상태
				if (!iter->second->Press && !iter->second->Hold)
				{
					iter->second->Press = true;
					iter->second->Hold = true;
				}
				//이전 프레임에서도 press 상태라면
				else
				{
					iter->second->Press = false;
				}
			}
			else if (iter->second->Hold)
			{
				iter->second->Press = false;
				iter->second->Hold = false;
				iter->second->Release = true;
			}
			else if (iter->second->Release)
			{
				iter->second->Release = false;
			}

			break;
		}
	}
}

void CInput::UpdateWindowInput(float DeltaTime)
{
	//먼저 시스템키들의 상태를 확인한다.
		//컨트롤이 눌렸을 경우
	if (GetAsyncKeyState(VK_LCONTROL) & 0x8000)
	{
		//현재 눌린 상태이므로 release를 false로 변경한다.
		mCtrlState[EInputType::Release] = false;

		//이제 막 눌린상태
		if (!mCtrlState[EInputType::Press] && !mCtrlState[EInputType::Hold])
		{
			mCtrlState[EInputType::Press] = true;
			mCtrlState[EInputType::Hold] = true;
		}
		//이전 프레임에서도 press 상태라면
		else
		{
			mCtrlState[EInputType::Press] = false;
		}
	}
	//컨트롤이 떼진경우
	else if (mCtrlState[EInputType::Hold])
	{
		mCtrlState[EInputType::Press] = false;
		mCtrlState[EInputType::Hold] = false;
		mCtrlState[EInputType::Release] = true;
	}
	//컨트롤이 떼진상태고, Release가 true일때
	else if (mCtrlState[EInputType::Release])
	{
		mCtrlState[EInputType::Release] = false;
	}

	//알트
	if (GetAsyncKeyState(VK_LMENU) & 0x8000)
	{
		//현재 눌린 상태이므로 release를 false로 변경한다.
		mAltState[EInputType::Release] = false;

		//이제 막 눌린상태
		if (!mAltState[EInputType::Press] && !mAltState[EInputType::Hold])
		{
			mAltState[EInputType::Press] = true;
			mAltState[EInputType::Hold] = true;
		}
		//이전 프레임에서도 press 상태라면
		else
		{
			mAltState[EInputType::Press] = false;
		}
	}
	else if (mAltState[EInputType::Hold])
	{
		mAltState[EInputType::Press] = false;
		mAltState[EInputType::Hold] = false;
		mAltState[EInputType::Release] = true;
	}
	else if (mAltState[EInputType::Release])
	{
		mAltState[EInputType::Release] = false;
	}

	if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
	{
		//현재 눌린 상태이므로 release를 false로 변경한다.
		mShiftState[EInputType::Release] = false;

		//이제 막 눌린상태
		if (!mShiftState[EInputType::Press] && !mShiftState[EInputType::Hold])
		{
			mShiftState[EInputType::Press] = true;
			mShiftState[EInputType::Hold] = true;
		}
		//이전 프레임에서도 press 상태라면
		else
		{
			mShiftState[EInputType::Press] = false;
		}
	}
	else if (mShiftState[EInputType::Hold])
	{
		mShiftState[EInputType::Press] = false;
		mShiftState[EInputType::Hold] = false;
		mShiftState[EInputType::Release] = true;
	}
	else if (mShiftState[EInputType::Release])
	{
		mShiftState[EInputType::Release] = false;
	}


	int MouseKey[EMouseType::End] =
	{
		VK_LBUTTON,
		VK_RBUTTON,
		VK_MBUTTON
	};

	//시스템키 확인이후 직접적인 키입력을 확인한다.

	//먼저 마우스버튼의 입력을 확인한다.
	for (int i = 0; i < EMouseType::End; ++i)
	{
		//버튼인덱스 0= 왼쪽 1= 오른쪽 2= 휠버튼
		if (GetAsyncKeyState(MouseKey[i]) & 0x8000)
		{
			//현재 눌린 상태이므로 release를 false로 변경한다.
			mMouseButton[i][EInputType::Release] = false;

			//이제 막 눌린상태
			if (!mMouseButton[i][EInputType::Press] && !mMouseButton[i][EInputType::Hold])
			{
				mMouseButton[i][EInputType::Press] = true;
				mMouseButton[i][EInputType::Hold] = true;
			}
			//이전 프레임에서도 press 상태라면
			else
			{
				mMouseButton[i][EInputType::Press] = false;
			}
		}
		else if (mMouseButton[i][EInputType::Hold])
		{
			mMouseButton[i][EInputType::Press] = false;
			mMouseButton[i][EInputType::Hold] = false;
			mMouseButton[i][EInputType::Release] = true;
		}
		else if (mMouseButton[i][EInputType::Release])
		{
			mMouseButton[i][EInputType::Release] = false;
		}
	}

	// Window 입력 모드: WM_MOUSEWHEEL로 누적된 값 읽고 초기화
	mMouseWheelDelta = (float)sWheelAccum;
	sWheelAccum = 0;

	//인풋시스템에서 바인드된 키를 확인한다.

	auto iter = mKeyStateMap.begin();
	auto iterEnd = mKeyStateMap.end();

	for (; iter != iterEnd; ++iter)
	{
		//시스템키와 동일하게 먼저 press,hold,release처리를 일괄적으로 한다.
		//윈도우는 입력을 하나로 처리한다.
		if (GetAsyncKeyState(iter->first) & 0x8000)
		{
			//현재 눌린 상태이므로 release를 false로 변경한다.
			iter->second->Release = false;

			//이제 막 눌린상태
			if (!iter->second->Press && !iter->second->Hold)
			{
				iter->second->Press = true;
				iter->second->Hold = true;
			}
			//이전 프레임에서도 press 상태라면
			else
			{
				iter->second->Press = false;
			}
		}
		else if (iter->second->Hold)
		{
			iter->second->Press = false;
			iter->second->Hold = false;
			iter->second->Release = true;
		}
		else if (iter->second->Release)
		{
			iter->second->Release = false;
		}

	}

}

void CInput::UpdateBindKey(float DeltaTime)
{
	//직접적으로 키의 콜백함수를 부르는 함수

	//키가 입력되었을때 시스템키를 포함하여
	//키가 정상적으로 입력되었을때 콜백함수를 호출한다.

	auto iter = mBindKeyMap.begin();
	auto iterEnd = mBindKeyMap.end();

	for (; iter != iterEnd; ++iter)
	{
		//먼저 Press
		//키가 눌렸을때 설정된 시스템키를 포함해서 키가 눌리면 콜백함수를 호출한다.
		//Press는 프레임이 워낙 빨라 확인용도로는 보통 사용하지 않는다.
		if (iter->second->Key->Press &&
			iter->second->Ctrl == mCtrlState[EInputType::Hold] &&
			iter->second->Alt == mAltState[EInputType::Hold] &&
			iter->second->Shift == mShiftState[EInputType::Hold])
		{
			if (iter->second->Func[EInputType::Press])
			{
				iter->second->Func[EInputType::Press]();
			}
		}

		//Hold
		if (iter->second->Key->Hold &&
			iter->second->Ctrl == mCtrlState[EInputType::Hold] &&
			iter->second->Alt == mAltState[EInputType::Hold] &&
			iter->second->Shift == mShiftState[EInputType::Hold])
		{
			//현재 키가 홀드상태인지 확인한다.
			iter->second->KeyHold = true;

			//LOG_DEBUG(true, "Hold");

			if (iter->second->Func[EInputType::Hold])
			{
				iter->second->Func[EInputType::Hold]();
			}
		}

		//Release
		//release는 조합키가 하나라도(시스템키 포함) 떼지면 release
		//확인용 불변수
		bool Verification = false;

		//Ctrl을 사용하는 경우
		if (iter->second->Ctrl)
		{
			if (mCtrlState[EInputType::Release])
			{
				Verification = true;
			}
		}
		//사용되지 않았을 경우
		//press나 hold 상태가 아니여야한다.
		else
		{
			if (mCtrlState[EInputType::Hold])
			{
				Verification = true;
			}
		}

		if (iter->second->Alt)
		{
			if (mAltState[EInputType::Release])
			{
				Verification = true;
			}
		}
		//사용되지 않았을 경우
		//press나 hold 상태가 아니여야한다.
		else
		{
			if (mAltState[EInputType::Hold])
			{
				Verification = true;
			}
		}

		if (iter->second->Shift)
		{
			if (mShiftState[EInputType::Release])
			{
				Verification = true;
			}
		}
		//사용되지 않았을 경우
		//press나 hold 상태가 아니여야한다.
		else
		{
			if (mShiftState[EInputType::Hold])
			{
				Verification = true;
			}
		}

		//키가 release상태이거나 조합키가 변경되었을경우 release함수를 호출한다.
		if ((iter->second->Key->Release || Verification) && iter->second->KeyHold)
		{
			iter->second->KeyHold = false;

			if (iter->second->Func[EInputType::Release])
			{
				iter->second->Func[EInputType::Release]();
			}
		}
	}
}

void CInput::UpdateMousePos(float DeltaTime)
{
	//마우스의 위치를 화면의 Screen좌표로 구해준다.
	POINT MousePT;

	//GetCursorPos함수를 사용해 Screen좌표를 받아온다.
	GetCursorPos(&MousePT);

	//클라이언트 좌표로 변경한다.
	//ScreenToClient함수를 사용한다.
	ScreenToClient(mhWnd, &MousePT);

	//디바이스의 해상도와 윈도우의 크기가 다를수 있다.
	//크기 비율을 이용해서 마우스 좌표를 디바이스 공간의 좌표로 변경한다.

	//따라서 해상도의 비율을 가져와서 마우스 좌표에 곱해줘 최종적으로 어느 위치에 있는지
	//좌표를 계산해준다.

	//ex) 해상도는 1280 720, 크기는 1920 1080일때 마우스좌표를 1280 720에 맞춰준다.

	//먼저 해상도의 비율을 가져와준다.
	FVector2 Ratio = CDevice::GetInst()->GetResolutionRatio();
	FResolution ViewportRS = CDevice::GetInst()->GetResolution();

	//마우스의 좌표를 변환해준다.
	FVector2 MousePos;

	MousePos.x = MousePT.x * Ratio.x;
	MousePos.y = MousePT.y * Ratio.y;

	//마우스 좌표를 확인하면 x가 좌에서 우로 0 ~ 해상도 크기 만큼
	//위에서 아래로 y가 -0 ~ +해상도 크기 만큼
	//월드 좌표에선 x가 좌에서 우로 -해상도 ~ +해상도 만큼
	//y가 위에서 아래로 +해상도 ~ -해상도

	//클라이언트 영역과 월드의 y좌표는 서로 반대방향이다.
	FVector2 MouseWorldPos = MousePos;

	//따라서 y좌표를 반전시켜 먼저 y좌표의 월드 위치를 잡아준다.
	//현재 해상도가 720일때 현재 마우스 위치만큼 빼줘서 y좌표를 반전시킨다.
	MouseWorldPos.y = ViewportRS.Height - MousePos.y;

	if (mMouseCheckStart)
	{
		//마우스의 이동속도를 구해준다.
		mMouseMove = MousePos - mMousePos;
	}
	else
	{
		mMouseCheckStart = true;
	}

	mMousePos = MousePos;

	//최종 월드 위치
	//카메라의 위치에서 위에서 구한 마우스의 월드 공간 좌표를 더해 구해준다.
	auto World = mWorld.lock();

	auto CameraMgr = World->GetCameraManager().lock();

	FVector3 CameraPos = CameraMgr->GetMainCameraPos();

	//위치를 구할때 해상도의 절반만큼 빼준다.
	//해상도의 절반만큼 빼주는 이유는 월드 좌표가 원점을 기준으로 -해상도 ~ +해상도 이기때문
	mMouseWorldPos.x = CameraPos.x + MouseWorldPos.x - ViewportRS.Width * 0.5f;
	mMouseWorldPos.y = CameraPos.y + MouseWorldPos.y - ViewportRS.Height * 0.5f;
}

void CInput::AddBindKey(const std::string& Name, unsigned char Key)
{
	//키 등록
	//먼저 키가 등록되어있는지 체크한다.

	auto iter = mBindKeyMap.find(Name);

	if (iter != mBindKeyMap.end())
	{
		return;
	}

	std::unique_ptr<FBindKey> NewKey = std::make_unique<FBindKey>();

	FKeyState* State = FindKeyState(Key);

	if (!State)
	{
		State = new FKeyState;

		State->Key = ConvertKey(Key);

		mKeyStateMap.insert(std::make_pair(Key, State));

	}

	NewKey->Key = State;

	//소유권을 옮겨준다.
	mBindKeyMap.insert(std::make_pair(Name, std::move(NewKey)));
}

void CInput::SetKeyCtrl(const std::string& Name, bool Ctrl)
{
	auto Key = FindBindKey(Name);

	if (!Key)
	{
		return;
	}

	Key->Ctrl = Ctrl;
}

void CInput::SetKeyAlt(const std::string& Name, bool Alt)
{
	auto Key = FindBindKey(Name);

	if (!Key)
	{
		return;
	}

	Key->Alt = Alt;
}

void CInput::SetKeyShift(const std::string& Name, bool Shift)
{
	auto Key = FindBindKey(Name);

	if (!Key)
	{
		return;
	}

	Key->Shift = Shift;
}

FBindKey* CInput::FindBindKey(const std::string& Name)
{
	auto iter = mBindKeyMap.find(Name);

	if (iter == mBindKeyMap.end())
	{
		return nullptr;
	}

	return iter->second.get();
}

FKeyState* CInput::FindKeyState(unsigned char Key)
{
	auto iter = mKeyStateMap.find(Key);

	if (iter == mKeyStateMap.end())
	{
		return nullptr;
	}

	return iter->second;
}

unsigned char CInput::ConvertKey(unsigned char Key)
{
	if (mInputType == EInputSystemType::DInput)
	{
		switch (Key)
		{
		case VK_LBUTTON:
			return DIK_MOUSELBUTTON;
		case VK_RBUTTON:
			return DIK_MOUSERBUTTON;
		case VK_MBUTTON:
			return DIK_MOUSEWHEELBUTTON;
		case VK_BACK:
			return DIK_BACK;
		case VK_TAB:
			return DIK_TAB;
		case VK_RETURN:
			return DIK_RETURN;
		case VK_LCONTROL:
			return DIK_LCONTROL;
		case VK_RCONTROL:
			return DIK_RCONTROL;
		case VK_LMENU:
			return DIK_LALT;
		case VK_RMENU:
			return DIK_RALT;
		case VK_LSHIFT:
			return DIK_LSHIFT;
		case VK_RSHIFT:
			return DIK_RSHIFT;
		case VK_PAUSE:
			return DIK_PAUSE;
		case VK_CAPITAL:
			return DIK_CAPSLOCK;
		case VK_ESCAPE:
			return DIK_ESCAPE;
		case VK_SPACE:
			return DIK_SPACE;
		case VK_NEXT:
			return DIK_PGDN;
		case VK_PRIOR:
			return DIK_PGUP;
		case VK_END:
			return DIK_END;
		case VK_HOME:
			return DIK_HOME;
		case VK_LEFT:
			return DIK_LEFT;
		case VK_UP:
			return DIK_UP;
		case VK_RIGHT:
			return DIK_RIGHT;
		case VK_DOWN:
			return DIK_DOWN;
		case VK_PRINT:
			return DIK_SYSRQ;
		case VK_INSERT:
			return DIK_INSERT;
		case VK_DELETE:
			return DIK_DELETE;
		case VK_HELP:
			return 0;
		case '0':
			return DIK_0;
		case '1':
			return DIK_1;
		case '2':
			return DIK_2;
		case '3':
			return DIK_3;
		case '4':
			return DIK_4;
		case '5':
			return DIK_5;
		case '6':
			return DIK_6;
		case '7':
			return DIK_7;
		case '8':
			return DIK_8;
		case '9':
			return DIK_9;
		case 'A':
			return DIK_A;
		case 'B':
			return DIK_B;
		case 'C':
			return DIK_C;
		case 'D':
			return DIK_D;
		case 'E':
			return DIK_E;
		case 'F':
			return DIK_F;
		case 'G':
			return DIK_G;
		case 'H':
			return DIK_H;
		case 'I':
			return DIK_I;
		case 'J':
			return DIK_J;
		case 'K':
			return DIK_K;
		case 'L':
			return DIK_L;
		case 'M':
			return DIK_M;
		case 'N':
			return DIK_N;
		case 'O':
			return DIK_O;
		case 'P':
			return DIK_P;
		case 'Q':
			return DIK_Q;
		case 'R':
			return DIK_R;
		case 'S':
			return DIK_S;
		case 'T':
			return DIK_T;
		case 'U':
			return DIK_U;
		case 'V':
			return DIK_V;
		case 'W':
			return DIK_W;
		case 'X':
			return DIK_X;
		case 'Y':
			return DIK_Y;
		case 'Z':
			return DIK_Z;
		case VK_OEM_3:
			return DIK_GRAVE;
		case VK_OEM_MINUS:
			return DIK_MINUS;
		case VK_OEM_NEC_EQUAL:
			return DIK_EQUALS;
		case VK_OEM_4:
			return DIK_LBRACKET;
		case VK_OEM_6:
			return DIK_RBRACKET;
		case VK_OEM_5:
			return DIK_BACKSLASH;
		case VK_OEM_1:
			return DIK_SEMICOLON;
		case VK_OEM_7:
			return DIK_APOSTROPHE;
		case VK_OEM_COMMA:
			return DIK_COMMA;
		case VK_OEM_PERIOD:
			return DIK_PERIOD;
		case VK_OEM_2:
			return DIK_SLASH;
		case VK_NUMPAD0:
			return DIK_NUMPAD0;
		case VK_NUMPAD1:
			return DIK_NUMPAD1;
		case VK_NUMPAD2:
			return DIK_NUMPAD2;
		case VK_NUMPAD3:
			return DIK_NUMPAD3;
		case VK_NUMPAD4:
			return DIK_NUMPAD4;
		case VK_NUMPAD5:
			return DIK_NUMPAD5;
		case VK_NUMPAD6:
			return DIK_NUMPAD6;
		case VK_NUMPAD7:
			return DIK_NUMPAD7;
		case VK_NUMPAD8:
			return DIK_NUMPAD8;
		case VK_NUMPAD9:
			return DIK_NUMPAD9;
		case VK_MULTIPLY:
			return DIK_MULTIPLY;
		case VK_ADD:
			return DIK_ADD;
		case VK_SEPARATOR:
			return DIK_NUMPADCOMMA;
		case VK_SUBTRACT:
			return DIK_SUBTRACT;
		case VK_DECIMAL:
			return DIK_DECIMAL;
		case VK_DIVIDE:
			return DIK_DIVIDE;
			//case VK_RETURN:		
			//return DIK_NUMPADENTER;
		case VK_F1:
			return DIK_F1;
		case VK_F2:
			return DIK_F2;
		case VK_F3:
			return DIK_F3;
		case VK_F4:
			return DIK_F4;
		case VK_F5:
			return DIK_F5;
		case VK_F6:
			return DIK_F6;
		case VK_F7:
			return DIK_F7;
		case VK_F8:
			return DIK_F8;
		case VK_F9:
			return DIK_F9;
		case VK_F10:
			return DIK_F10;
		case VK_F11:
			return DIK_F11;
		case VK_F12:
			return DIK_F12;
		case VK_F13:
			return DIK_F13;
		case VK_F14:
			return DIK_F14;
		case VK_F15:
			return DIK_F15;
		case VK_F16:
		case VK_F17:
		case VK_F18:
		case VK_F19:
		case VK_F20:
		case VK_F21:
		case VK_F22:
		case VK_F23:
		case VK_F24:
			return 0;
		case VK_NUMLOCK:
			return DIK_NUMLOCK;
		case VK_SCROLL:
			return DIK_SCROLL;
		case VK_LWIN:
			return DIK_LWIN;
		case VK_RWIN:
			return DIK_RWIN;
		case VK_APPS:
			return DIK_APPS;
		case VK_OEM_102:
			return DIK_OEM_102;
		}

		return 0xff;
	}

	return Key;
}
