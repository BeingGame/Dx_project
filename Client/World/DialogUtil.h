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
        std::string S = Path;
        size_t Slash = S.rfind('\\');
        return (Slash != std::string::npos) ? S.substr(0, Slash + 1) : S;
    }

    // ANSI(CP_ACP) 문자열 → UTF-16 wide 문자열 변환 (한글 경로 지원)
    inline std::wstring ToWide(const std::string& Narrow)
    {
        if (Narrow.empty()) return {};
        int Len = MultiByteToWideChar(CP_ACP, 0, Narrow.c_str(), -1, nullptr, 0);
        std::wstring Wide(Len - 1, L'\0');
        MultiByteToWideChar(CP_ACP, 0, Narrow.c_str(), -1, &Wide[0], Len);
        return Wide;
    }

    // 전체 경로에서 확장자를 제거한 파일명만 추출
    inline std::string ExtractBaseName(const std::string& FullPath)
    {
        size_t Slash = FullPath.rfind('\\');
        if (Slash == std::string::npos) Slash = FullPath.rfind('/');
        std::string Name = (Slash != std::string::npos) ? FullPath.substr(Slash + 1) : FullPath;
        size_t Dot = Name.rfind('.');
        return (Dot != std::string::npos) ? Name.substr(0, Dot) : Name;
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
            MSG msg = {};
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT) { PostQuitMessage(0); break; }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    // 파일 열기 대화상자 — 취소 시 빈 문자열 반환
    inline std::string OpenFile(const char* Filter, const char* InitialDir = nullptr)
    {
        HWND hWnd = CEngine::GetInst()->GetWindowHandle();
        Detail::ShowSystemCursor();

        char PathBuf[MAX_PATH] = {};
        OPENFILENAMEA Ofn   = {};
        Ofn.lStructSize     = sizeof(Ofn);
        Ofn.hwndOwner       = hWnd;
        Ofn.lpstrFilter     = Filter;
        Ofn.lpstrFile       = PathBuf;
        Ofn.nMaxFile        = MAX_PATH;
        Ofn.lpstrInitialDir = InitialDir;
        Ofn.Flags           = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
        bool Ok = GetOpenFileNameA(&Ofn) != 0;

        Detail::HideSystemCursor();
        Detail::ReacquireInput(hWnd);
        return Ok ? PathBuf : "";
    }

    // 파일 저장 대화상자 — 취소 시 빈 문자열 반환
    inline std::string SaveFile(const char* Filter, const char* InitialDir = nullptr,
                                const char* DefaultExt = nullptr)
    {
        HWND hWnd = CEngine::GetInst()->GetWindowHandle();
        Detail::ShowSystemCursor();

        char PathBuf[MAX_PATH] = {};
        OPENFILENAMEA Ofn   = {};
        Ofn.lStructSize     = sizeof(Ofn);
        Ofn.hwndOwner       = hWnd;
        Ofn.lpstrFilter     = Filter;
        Ofn.lpstrFile       = PathBuf;
        Ofn.nMaxFile        = MAX_PATH;
        Ofn.lpstrInitialDir = InitialDir;
        Ofn.lpstrDefExt     = DefaultExt;
        Ofn.Flags           = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
        bool Ok = GetSaveFileNameA(&Ofn) != 0;

        Detail::HideSystemCursor();
        Detail::ReacquireInput(hWnd);
        return Ok ? PathBuf : "";
    }
}
