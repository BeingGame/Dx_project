#pragma once

//장치를 가져오기위한 헤더와 매크로 선언
#define DIRECTINPUT_VERSION 0x0800 //dinput8 버전을 사용한다는 의미의 매크로

#define DIK_MOUSELBUTTON 0xfc
#define DIK_MOUSERBUTTON 0xfd
#define DIK_MOUSEWHEELBUTTON 0xfe

#include "../EngineInfo.h"

#include <dinput.h>

#pragma comment(lib,"dinput8.lib")

//인풋시스템을 위한 열거형
enum class EInputSystemType
{
	DInput,
	Window
};

namespace EInputType
{
	enum Type
	{
		Press,
		Hold,
		Release,
		End
	};
}

namespace EMouseType
{
	enum Type
	{
		LButton,
		RButton,
		MButton,
		End
	};
}

//키 상태 구조체
struct FKeyState
{
	unsigned char Key = 0;
	bool Press = false;
	bool Hold = false;
	bool Release = false;
};

//키입력을 바인드해줄 구조체
//인풋시스템에서 바인딩된다.
struct FBindKey
{
	FKeyState* Key = nullptr;
	bool Ctrl = false;
	bool Alt = false;
	bool Shift = false;
	std::function<void()> Func[EInputType::End];
	bool KeyHold = false;
};

class CInput
{
public:
	CInput();
	~CInput();

private:
	std::weak_ptr<class CWorld> mWorld;

	EInputSystemType mInputType = EInputSystemType::DInput;
	HINSTANCE mhInst;
	HWND mhWnd;

	//DInput 장치 객체
	IDirectInput8* mInput = nullptr;
	IDirectInputDevice8* mKeyboard = nullptr;
	IDirectInputDevice8* mMouse = nullptr;

	//DInput의 상태를 저장하기 위한 변수
	//
	unsigned char mKeyState[256] = {};
	DIMOUSESTATE mMouseState = {};

private:
	std::unordered_map<unsigned char, FKeyState*> mKeyStateMap;
	//unique_ptr로 만드는 이유는 키가 여러개가 존재할수 없기때문
	std::unordered_map<std::string, std::unique_ptr<FBindKey>> mBindKeyMap;

	//시스템키 상태
	bool mCtrlState[EInputType::End] = {};
	bool mAltState[EInputType::End] = {};
	bool mShiftState[EInputType::End] = {};

	//마우스 키 상태
	bool mMouseButton[EMouseType::End][EInputType::End] = {};

	//마우스의 위치
	FVector2 mMousePos;
	FVector2 mMouseWorldPos;
	//현재 마우스가 움직인 거리
	FVector2 mMouseMove;
	bool mMouseCheckStart = false;

public:
	void SetWorld(const std::weak_ptr<class CWorld> World)
	{
		mWorld = World;
	}

	bool GetMouseState(EMouseType::Type MouseType, EInputType::Type InputType) const
	{
		return mMouseButton[MouseType][InputType];
	}

	bool GetCtrl(EInputType::Type Type) const
	{
		return mCtrlState[Type];
	}
	bool GetAlt(EInputType::Type Type) const
	{
		return mAltState[Type];
	}
	bool GetShift(EInputType::Type Type) const
	{
		return mShiftState[Type];
	}

	const FVector2& GetMousePos() const
	{
		return mMousePos;
	}

	const FVector2& GetWorldMousePos() const
	{
		return mMouseWorldPos;
	}

	const FVector2& GetMouseMove() const
	{
		return mMouseMove;
	}

public:
	void DeviceAcquire();
	void DeviceUnAcquire();

	bool Init();
	void Update(float DeltaTime);
	
private:
	bool InitDevice();

	void UpdateKeyboard();
	void UpdateMouse();

	//갱신된 입력을 처리하는 함수
	//window와 dinput 두개의 방식을 처리할수 있는 함수를 각각 만들어준다.
	void UpdateInput(float DeltaTime);
	void UpdateDInput(float DeltaTime);
	void UpdateWindowInput(float DeltaTime);

	//바인드 키 처리함수
	void UpdateBindKey(float DeltaTime);

	void UpdateMousePos(float DeltaTime);


public:
	void AddBindKey(const std::string& Name, unsigned char Key);
	void SetKeyCtrl(const std::string& Name, bool Ctrl);
	void SetKeyAlt(const std::string& Name, bool Alt);
	void SetKeyShift(const std::string& Name, bool Shift);


private:
	FBindKey* FindBindKey(const std::string& Name);
	FKeyState* FindKeyState(unsigned char Key);

	//윈도우 입력을 DInput으로 변경하는 함수
	unsigned char ConvertKey(unsigned char Key);

public:
	template<typename T>
	void SetBindFunction(const std::string& Name, EInputType::Type Type, T* Object, void (T::* Func)())
	{
		FBindKey* Key = FindBindKey(Name);

		if (!Key)
		{
			return;
		}

		Key->Func[Type] = std::bind(Func, Object);
	}
};

