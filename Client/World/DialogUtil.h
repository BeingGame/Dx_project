#pragma once

#include <windows.h>
#include <commdlg.h>
#include <string>
#include "Engine.h"

#pragma comment(lib, "comdlg32.lib")

namespace DialogUtil
{
    // 실행 파일이 있는 디렉토리 절대경로 반환 (끝에 '\\' 포함)
    inline std::string GetExeDir()
    {
        char Path[MAX_PATH] = {};
        GetModuleFileNameA(nullptr, Path, MAX_PATH);
        std::string FullPath = Path;
        size_t Slash = FullPath.rfind('\\');
        return (Slash != std::string::npos) ? FullPath.substr(0, Slash + 1) : FullPath;
    }

    // 지정한 코드페이지로 해석해본다. 깨진 바이트가 하나라도 있으면 빈 문자열.
    // (MB_ERR_INVALID_CHARS를 줘야 U+FFFD로 뭉개지 않고 실패로 알려준다)
    inline std::wstring TryDecode(const std::string& Narrow, UINT CodePage)
    {
        int Length = MultiByteToWideChar(CodePage, MB_ERR_INVALID_CHARS,
                                      Narrow.c_str(), -1, nullptr, 0);
        if (Length <= 1) return {};

        std::wstring Wide(Length - 1, L'\0');
        MultiByteToWideChar(CodePage, MB_ERR_INVALID_CHARS, Narrow.c_str(), -1, &Wide[0], Length);
        return Wide;
    }

    // 좁은 문자열 → UTF-16. 코드페이지를 자동으로 판별한다.
    //
    // 에셋 파일(.anim2d 등)에 적힌 한글은 "저장한 PC의 ANSI 코드페이지"를 따른다.
    // 그런데 윈도우의 "UTF-8 베타 지원"을 켠 PC는 CP_ACP가 65001이라서,
    // CP949로 저장된 파일을 CP_ACP로 풀면 전부 U+FFFD가 되고
    // 텍스처 경로를 못 찾아 애니메이션에 텍스처가 안 붙는다.
    // (그 상태로 재생하면 CMaterial::SetTexture에서 빈 텍스처를 만진다)
    //
    // 그래서 UTF-8 → CP949 → CP_ACP 순으로 시도한다.
    // 아스키는 UTF-8에서 그대로 통과하므로 영향이 없다.
    inline std::wstring ToWide(const std::string& Narrow)
    {
        if (Narrow.empty()) return {};

        if (std::wstring Wide = TryDecode(Narrow, CP_UTF8); !Wide.empty())
            return Wide;

        if (std::wstring Wide = TryDecode(Narrow, 949); !Wide.empty())
            return Wide;

        //둘 다 아니면 시스템 코드페이지로 최대한 살려본다. (여기선 깨져도 그대로 둔다)
        int Length = MultiByteToWideChar(CP_ACP, 0, Narrow.c_str(), -1, nullptr, 0);
        if (Length <= 1) return {};

        std::wstring Wide(Length - 1, L'\0');
        MultiByteToWideChar(CP_ACP, 0, Narrow.c_str(), -1, &Wide[0], Length);
        return Wide;
    }

    // UTF-16 wide 문자열 → ANSI(CP_ACP) 문자열 변환 (ToWide의 역변환)
    inline std::string ToNarrow(const std::wstring& Wide)
    {
        if (Wide.empty()) return {};
        int Length = WideCharToMultiByte(CP_ACP, 0, Wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (Length <= 1) return {};
        std::string Narrow(Length - 1, '\0');
        WideCharToMultiByte(CP_ACP, 0, Wide.c_str(), -1, &Narrow[0], Length, nullptr, nullptr);
        return Narrow;
    }

    // 파일에서 읽은 문자열을 이 PC의 ANSI 코드페이지로 맞춰준다.
    //
    // GetExeDir / FindFirstFileA / ifstream 같은 A함수들은 전부 CP_ACP를 쓰므로,
    // 파일에서 읽은 문자열을 그대로 이어붙이면 코드페이지가 섞인 경로가 만들어진다.
    // (예: UTF-8인 exe 경로 + CP949인 상대 경로 → 어느 쪽으로 풀어도 깨진다)
    // 그래서 파일에서 읽은 직후에 한 번 통과시켜 준다.
    inline std::string ToAcp(const std::string& Narrow)
    {
        if (Narrow.empty()) return {};

        std::wstring Wide = ToWide(Narrow);
        if (Wide.empty()) return Narrow;

        int Length = WideCharToMultiByte(CP_ACP, 0, Wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (Length <= 1) return Narrow;

        std::string Result(Length - 1, '\0');
        WideCharToMultiByte(CP_ACP, 0, Wide.c_str(), -1, &Result[0], Length, nullptr, nullptr);
        return Result;
    }

    // 절대경로를 exe 기준 상대경로로 바꿔준다. exe 폴더 밖이면 절대경로 그대로 반환.
    inline std::string ToRelativePath(const std::string& FullPath)
    {
        std::string ExeDir = GetExeDir();

        if (FullPath.size() > ExeDir.size() &&
            _strnicmp(FullPath.c_str(), ExeDir.c_str(), ExeDir.size()) == 0)
        {
            return FullPath.substr(ExeDir.size());
        }

        return FullPath;
    }

    // 전체 경로에서 확장자를 제거한 파일명만 추출
    inline std::string ExtractBaseName(const std::string& FullPath)
    {
        size_t Slash = FullPath.rfind('\\');
        if (Slash == std::string::npos) Slash = FullPath.rfind('/');
        std::string Name = (Slash != std::string::npos) ? FullPath.substr(Slash + 1) : FullPath;
        size_t DotPos = Name.rfind('.');
        return (DotPos != std::string::npos) ? Name.substr(0, DotPos) : Name;
    }

    namespace Detail
    {
        // 다이얼로그 열기 전: ShowCursor 카운트를 루프로 올려 시스템 커서 확실히 표시
        // 게임이 SetMouseWidget에서 ShowCursor(FALSE)를 여러 번 호출해
        // 카운트가 음수일 수 있으므로 단순 +1이 아닌 루프 필요
        inline void ShowSystemCursor()
        {
            while (ShowCursor(TRUE) < 1) {}
        }

        // 다이얼로그 닫은 후: ShowCursor 카운트를 루프로 내려 시스템 커서 확실히 숨김
        // 파일 탐색기 주소창 등 내부에서 ShowCursor를 변경했을 수 있으므로
        // 단순 -1이 아닌 루프로 카운트 상태를 완전히 복원
        inline void HideSystemCursor()
        {
            while (ShowCursor(FALSE) >= 0) {}
        }

        // WM_SETFOCUS를 즉시 플러시 → DirectInput 재획득이 OpenFile 반환 전에 완료됨
        // (지연 처리하면 SetTextureFullPath/Rebuild 도중 입력이 끊겨 멈춤처럼 보임)
        inline void ReacquireInput(HWND hWnd)
        {
            PostMessage(hWnd, WM_SETFOCUS, 0, 0);
            MSG WinMessage = {};
            while (PeekMessage(&WinMessage, nullptr, 0, 0, PM_REMOVE))
            {
                if (WinMessage.message == WM_QUIT) { PostQuitMessage(0); break; }
                TranslateMessage(&WinMessage);
                DispatchMessage(&WinMessage);
            }
        }
    }

    // 파일 열기 대화상자 — 취소 시 빈 문자열 반환
    inline std::string OpenFile(const char* Filter, const char* InitialDir = nullptr)
    {
        HWND hWnd = CEngine::GetInst()->GetWindowHandle();
        Detail::ShowSystemCursor();

        char PathBuf[MAX_PATH] = {};
        OPENFILENAMEA OpenFileName   = {};
        OpenFileName.lStructSize     = sizeof(OpenFileName);
        OpenFileName.hwndOwner       = hWnd;
        OpenFileName.lpstrFilter     = Filter;
        OpenFileName.lpstrFile       = PathBuf;
        OpenFileName.nMaxFile        = MAX_PATH;
        OpenFileName.lpstrInitialDir = InitialDir;
        OpenFileName.Flags           = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
        bool bSucceeded = GetOpenFileNameA(&OpenFileName) != 0;

        Detail::HideSystemCursor();
        Detail::ReacquireInput(hWnd);
        return bSucceeded ? PathBuf : "";
    }

    // 사용자에게 꼭 보여야 하는 알림. (로그만 남기면 아무도 못 본다)
    // 파일 대화상자와 같은 방식으로 커서를 살렸다가 되돌린다.
    inline void Alert(const wchar_t* Message, const wchar_t* Title = L"알림")
    {
        HWND hWnd = CEngine::GetInst()->GetWindowHandle();
        Detail::ShowSystemCursor();

        MessageBoxW(hWnd, Message, Title, MB_OK | MB_ICONWARNING);

        Detail::HideSystemCursor();
        Detail::ReacquireInput(hWnd);
    }

    // 파일 저장 대화상자 — 취소 시 빈 문자열 반환
    // InitialFileName을 주면 그 경로/이름이 미리 채워진 채로 열린다.
    inline std::string SaveFile(const char* Filter, const char* InitialDir = nullptr,
                                const char* DefaultExt = nullptr,
                                const char* InitialFileName = nullptr)
    {
        HWND hWnd = CEngine::GetInst()->GetWindowHandle();
        Detail::ShowSystemCursor();

        char PathBuf[MAX_PATH] = {};

        if (InitialFileName && *InitialFileName)
        {
            strncpy_s(PathBuf, MAX_PATH, InitialFileName, _TRUNCATE);
        }

        OPENFILENAMEA OpenFileName   = {};
        OpenFileName.lStructSize     = sizeof(OpenFileName);
        OpenFileName.hwndOwner       = hWnd;
        OpenFileName.lpstrFilter     = Filter;
        OpenFileName.lpstrFile       = PathBuf;
        OpenFileName.nMaxFile        = MAX_PATH;
        OpenFileName.lpstrInitialDir = InitialDir;
        OpenFileName.lpstrDefExt     = DefaultExt;
        OpenFileName.Flags           = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
        bool bSucceeded = GetSaveFileNameA(&OpenFileName) != 0;

        Detail::HideSystemCursor();
        Detail::ReacquireInput(hWnd);
        return bSucceeded ? PathBuf : "";
    }
}
