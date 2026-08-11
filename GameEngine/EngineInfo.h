#pragma once

//라이브러리
//미리 컴파일해놓은 코드의 모음집
//ex) printf, scanf_s와 같은 기능들도 C 표준 라이브러리에 포함되어 있다.

//정적, 동적

//정적 라이브러리 (.lib)
//컴파일 시점에 라이브러리 코드가 실행파일에 통째로 복사된다.
//라이브러리가 변경되면 전체 재컴파일이 필요, 같은 라이브러리 쓰는 프로그램이면 각자 메모리에 중복된 코드를 적재하게된다.

//동적 라이브러리(.dll)
//컴파일 시점에 링크만 걸어두고, 실행할때 필요한 라이브러리를 메모리에 불러온다.
//파일 크기가 정적라이브러리보다 작다. dll만 변경하면 재컴파일 불필요하다.
//중복 프로그램에서 메모리에서 공유해서 사용할수 있다.
//상대적으로 느리다.

#include <Windows.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <stack>

#include <functional>

#include <memory>

#include "Macro.h"

#include <crtdbg.h>

//DirectX3D11의 핵심 API
#include <d3d11.h>
//HLSL을 런타임에 컴파일할때 쓰는 함수가 있는곳
#include <d3dcompiler.h>

#include "wrl.h"
using namespace Microsoft::WRL;

#include "Math/Matrix.h"
#include "Enums.h"
#include "Structs.h"

//#pragma comment(확장자,"파일명.확장자")

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

//각종 인터페이스가 가진 guid 실제 값이 정의된 곳
#pragma comment(lib,"dxguid.lib")

//폰트 출력을 위한 DX2D
#include <dwrite_3.h>
#include <d2d1.h>

#pragma comment(lib,"dwrite.lib")
#pragma comment(lib,"d2d1.lib")

//robocopy <복사할 폴더> <복사될 폴더> <복사할 파일> /S <- 하위폴더를 포함해서 복사를 하라는 옵션

