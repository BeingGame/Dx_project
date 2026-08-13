#include "AnimEditorUI.h"
#include "SpriteViewerUI.h"

#include "World/Button.h"
#include "World/TextBlock.h"
#include "World/TitleBar.h"
#include "World/Actor.h"
#include "World/Animation2DComponent.h"
#include "World/Input.h"
#include "World/World.h"
#include "Asset/AssetManager.h"
#include "Asset/AnimationManager.h"
#include "Asset/Texture.h"
#include "LogManager.h"
#include "DialogUtil.h"

#include <fstream>
#include <sstream>

CAnimEditorUI::CAnimEditorUI()
{}

CAnimEditorUI::CAnimEditorUI(const CAnimEditorUI& src)
    : CWidgetContainer(src)
{}

CAnimEditorUI::~CAnimEditorUI()
{}

// ── 위젯 헬퍼 ────────────────────────────────────────────────────────────────

std::weak_ptr<CButton> CAnimEditorUI::MakeBtn(const std::string& Name,
    float X, float Y, float W, float H, float R, float G, float B)
{
    auto Btn = CreateWidget<CButton>(Name, 3).lock();
    if (Btn)
    {
        Btn->SetPos(X, Y);
        Btn->SetSize(W, H);
        float HR = min(R + .15f, 1.f), HG = min(G + .15f, 1.f), HB = min(B + .15f, 1.f);
        float CR = min(R + .25f, 1.f), CG = min(G + .25f, 1.f), CB = min(B + .25f, 1.f);
        Btn->SetTint(EWidgetState::Normal,  R,  G,  B,  1.f);
        Btn->SetTint(EWidgetState::Hovered, HR, HG, HB, 1.f);
        Btn->SetTint(EWidgetState::Clicked, CR, CG, CB, 1.f);
        Btn->SetTint(EWidgetState::Release, HR, HG, HB, 1.f);
        Btn->SetTint(EWidgetState::Disable, 0.15f, 0.15f, 0.15f, 0.5f);
    }
    return Btn;
}

std::weak_ptr<CTextBlock> CAnimEditorUI::MakeLbl(const std::string& Name,
    float X, float Y, float W, float H, const wchar_t* Text,
    float FontSize, ETextAlignH AlignH)
{
    auto Lbl = CreateWidget<CTextBlock>(Name, 4).lock();
    if (Lbl)
    {
        Lbl->SetPos(X, Y);
        Lbl->SetSize(W, H);
        Lbl->SetText(Text);
        Lbl->SetFontSize(FontSize);
        Lbl->SetTextColor(FVector4(0.88f, 0.88f, 0.92f, 1.f));
        Lbl->SetAlignH(AlignH);
        Lbl->SetAlignV(ETextAlignV::Middle);
    }
    return Lbl;
}

// ── Prop Row (시퀀스 속성) ────────────────────────────────────────────────────

void CAnimEditorUI::AddPropRow(float& Y, const wchar_t* Label, float Step,
    std::function<float()> Get, std::function<void(float)> Set)
{
    int I = mDynIdx++;
    float W = GetSize().x;

    MakeLbl("APL" + std::to_string(I), 6.f, Y, 72.f, ROW_H, Label, 11.f);

    auto MinBtn = MakeBtn("APM" + std::to_string(I), 80.f, Y + 1.f, 22.f, ROW_H - 2.f, 0.22f, 0.22f, 0.28f);
    MakeLbl("APML" + std::to_string(I), 80.f, Y + 1.f, 22.f, ROW_H - 2.f, TEXT("−"), 12.f, ETextAlignH::Center);

    TCHAR ValBuf[32];
    swprintf_s(ValBuf, 32, L"%.2f", Get ? Get() : 0.f);
    auto ValLbl = CreateWidget<CTextBlock>("APV" + std::to_string(I), 4).lock();
    if (ValLbl)
    {
        ValLbl->SetPos(104.f, Y);
        ValLbl->SetSize(W - 140.f, ROW_H);
        ValLbl->SetText(ValBuf);
        ValLbl->SetFontSize(11.f);
        ValLbl->SetTextColor(FVector4(0.9f, 0.95f, 1.f, 1.f));
        ValLbl->SetAlignH(ETextAlignH::Center);
        ValLbl->SetAlignV(ETextAlignV::Middle);
    }

    auto PlusBtn = MakeBtn("APP" + std::to_string(I), W - 34.f, Y + 1.f, 22.f, ROW_H - 2.f, 0.22f, 0.22f, 0.28f);
    MakeLbl("APPL" + std::to_string(I), W - 34.f, Y + 1.f, 22.f, ROW_H - 2.f, TEXT("+"), 12.f, ETextAlignH::Center);

    mPropBtns.push_back({ MinBtn, PlusBtn, ValLbl, Step, Get, Set });
    Y += ROW_H + 2.f;
}

// ── Frame Prop Row (프레임 속성, mFramePropBtns) ───────────────────────────────

void CAnimEditorUI::AddFramePropRow(float& Y, const wchar_t* Label, float Step,
    std::function<float()> Get, std::function<void(float)> Set)
{
    int I = mDynIdx++;
    float W = GetSize().x;

    MakeLbl("FPL" + std::to_string(I), 6.f, Y, 72.f, ROW_H, Label, 11.f);

    auto MinBtn = MakeBtn("FPM" + std::to_string(I), 80.f, Y + 1.f, 22.f, ROW_H - 2.f, 0.22f, 0.25f, 0.22f);
    MakeLbl("FPML" + std::to_string(I), 80.f, Y + 1.f, 22.f, ROW_H - 2.f, TEXT("−"), 12.f, ETextAlignH::Center);

    TCHAR ValBuf[32];
    swprintf_s(ValBuf, 32, L"%.1f", Get ? Get() : 0.f);
    auto ValLbl = CreateWidget<CTextBlock>("FPV" + std::to_string(I), 4).lock();
    if (ValLbl)
    {
        ValLbl->SetPos(104.f, Y);
        ValLbl->SetSize(W - 140.f, ROW_H);
        ValLbl->SetText(ValBuf);
        ValLbl->SetFontSize(11.f);
        ValLbl->SetTextColor(FVector4(0.85f, 1.f, 0.85f, 1.f));
        ValLbl->SetAlignH(ETextAlignH::Center);
        ValLbl->SetAlignV(ETextAlignV::Middle);
    }

    auto PlusBtn = MakeBtn("FPP" + std::to_string(I), W - 34.f, Y + 1.f, 22.f, ROW_H - 2.f, 0.22f, 0.25f, 0.22f);
    MakeLbl("FPPL" + std::to_string(I), W - 34.f, Y + 1.f, 22.f, ROW_H - 2.f, TEXT("+"), 12.f, ETextAlignH::Center);

    mFramePropBtns.push_back({ MinBtn, PlusBtn, ValLbl, Step, Get, Set });
    Y += ROW_H + 2.f;
}

// ── 토글 행 ──────────────────────────────────────────────────────────────────

void CAnimEditorUI::AddToggleRow(float& Y, const wchar_t* Label,
    std::function<bool()> Get, std::function<void(bool)> Set)
{
    int I = mDynIdx++;
    MakeLbl("ATL" + std::to_string(I), 6.f, Y, 72.f, ROW_H, Label, 11.f);

    bool bOn = Get ? Get() : false;
    auto Btn = MakeBtn("ATB" + std::to_string(I), 80.f, Y + 1.f, 60.f, ROW_H - 2.f,
        bOn ? 0.12f : 0.22f, bOn ? 0.38f : 0.22f, bOn ? 0.12f : 0.28f);

    auto Lbl = CreateWidget<CTextBlock>("ATBL" + std::to_string(I), 5).lock();
    if (Lbl)
    {
        Lbl->SetPos(80.f, Y + 1.f);
        Lbl->SetSize(60.f, ROW_H - 2.f);
        Lbl->SetText(bOn ? TEXT("ON") : TEXT("OFF"));
        Lbl->SetFontSize(11.f);
        Lbl->SetTextColor(FVector4::White);
        Lbl->SetAlignH(ETextAlignH::Center);
        Lbl->SetAlignV(ETextAlignV::Middle);
    }

    mToggleBtns.push_back({ Btn, Lbl, Get, Set });
    Y += ROW_H + 2.f;
}

// ── 초기화 ───────────────────────────────────────────────────────────────────

bool CAnimEditorUI::Init()
{
    CWidgetContainer::Init();

    SetPos(PANEL_X, PANEL_Y);
    SetSize(PANEL_W, PANEL_H);

    auto Bg = CreateWidget<CButton>("AnimEdBg", 0).lock();
    if (Bg)
    {
        Bg->SetPos(0.f, 0.f);
        Bg->SetSize(PANEL_W, PANEL_H);
        Bg->SetTint(EWidgetState::Normal,  0.11f, 0.11f, 0.15f, 0.95f);
        Bg->SetTint(EWidgetState::Hovered, 0.11f, 0.11f, 0.15f, 0.95f);
        Bg->SetTint(EWidgetState::Clicked, 0.11f, 0.11f, 0.15f, 0.95f);
        Bg->SetTint(EWidgetState::Release, 0.11f, 0.11f, 0.15f, 0.95f);
        Bg->SetTint(EWidgetState::Disable, 0.11f, 0.11f, 0.15f, 0.95f);
        mBackground = Bg;
    }

    auto TB = CreateWidget<CTitleBar>("AnimEdTB", 1).lock();
    if (TB)
    {
        TB->SetPos(0.f, 0.f);
        TB->SetSize(PANEL_W, TITLE_H);
        TB->SetTint(0.18f, 0.18f, 0.28f, 1.f);
        TB->SetUpdateWidget(GetThisPtr<CWidget>());
        mTitleBarWidget = TB;
    }

    auto TT = CreateWidget<CTextBlock>("AnimEdTitle", 2).lock();
    if (TT)
    {
        TT->SetPos(0.f, 0.f);
        TT->SetSize(PANEL_W, TITLE_H);
        TT->SetText(TEXT("Animation Editor"));
        TT->SetFontSize(14.f);
        TT->SetTextColor(FVector4::White);
        TT->SetAlignH(ETextAlignH::Center);
        TT->SetAlignV(ETextAlignV::Middle);
        mTitleText = TT;
    }

    auto MakeHandle = [&](const std::string& N, float X, float Y2) -> std::weak_ptr<CButton>
    {
        auto H = CreateWidget<CButton>(N, 10).lock();
        if (H)
        {
            H->SetPos(X, Y2);
            H->SetSize(HANDLE_SZ, HANDLE_SZ);
            H->SetTint(EWidgetState::Normal,  0.40f, 0.40f, 0.50f, 0.85f);
            H->SetTint(EWidgetState::Hovered, 0.70f, 0.90f, 1.00f, 1.f);
            H->SetTint(EWidgetState::Clicked, 1.00f, 1.00f, 1.00f, 1.f);
            H->SetTint(EWidgetState::Release, 1.00f, 1.00f, 1.00f, 1.f);
            H->SetTint(EWidgetState::Disable, 0.25f, 0.25f, 0.25f, 0.5f);
        }
        return H;
    };
    mHandleTL = MakeHandle("AEH_TL", 0.f,               0.f);
    mHandleTR = MakeHandle("AEH_TR", PANEL_W - HANDLE_SZ, 0.f);
    mHandleBL = MakeHandle("AEH_BL", 0.f,               PANEL_H - HANDLE_SZ);
    mHandleBR = MakeHandle("AEH_BR", PANEL_W - HANDLE_SZ, PANEL_H - HANDLE_SZ);

    mStaticChildCount = (int)mChildList.size();
    mStaticWidgets    = mChildList;   // 정적 위젯 목록을 강한 참조로 보존
    return true;
}

// ── 핵심 로직 ─────────────────────────────────────────────────────────────────

void CAnimEditorUI::SyncFrames(int ci, int si)
{
    if (ci < 0 || ci >= (int)mComps.size()) return;
    if (si < 0 || si >= (int)mComps[ci].Seqs.size()) return;

    auto& SD = mComps[ci].Seqs[si];
    SD.Frames.clear();

    auto AnimMgr = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D);
    if (!AnimMgr) return;

    auto Anim = AnimMgr->FindAnimation(SD.Name).lock();
    if (!Anim) return;

    SD.TextureType = Anim->GetType();
    if (auto Tex = Anim->GetTexture().lock())
        SD.TextureName = Tex->GetName();

    int Count = Anim->GetFrameCount();
    for (int i = 0; i < Count; ++i)
    {
        const FTextureFrame& TF = Anim->GetFrame(i);
        FFrameData FD;
        FD.Start  = TF.Start;
        FD.Size   = TF.Size;
        FD.Offset = TF.Offset;
        SD.Frames.push_back(FD);
    }
    SD.SelectedFrame = SD.Frames.empty() ? -1 : 0;
}

void CAnimEditorUI::ApplyFrames(int ci, int si)
{
    if (ci < 0 || ci >= (int)mComps.size()) return;
    if (si < 0 || si >= (int)mComps[ci].Seqs.size()) return;

    auto& SD = mComps[ci].Seqs[si];

    auto AnimMgr = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D);
    if (!AnimMgr) return;

    auto Anim = AnimMgr->FindAnimation(SD.Name).lock();
    if (!Anim) return;

    Anim->ClearFrame();
    for (auto& FD : SD.Frames)
        Anim->AddFrame(FD.Start, FD.Size, FD.Offset);
}

void CAnimEditorUI::SetTarget(std::weak_ptr<CActor> Actor)
{
    mTarget = Actor;
    mComps.clear();
    mActiveComp      = 0;
    mShowRegistry    = false;
    mShowFrameEditor = false;

    auto A = Actor.lock();
    if (A)
    {
        for (auto& Comp : A->GetActorCompList())
        {
            auto AnimComp = std::dynamic_pointer_cast<CAnimation2DComponent>(Comp);
            if (!AnimComp) continue;

            FCompData D;
            D.Comp     = AnimComp;
            D.CompName = Comp->GetName();
            mComps.push_back(D);
        }
    }
    Rebuild();
}

void CAnimEditorUI::AddSeq(int CompIdx, const std::string& AnimName)
{
    if (CompIdx < 0 || CompIdx >= (int)mComps.size()) return;
    auto& CD = mComps[CompIdx];

    for (auto& S : CD.Seqs) if (S.Name == AnimName) return;

    FSeqData SD;
    SD.Name = AnimName;

    if (auto C = CD.Comp.lock())
        C->AddAnimation(AnimName, SD.PlayTime, SD.PlayRate, SD.Loop, SD.Reverse, SD.Symmetry);

    CD.Seqs.push_back(SD);
    CD.Selected = (int)CD.Seqs.size() - 1;

    // 기존 프레임 데이터 동기화
    SyncFrames(CompIdx, CD.Selected);

    mShowRegistry = false;
    Rebuild();
}

void CAnimEditorUI::PlaySeq(int CompIdx, int SeqIdx)
{
    if (CompIdx < 0 || CompIdx >= (int)mComps.size()) return;
    auto& CD = mComps[CompIdx];
    if (SeqIdx < 0 || SeqIdx >= (int)CD.Seqs.size()) return;

    if (auto C = CD.Comp.lock())
        C->ChangeAnimation(CD.Seqs[SeqIdx].Name);

    CD.Selected = SeqIdx;
    Rebuild();
}

void CAnimEditorUI::CreateNewAnim()
{
    std::string Dir = DialogUtil::GetExeDir() + "Asset\\Anim\\";
    CreateDirectoryA((DialogUtil::GetExeDir() + "Asset\\").c_str(), nullptr);
    CreateDirectoryA(Dir.c_str(), nullptr);

    // SaveFile 다이얼로그: 파일명 입력 = 애니메이션 이름
    std::string Path = DialogUtil::SaveFile(
        "Anim Files\0*.anim2d\0All Files\0*.*\0", Dir.c_str(), "anim2d");
    if (Path.empty()) return;

    std::string AnimName = DialogUtil::ExtractBaseName(Path);
    if (AnimName.empty()) return;

    auto AnimMgr = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D);
    if (!AnimMgr) return;

    if (!AnimMgr->FindAnimation(AnimName).lock())
        AnimMgr->CreateAnimation(AnimName);

    CAnimRegistry::Register(AnimName);

    // AnimComp가 있으면 시퀀스도 추가, 없으면 에셋만 등록
    if (!mComps.empty())
    {
        auto& CD = mComps[mActiveComp];
        bool AlreadyHave = false;
        for (auto& S : CD.Seqs) if (S.Name == AnimName) { AlreadyHave = true; break; }

        if (!AlreadyHave)
        {
            if (auto C = CD.Comp.lock())
                C->AddAnimation(AnimName, 1.f, 1.f, true, false, false);

            FSeqData SD;
            SD.Name = AnimName;
            CD.Seqs.push_back(SD);
            CD.Selected = (int)CD.Seqs.size() - 1;
        }
    }

    LOG_DEBUG("[AnimEditor] Created: %s", AnimName.c_str());
    Rebuild();
}

void CAnimEditorUI::SetAnimTexture(int ci, int si)
{
    if (ci < 0 || ci >= (int)mComps.size()) return;
    if (si < 0 || si >= (int)mComps[ci].Seqs.size()) return;

    auto& SD = mComps[ci].Seqs[si];

    std::string Path = DialogUtil::OpenFile(
        "Image Files\0*.png;*.jpg;*.bmp;*.tga\0All Files\0*.*\0", nullptr);
    if (Path.empty()) return;

    std::string ExeDir = DialogUtil::GetExeDir();
    std::string TexName = DialogUtil::ExtractBaseName(Path);
    SD.TextureName = TexName;

    // exe 기준 상대경로 저장 (Save/Load 재사용 위해)
    if (Path.size() > ExeDir.size() &&
        _strnicmp(Path.c_str(), ExeDir.c_str(), ExeDir.size()) == 0)
        SD.TextureRelPath = Path.substr(ExeDir.size());
    else
        SD.TextureRelPath = Path; // 외부 경로면 절대경로 그대로

    auto AnimMgr = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D);
    if (!AnimMgr) return;

    auto Anim = AnimMgr->FindAnimation(SD.Name).lock();
    if (!Anim) return;

    std::wstring WPath = DialogUtil::ToWide(Path);
    Anim->SetTextureFullPath(TexName, WPath.c_str());

    LOG_DEBUG("[AnimEditor] Texture set: %s -> %s", SD.Name.c_str(), TexName.c_str());
    Rebuild();
}

void CAnimEditorUI::SaveAnim(int ci, int si)
{
    if (ci < 0 || ci >= (int)mComps.size()) return;
    if (si < 0 || si >= (int)mComps[ci].Seqs.size()) return;

    auto& SD = mComps[ci].Seqs[si];

    std::string Dir = DialogUtil::GetExeDir() + "Asset\\Anim\\";
    CreateDirectoryA((DialogUtil::GetExeDir() + "Asset\\").c_str(), nullptr);
    CreateDirectoryA(Dir.c_str(), nullptr);

    std::string Path = DialogUtil::SaveFile(
        "Anim Files\0*.anim2d\0All Files\0*.*\0", Dir.c_str(), "anim2d");
    if (Path.empty()) return;

    std::ofstream File(Path);
    if (!File) return;

    std::string TypeStr = (SD.TextureType == EAnimation2DTextureType::Frame) ? "Frame" : "SpriteSheet";

    File << "ANIM2D 1\n";
    File << "AnimName "      << SD.Name            << "\n";
    File << "TextureType "   << TypeStr             << "\n";
    File << "TextureName "   << SD.TextureName      << "\n";
    File << "TextureRelPath "<< SD.TextureRelPath   << "\n";
    File << "PlayTime "      << SD.PlayTime         << "\n";
    File << "PlayRate "   << SD.PlayRate    << "\n";
    File << "Loop "       << (SD.Loop     ? 1 : 0) << "\n";
    File << "Reverse "    << (SD.Reverse  ? 1 : 0) << "\n";
    File << "Symmetry "   << (SD.Symmetry ? 1 : 0) << "\n";
    File << "Frames "     << SD.Frames.size() << "\n";
    for (auto& FD : SD.Frames)
        File << FD.Start.x  << " " << FD.Start.y  << " "
             << FD.Size.x   << " " << FD.Size.y   << " "
             << FD.Offset.x << " " << FD.Offset.y << "\n";

    LOG_DEBUG("[AnimEditor] Saved: %s", Path.c_str());
}

// ── 공통 파싱: 파일 하나를 읽어 CAnimation2D 생성 + CAnimRegistry 등록 ──────

void CAnimEditorUI::LoadAnimFromFile(const std::string& Path)
{
    std::ifstream File(Path);
    if (!File) return;

    std::string AnimName, TextureName, TextureRelPath;
    EAnimation2DTextureType TextureType = EAnimation2DTextureType::SpriteSheet;
    float PlayTime = 1.f, PlayRate = 1.f;
    bool  Loop = true, Reverse = false, Symmetry = false;
    std::vector<FFrameData> Frames;

    std::string Line;
    while (std::getline(File, Line))
    {
        if (Line.empty() || Line[0] == '#') continue;
        std::istringstream SS(Line);
        std::string Key; SS >> Key;

        if      (Key == "ANIM2D")        { /* 버전 */ }
        else if (Key == "AnimName")       SS >> AnimName;
        else if (Key == "TextureType")    { std::string T; SS >> T; TextureType = (T == "Frame") ? EAnimation2DTextureType::Frame : EAnimation2DTextureType::SpriteSheet; }
        else if (Key == "TextureName")    SS >> TextureName;
        else if (Key == "TextureRelPath") std::getline(SS >> std::ws, TextureRelPath);
        else if (Key == "PlayTime")       SS >> PlayTime;
        else if (Key == "PlayRate")       SS >> PlayRate;
        else if (Key == "Loop")           { int V; SS >> V; Loop    = (V != 0); }
        else if (Key == "Reverse")        { int V; SS >> V; Reverse = (V != 0); }
        else if (Key == "Symmetry")       { int V; SS >> V; Symmetry= (V != 0); }
        else if (Key == "Frames")
        {
            int Count = 0; SS >> Count;
            for (int i = 0; i < Count; ++i)
            {
                std::string FLine;
                while (std::getline(File, FLine) && FLine.empty()) {}
                FFrameData FD;
                std::istringstream FSS(FLine);
                FSS >> FD.Start.x >> FD.Start.y >> FD.Size.x >> FD.Size.y >> FD.Offset.x >> FD.Offset.y;
                Frames.push_back(FD);
            }
        }
    }

    if (AnimName.empty()) return;

    auto AnimMgr = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D);
    if (!AnimMgr) return;

    if (!AnimMgr->FindAnimation(AnimName).lock())
        AnimMgr->CreateAnimation(AnimName);

    auto Anim = AnimMgr->FindAnimation(AnimName).lock();
    if (!Anim) return;

    Anim->SetAnimationTextureType(TextureType);

    // TextureRelPath 우선, 없으면 TextureName 폴백
    if (!TextureRelPath.empty())
    {
        std::string FullPath = DialogUtil::GetExeDir() + TextureRelPath;
        std::wstring WPath = DialogUtil::ToWide(FullPath);
        Anim->SetTextureFullPath(TextureName, WPath.c_str());
    }
    else if (!TextureName.empty())
    {
        Anim->SetTexture(TextureName);
    }

    Anim->ClearFrame();
    for (auto& FD : Frames)
        Anim->AddFrame(FD.Start, FD.Size, FD.Offset);

    CAnimRegistry::Register(AnimName);
}

// ── 시작 시 전체 자동 로드 ────────────────────────────────────────────────────

void CAnimEditorUI::LoadAllAnims()
{
    std::string Dir = DialogUtil::GetExeDir() + "Asset\\Anim\\";

    WIN32_FIND_DATAA FindData;
    HANDLE hFind = FindFirstFileA((Dir + "*.anim2d").c_str(), &FindData);
    if (hFind == INVALID_HANDLE_VALUE) return;

    int Count = 0;
    do
    {
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        LoadAnimFromFile(Dir + FindData.cFileName);
        ++Count;
    } while (FindNextFileA(hFind, &FindData));

    FindClose(hFind);
    LOG_DEBUG("[AnimEditor] Auto-loaded %d anim(s) from %s", Count, Dir.c_str());
}

void CAnimEditorUI::LoadAnim()
{
    std::string Dir = DialogUtil::GetExeDir() + "Asset\\Anim\\";
    std::string Path = DialogUtil::OpenFile(
        "Anim Files\0*.anim2d\0All Files\0*.*\0", Dir.c_str());
    if (Path.empty()) return;

    // CAnimation2D 생성 + 레지스트리 등록
    LoadAnimFromFile(Path);

    // 파일에서 AnimName 다시 읽어서 컴포넌트에 붙이기
    std::ifstream File(Path);
    std::string AnimName, Line;
    while (std::getline(File, Line))
    {
        std::istringstream SS(Line);
        std::string Key; SS >> Key;
        if (Key == "AnimName") { SS >> AnimName; break; }
    }
    if (AnimName.empty()) return;

    if (mActiveComp < (int)mComps.size())
    {
        auto& CD = mComps[mActiveComp];
        bool AlreadyHave = false;
        for (auto& S : CD.Seqs) if (S.Name == AnimName) { AlreadyHave = true; break; }

        if (!AlreadyHave)
        {
            if (auto C = CD.Comp.lock())
                C->AddAnimation(AnimName, 1.f, 1.f, true, false, false);

            FSeqData SD;
            SD.Name = AnimName;
            CD.Seqs.push_back(SD);
            CD.Selected = (int)CD.Seqs.size() - 1;
            SyncFrames(mActiveComp, CD.Selected);
        }
    }

    LOG_DEBUG("[AnimEditor] Loaded: %s", AnimName.c_str());
    Rebuild();
}

void CAnimEditorUI::UpdateHandles(float nw, float nh)
{
    if (auto B = mBackground.lock())     B->SetSize(nw, nh);
    if (auto T = mTitleBarWidget.lock()) T->SetSize(nw, TITLE_H);
    if (auto T = mTitleText.lock())      T->SetSize(nw, TITLE_H);
    if (auto H = mHandleTL.lock()) H->SetPos(0.f,            0.f);
    if (auto H = mHandleTR.lock()) H->SetPos(nw - HANDLE_SZ, 0.f);
    if (auto H = mHandleBL.lock()) H->SetPos(0.f,            nh - HANDLE_SZ);
    if (auto H = mHandleBR.lock()) H->SetPos(nw - HANDLE_SZ, nh - HANDLE_SZ);
}

// ── 위젯 재구성 ──────────────────────────────────────────────────────────────

void CAnimEditorUI::Rebuild()
{
    // 정적 위젯 목록으로 mChildList를 완전히 초기화 — 이전 Rebuild의 동적 위젯을 확실히 제거
    if (!mStaticWidgets.empty())
        mChildList = mStaticWidgets;
    else if ((int)mChildList.size() > mStaticChildCount)
        mChildList.resize(mStaticChildCount);

    mTabBtns.clear(); mSeqBtns.clear();
    mPropBtns.clear(); mToggleBtns.clear();
    mRegBtns.clear(); mFramePropBtns.clear();
    mAddToggleBtn = {}; mFrameCountText = {};
    mToggleFrameBtn = {}; mFramePrevBtn = {}; mFrameNextBtn = {};
    mFrameIdxLbl = {}; mAddFrameBtn = {}; mDelFrameBtn = {}; mClearFramesBtn = {};
    mSetTextureBtn = {}; mTypeToggleBtn = {};
    mSaveAnimBtn = {}; mLoadAnimBtn = {}; mNewAnimBtn = {};
    mOpenViewerBtn = {};

    float W = GetSize().x;
    float H = GetSize().y;
    float Y = TITLE_H + 4.f;

    // ── Actor 이름 ────────────────────────────────────────────────────────────
    {
        std::string AN = "(none)";
        if (auto A = mTarget.lock()) AN = A->GetName();
        std::wstring WN(AN.begin(), AN.end());
        TCHAR Buf[128]; wsprintf(Buf, TEXT("Actor: %s"), WN.c_str());
        MakeLbl("AEActN" + std::to_string(mDynIdx++), 6.f, Y, W - 8.f, ROW_H, Buf, 11.f);
        Y += ROW_H + 2.f;
    }

    // ── 새 애니메이션 만들기 버튼 (항상 표시) ─────────────────────────────────
    {
        auto NewBtn = MakeBtn("AENewAnim" + std::to_string(mDynIdx), 4.f, Y, W - 8.f, ROW_H,
            0.28f, 0.22f, 0.10f);
        MakeLbl("AENewAnimL" + std::to_string(mDynIdx++), 4.f, Y, W - 8.f, ROW_H,
            TEXT("★ 새 애니메이션 만들기"), 11.f, ETextAlignH::Center);
        mNewAnimBtn = NewBtn;
        Y += ROW_H + 4.f;
    }

    if (mComps.empty())
    {
        MakeLbl("AENoComp" + std::to_string(mDynIdx++), 6.f, Y, W - 8.f, ROW_H,
            TEXT("▲ AnimComp 없음 — 에셋만 생성됨"), 10.f);
        return;
    }

    // ── AnimComp 탭 ───────────────────────────────────────────────────────────
    {
        float TW = min((W - 8.f) / (float)mComps.size(), 90.f);
        float TX = 4.f;
        for (int i = 0; i < (int)mComps.size(); ++i)
        {
            std::wstring WN(mComps[i].CompName.begin(), mComps[i].CompName.end());
            bool bActive = (i == mActiveComp);
            auto Btn = MakeBtn("AETab" + std::to_string(mDynIdx), TX, Y, TW - 2.f, 22.f,
                bActive ? 0.22f : 0.16f,
                bActive ? 0.38f : 0.22f,
                bActive ? 0.55f : 0.28f);
            MakeLbl("AETabL" + std::to_string(mDynIdx++), TX, Y, TW - 2.f, 22.f,
                WN.c_str(), 9.f, ETextAlignH::Center);
            mTabBtns.push_back({ Btn, i });
            TX += TW;
        }
        Y += 26.f;
    }

    if (mActiveComp >= (int)mComps.size()) mActiveComp = 0;
    auto& CD = mComps[mActiveComp];

    // ── 시퀀스 목록 ───────────────────────────────────────────────────────────
    MakeLbl("AESeqHdr" + std::to_string(mDynIdx++), 4.f, Y, W - 8.f, 14.f,
        TEXT("── Sequences ──"), 10.f);
    Y += 16.f;

    float MaxSeqY = H - 300.f;
    for (int i = 0; i < (int)CD.Seqs.size(); ++i)
    {
        if (Y + ROW_H > MaxSeqY) break;
        std::wstring WN(CD.Seqs[i].Name.begin(), CD.Seqs[i].Name.end());
        bool bSel = (i == CD.Selected);
        auto Btn = MakeBtn("AESeq" + std::to_string(mDynIdx), 4.f, Y, W - 8.f, ROW_H,
            bSel ? 0.18f : 0.14f,
            bSel ? 0.38f : 0.20f,
            bSel ? 0.65f : 0.26f);
        MakeLbl("AESeqL" + std::to_string(mDynIdx++), 8.f, Y, W - 12.f, ROW_H,
            WN.c_str(), 11.f);
        mSeqBtns.push_back({ Btn, i });
        Y += ROW_H + 2.f;
    }

    // ── 선택된 시퀀스 속성 ────────────────────────────────────────────────────
    if (CD.Selected >= 0 && CD.Selected < (int)CD.Seqs.size())
    {
        Y += 4.f;
        MakeLbl("AEPropHdr" + std::to_string(mDynIdx++), 4.f, Y, W - 8.f, 14.f,
            TEXT("── Properties ──"), 10.f);
        Y += 16.f;

        int ci = mActiveComp, si = CD.Selected;
        std::weak_ptr<CAnimation2DComponent> WeakComp = CD.Comp;

        AddPropRow(Y, TEXT("PlayTime"), 0.1f,
            [this, ci, si]() -> float {
                if (ci < (int)mComps.size() && si < (int)mComps[ci].Seqs.size())
                    return mComps[ci].Seqs[si].PlayTime;
                return 1.f;
            },
            [this, ci, si, WeakComp](float V) {
                if (ci >= (int)mComps.size() || si >= (int)mComps[ci].Seqs.size()) return;
                mComps[ci].Seqs[si].PlayTime = max(0.05f, V);
                if (auto C = WeakComp.lock())
                    C->SetPlayTime(mComps[ci].Seqs[si].Name, mComps[ci].Seqs[si].PlayTime);
            });

        AddPropRow(Y, TEXT("PlayRate"), 0.1f,
            [this, ci, si]() -> float {
                if (ci < (int)mComps.size() && si < (int)mComps[ci].Seqs.size())
                    return mComps[ci].Seqs[si].PlayRate;
                return 1.f;
            },
            [this, ci, si, WeakComp](float V) {
                if (ci >= (int)mComps.size() || si >= (int)mComps[ci].Seqs.size()) return;
                mComps[ci].Seqs[si].PlayRate = max(0.05f, V);
                if (auto C = WeakComp.lock())
                    C->SetPlayRate(mComps[ci].Seqs[si].Name, mComps[ci].Seqs[si].PlayRate);
            });

        AddToggleRow(Y, TEXT("Loop"),
            [this, ci, si]() -> bool { return ci < (int)mComps.size() && si < (int)mComps[ci].Seqs.size() ? mComps[ci].Seqs[si].Loop : true; },
            [this, ci, si, WeakComp](bool V) {
                if (ci >= (int)mComps.size() || si >= (int)mComps[ci].Seqs.size()) return;
                mComps[ci].Seqs[si].Loop = V;
                if (auto C = WeakComp.lock()) C->SetLoop(mComps[ci].Seqs[si].Name, V);
            });

        AddToggleRow(Y, TEXT("Reverse"),
            [this, ci, si]() -> bool { return ci < (int)mComps.size() && si < (int)mComps[ci].Seqs.size() ? mComps[ci].Seqs[si].Reverse : false; },
            [this, ci, si, WeakComp](bool V) {
                if (ci >= (int)mComps.size() || si >= (int)mComps[ci].Seqs.size()) return;
                mComps[ci].Seqs[si].Reverse = V;
                if (auto C = WeakComp.lock()) C->SetReverse(mComps[ci].Seqs[si].Name, V);
            });

        AddToggleRow(Y, TEXT("Symmetry"),
            [this, ci, si]() -> bool { return ci < (int)mComps.size() && si < (int)mComps[ci].Seqs.size() ? mComps[ci].Seqs[si].Symmetry : false; },
            [this, ci, si, WeakComp](bool V) {
                if (ci >= (int)mComps.size() || si >= (int)mComps[ci].Seqs.size()) return;
                mComps[ci].Seqs[si].Symmetry = V;
                if (auto C = WeakComp.lock()) C->SetSymmetry(mComps[ci].Seqs[si].Name, V);
            });

        Y += 4.f;

        // ── 텍스처 설정 ───────────────────────────────────────────────────────
        if (Y + ROW_H < H - 10.f)
        {
            auto& SD2 = CD.Seqs[CD.Selected];
            std::wstring TexW = SD2.TextureName.empty()
                ? L"(없음)" : std::wstring(SD2.TextureName.begin(), SD2.TextureName.end());
            TCHAR TexBuf[128];
            swprintf_s(TexBuf, 128, L"Tex: %s", TexW.c_str());
            float LW = W * 0.55f - 4.f;
            MakeLbl("AETexLbl" + std::to_string(mDynIdx++), 6.f, Y, LW, ROW_H, TexBuf, 9.f);
            float BX = 6.f + LW;
            auto TexBtn = MakeBtn("AESetTex" + std::to_string(mDynIdx), BX, Y + 1.f, W - BX - 6.f, ROW_H - 2.f, 0.18f, 0.20f, 0.38f);
            MakeLbl("AESetTexL" + std::to_string(mDynIdx++), BX, Y + 1.f, W - BX - 6.f, ROW_H - 2.f, TEXT("Set Texture"), 9.f, ETextAlignH::Center);
            mSetTextureBtn = TexBtn;
            Y += ROW_H + 2.f;
        }

        // ── TextureType 토글 (Frame / SpriteSheet) ────────────────────────────
        if (Y + ROW_H < H - 10.f)
        {
            auto& SD2 = CD.Seqs[CD.Selected];
            bool bFrame = (SD2.TextureType == EAnimation2DTextureType::Frame);
            MakeLbl("AETypLbl" + std::to_string(mDynIdx++), 6.f, Y, 52.f, ROW_H, TEXT("Type:"), 11.f);
            auto TypeBtn = MakeBtn("AETypTog" + std::to_string(mDynIdx), 60.f, Y + 1.f, W - 68.f, ROW_H - 2.f,
                bFrame ? 0.30f : 0.18f, 0.20f, bFrame ? 0.18f : 0.30f);
            MakeLbl("AETypTogL" + std::to_string(mDynIdx++), 60.f, Y + 1.f, W - 68.f, ROW_H - 2.f,
                bFrame ? TEXT("Frame") : TEXT("SpriteSheet"), 11.f, ETextAlignH::Center);
            mTypeToggleBtn = TypeBtn;
            Y += ROW_H + 4.f;
        }

        // ── Sprite Viewer 열기 ────────────────────────────────────────────────
        if (Y + ROW_H < H - 10.f)
        {
            auto ViewBtn = MakeBtn("AEOpenView" + std::to_string(mDynIdx), 4.f, Y, W - 8.f, ROW_H,
                0.14f, 0.28f, 0.20f);
            MakeLbl("AEOpenViewL" + std::to_string(mDynIdx++), 4.f, Y, W - 8.f, ROW_H,
                TEXT("🎨 Open Sprite Viewer"), 11.f, ETextAlignH::Center);
            mOpenViewerBtn = ViewBtn;
            Y += ROW_H + 4.f;
        }

        // ── 프레임 에디터 토글 ────────────────────────────────────────────────
        if (Y + ROW_H < H - 10.f)
        {
            auto& SD = CD.Seqs[CD.Selected];
            TCHAR FrTogBuf[64];
            swprintf_s(FrTogBuf, 64, L"%s Frames (%d)",
                mShowFrameEditor ? L"▼" : L"▶", (int)SD.Frames.size());

            auto TogBtn = MakeBtn("AEFrTog" + std::to_string(mDynIdx), 4.f, Y, W - 8.f, ROW_H,
                0.16f, 0.26f, 0.35f);
            MakeLbl("AEFrTogL" + std::to_string(mDynIdx++), 4.f, Y, W - 8.f, ROW_H,
                FrTogBuf, 11.f, ETextAlignH::Center);
            mToggleFrameBtn = TogBtn;
            Y += ROW_H + 4.f;
        }

        // ── 프레임 에디터 본문 ────────────────────────────────────────────────
        if (mShowFrameEditor && Y + ROW_H < H - 10.f)
        {
            auto& SD = CD.Seqs[CD.Selected];
            int FrameCount = (int)SD.Frames.size();
            int SF = SD.SelectedFrame;

            // 프레임 인덱스 내비게이션 [< 프레임 X/N >]
            {
                auto PrevBtn = MakeBtn("AEFrPrev" + std::to_string(mDynIdx), 4.f, Y, 24.f, ROW_H, 0.20f, 0.20f, 0.28f);
                MakeLbl("AEFrPrevL" + std::to_string(mDynIdx++), 4.f, Y, 24.f, ROW_H, TEXT("<"), 12.f, ETextAlignH::Center);
                mFramePrevBtn = PrevBtn;

                TCHAR FrBuf[64];
                if (FrameCount > 0)
                    swprintf_s(FrBuf, 64, L"Frame %d / %d", SF + 1, FrameCount);
                else
                    wcscpy_s(FrBuf, 64, L"(no frames)");
                mFrameIdxLbl = MakeLbl("AEFrIdx" + std::to_string(mDynIdx++),
                    30.f, Y, W - 60.f, ROW_H, FrBuf, 10.f, ETextAlignH::Center);

                auto NextBtn = MakeBtn("AEFrNext" + std::to_string(mDynIdx), W - 28.f, Y, 24.f, ROW_H, 0.20f, 0.20f, 0.28f);
                MakeLbl("AEFrNextL" + std::to_string(mDynIdx++), W - 28.f, Y, 24.f, ROW_H, TEXT(">"), 12.f, ETextAlignH::Center);
                mFrameNextBtn = NextBtn;
                Y += ROW_H + 2.f;
            }

            // 선택된 프레임 속성
            if (SF >= 0 && SF < FrameCount && Y + 6 * (ROW_H + 2.f) < H)
            {
                int ci2 = mActiveComp, si2 = CD.Selected, fi = SF;

                AddFramePropRow(Y, TEXT("Start.X"), 1.f,
                    [this, ci2, si2, fi]() -> float {
                        if (ci2<(int)mComps.size()&&si2<(int)mComps[ci2].Seqs.size()&&fi<(int)mComps[ci2].Seqs[si2].Frames.size())
                            return mComps[ci2].Seqs[si2].Frames[fi].Start.x;
                        return 0.f;
                    },
                    [this, ci2, si2, fi](float V) {
                        if (ci2<(int)mComps.size()&&si2<(int)mComps[ci2].Seqs.size()&&fi<(int)mComps[ci2].Seqs[si2].Frames.size())
                            mComps[ci2].Seqs[si2].Frames[fi].Start.x = V;
                    });

                AddFramePropRow(Y, TEXT("Start.Y"), 1.f,
                    [this, ci2, si2, fi]() -> float {
                        if (ci2<(int)mComps.size()&&si2<(int)mComps[ci2].Seqs.size()&&fi<(int)mComps[ci2].Seqs[si2].Frames.size())
                            return mComps[ci2].Seqs[si2].Frames[fi].Start.y;
                        return 0.f;
                    },
                    [this, ci2, si2, fi](float V) {
                        if (ci2<(int)mComps.size()&&si2<(int)mComps[ci2].Seqs.size()&&fi<(int)mComps[ci2].Seqs[si2].Frames.size())
                            mComps[ci2].Seqs[si2].Frames[fi].Start.y = V;
                    });

                AddFramePropRow(Y, TEXT("Size.W"), 1.f,
                    [this, ci2, si2, fi]() -> float {
                        if (ci2<(int)mComps.size()&&si2<(int)mComps[ci2].Seqs.size()&&fi<(int)mComps[ci2].Seqs[si2].Frames.size())
                            return mComps[ci2].Seqs[si2].Frames[fi].Size.x;
                        return 32.f;
                    },
                    [this, ci2, si2, fi](float V) {
                        if (ci2<(int)mComps.size()&&si2<(int)mComps[ci2].Seqs.size()&&fi<(int)mComps[ci2].Seqs[si2].Frames.size())
                            mComps[ci2].Seqs[si2].Frames[fi].Size.x = max(1.f, V);
                    });

                AddFramePropRow(Y, TEXT("Size.H"), 1.f,
                    [this, ci2, si2, fi]() -> float {
                        if (ci2<(int)mComps.size()&&si2<(int)mComps[ci2].Seqs.size()&&fi<(int)mComps[ci2].Seqs[si2].Frames.size())
                            return mComps[ci2].Seqs[si2].Frames[fi].Size.y;
                        return 32.f;
                    },
                    [this, ci2, si2, fi](float V) {
                        if (ci2<(int)mComps.size()&&si2<(int)mComps[ci2].Seqs.size()&&fi<(int)mComps[ci2].Seqs[si2].Frames.size())
                            mComps[ci2].Seqs[si2].Frames[fi].Size.y = max(1.f, V);
                    });

                AddFramePropRow(Y, TEXT("Offset.X"), 0.5f,
                    [this, ci2, si2, fi]() -> float {
                        if (ci2<(int)mComps.size()&&si2<(int)mComps[ci2].Seqs.size()&&fi<(int)mComps[ci2].Seqs[si2].Frames.size())
                            return mComps[ci2].Seqs[si2].Frames[fi].Offset.x;
                        return 0.f;
                    },
                    [this, ci2, si2, fi](float V) {
                        if (ci2<(int)mComps.size()&&si2<(int)mComps[ci2].Seqs.size()&&fi<(int)mComps[ci2].Seqs[si2].Frames.size())
                            mComps[ci2].Seqs[si2].Frames[fi].Offset.x = V;
                    });

                AddFramePropRow(Y, TEXT("Offset.Y"), 0.5f,
                    [this, ci2, si2, fi]() -> float {
                        if (ci2<(int)mComps.size()&&si2<(int)mComps[ci2].Seqs.size()&&fi<(int)mComps[ci2].Seqs[si2].Frames.size())
                            return mComps[ci2].Seqs[si2].Frames[fi].Offset.y;
                        return 0.f;
                    },
                    [this, ci2, si2, fi](float V) {
                        if (ci2<(int)mComps.size()&&si2<(int)mComps[ci2].Seqs.size()&&fi<(int)mComps[ci2].Seqs[si2].Frames.size())
                            mComps[ci2].Seqs[si2].Frames[fi].Offset.y = V;
                    });
            }

            // 추가 / 삭제 / 전체 삭제 버튼 (3열)
            if (Y + ROW_H < H - 10.f)
            {
                float BW = (W - 16.f) / 3.f;

                auto AddFBtn = MakeBtn("AEAddFr" + std::to_string(mDynIdx), 4.f, Y, BW, ROW_H, 0.14f, 0.30f, 0.14f);
                MakeLbl("AEAddFrL" + std::to_string(mDynIdx++), 4.f, Y, BW, ROW_H, TEXT("+ Frame"), 10.f, ETextAlignH::Center);
                mAddFrameBtn = AddFBtn;

                float DX = 4.f + BW + 4.f;
                bool bCanDel = !SD.Frames.empty() && SF >= 0 && SF < FrameCount;
                auto DelFBtn = MakeBtn("AEDelFr" + std::to_string(mDynIdx), DX, Y, BW, ROW_H, bCanDel ? 0.35f : 0.20f, 0.14f, 0.14f);
                MakeLbl("AEDelFrL" + std::to_string(mDynIdx++), DX, Y, BW, ROW_H, TEXT("- Del"), 10.f, ETextAlignH::Center);
                mDelFrameBtn = DelFBtn;

                float CX = DX + BW + 4.f;
                auto ClrBtn = MakeBtn("AEClrFr" + std::to_string(mDynIdx), CX, Y, BW, ROW_H, 0.25f, 0.10f, 0.10f);
                MakeLbl("AEClrFrL" + std::to_string(mDynIdx++), CX, Y, BW, ROW_H, TEXT("Clear"), 10.f, ETextAlignH::Center);
                mClearFramesBtn = ClrBtn;
                Y += ROW_H + 4.f;
            }
        }

        // ── 저장 / 불러오기 ──────────────────────────────────────────────────
        if (Y + ROW_H < H - 4.f)
        {
            float HW = (W - 12.f) / 2.f;
            auto SaveBtn = MakeBtn("AESave" + std::to_string(mDynIdx), 4.f, Y, HW, ROW_H, 0.16f, 0.30f, 0.16f);
            MakeLbl("AESaveL" + std::to_string(mDynIdx++), 4.f, Y, HW, ROW_H, TEXT("Save Anim"), 10.f, ETextAlignH::Center);
            mSaveAnimBtn = SaveBtn;

            float LX = 4.f + HW + 4.f;
            auto LoadBtn = MakeBtn("AELoad" + std::to_string(mDynIdx), LX, Y, HW, ROW_H, 0.16f, 0.22f, 0.35f);
            MakeLbl("AELoadL" + std::to_string(mDynIdx++), LX, Y, HW, ROW_H, TEXT("Load Anim"), 10.f, ETextAlignH::Center);
            mLoadAnimBtn = LoadBtn;
            Y += ROW_H + 4.f;
        }
    }

    // ── Add Animation 토글 버튼 ───────────────────────────────────────────────
    if (Y + ROW_H < H - 10.f)
    {
        auto AddBtn = MakeBtn("AEAddBtn" + std::to_string(mDynIdx), 4.f, Y, W - 8.f, ROW_H,
            0.14f, 0.30f, 0.14f);
        MakeLbl("AEAddBtnL" + std::to_string(mDynIdx++), 4.f, Y, W - 8.f, ROW_H,
            mShowRegistry ? TEXT("▲ 닫기") : TEXT("+ 애니메이션 추가"),
            11.f, ETextAlignH::Center);
        mAddToggleBtn = AddBtn;
        Y += ROW_H + 4.f;
    }

    // ── 레지스트리 목록 ───────────────────────────────────────────────────────
    if (mShowRegistry)
    {
        for (auto& AnimName : CAnimRegistry::GetAll())
        {
            if (Y + ROW_H > H - 6.f) break;
            std::wstring WN(AnimName.begin(), AnimName.end());
            auto Btn = MakeBtn("AEReg" + std::to_string(mDynIdx), 8.f, Y, W - 16.f, ROW_H, 0.14f, 0.22f, 0.32f);
            MakeLbl("AERegL" + std::to_string(mDynIdx++), 12.f, Y, W - 20.f, ROW_H, WN.c_str(), 10.f);
            mRegBtns.push_back({ Btn, AnimName });
            Y += ROW_H + 2.f;
        }

        if (CAnimRegistry::GetAll().empty())
        {
            MakeLbl("AERegEmpty" + std::to_string(mDynIdx++), 8.f, Y, W - 16.f, ROW_H,
                TEXT("(등록된 애니메이션 없음)"), 10.f);
        }
    }
}

// ── 업데이트 ─────────────────────────────────────────────────────────────────

void CAnimEditorUI::Update(float DeltaTime)
{
    // mComps가 actor 실제 상태와 다르면 자동 갱신
    {
        auto A = mTarget.lock();
        int ActualAnimCompCount = 0;
        if (A)
        {
            for (auto& Comp : A->GetActorCompList())
                if (std::dynamic_pointer_cast<CAnimation2DComponent>(Comp))
                    ++ActualAnimCompCount;
        }
        bool NeedsRefresh = (ActualAnimCompCount != (int)mComps.size());
        if (!NeedsRefresh)
        {
            for (auto& CD : mComps)
                if (CD.Comp.expired()) { NeedsRefresh = true; break; }
        }
        if (NeedsRefresh)
        {
            SetTarget(mTarget);
        }
    }

    CWidgetContainer::Update(DeltaTime);

    auto World = mWorld.lock();
    if (!World) return;

    // ── 리사이즈 (코너 드래그) ────────────────────────────────────────────────
    if (auto Input = World->GetInput().lock())
    {
        FVector2 Mouse   = Input->GetMousePos();
        FVector2 Delta   = Input->GetMouseMove();
        bool Press   = Input->GetMouseState(EMouseType::LButton, EInputType::Press);
        bool Held    = Input->GetMouseState(EMouseType::LButton, EInputType::Hold);
        bool Release = Input->GetMouseState(EMouseType::LButton, EInputType::Release);

        FVector3 PP = GetPos(), PS = GetSize();
        float W = PS.x, H = PS.y;
        float CX[4] = { PP.x, PP.x + W - HANDLE_SZ, PP.x, PP.x + W - HANDLE_SZ };
        float CY[4] = { PP.y, PP.y, PP.y + H - HANDLE_SZ, PP.y + H - HANDLE_SZ };

        if (mActiveCorner == -1 && Press)
            for (int i = 0; i < 4; ++i)
                if (Mouse.x >= CX[i] && Mouse.x < CX[i] + HANDLE_SZ &&
                    Mouse.y >= CY[i] && Mouse.y < CY[i] + HANDLE_SZ)
                { mActiveCorner = i; break; }

        if (mActiveCorner >= 0 && Release) mActiveCorner = -1;

        if (mActiveCorner >= 0 && Held && (Delta.x != 0.f || Delta.y != 0.f))
        {
            float nx = PP.x, ny = PP.y, nw = W, nh = H;
            switch (mActiveCorner)
            {
            case 0: nx += Delta.x; ny += Delta.y; nw -= Delta.x; nh -= Delta.y; break;
            case 1:                ny += Delta.y; nw += Delta.x; nh -= Delta.y; break;
            case 2: nx += Delta.x;                nw -= Delta.x; nh += Delta.y; break;
            case 3:                                nw += Delta.x; nh += Delta.y; break;
            }
            nw = max(nw, 220.f); nh = max(nh, 200.f);
            SetPos(nx, ny); SetSize(nw, nh);
            UpdateHandles(nw, nh);
            Rebuild();
            return;
        }
    }

    // ── 탭 버튼 ──────────────────────────────────────────────────────────────
    for (auto& T : mTabBtns)
    {
        if (auto B = T.Btn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
        {
            mActiveComp = T.Idx;
            Rebuild();
            return;
        }
    }

    // ── 시퀀스 버튼 (클릭 → 재생) ────────────────────────────────────────────
    for (auto& S : mSeqBtns)
    {
        if (auto B = S.Btn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
        {
            PlaySeq(mActiveComp, S.Idx);
            return;
        }
    }

    // ── PlayTime / PlayRate 증감 ─────────────────────────────────────────────
    for (auto& P : mPropBtns)
    {
        bool Changed = false;
        if (auto B = P.Minus.lock(); B && B->GetWidgetState() == EWidgetState::Release)
        { P.Set(P.Get() - P.Step); Changed = true; }
        if (auto B = P.Plus.lock(); B && B->GetWidgetState() == EWidgetState::Release)
        { P.Set(P.Get() + P.Step); Changed = true; }
        if (Changed)
        {
            if (auto L = P.Lbl.lock())
            {
                TCHAR Buf[32]; swprintf_s(Buf, 32, L"%.2f", P.Get());
                L->SetText(Buf);
            }
        }
    }

    // ── 토글 (Loop / Reverse / Symmetry) ────────────────────────────────────
    for (auto& T : mToggleBtns)
    {
        if (auto B = T.Btn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
        {
            bool NewVal = !T.Get();
            T.Set(NewVal);
            if (auto L = T.Lbl.lock()) L->SetText(NewVal ? TEXT("ON") : TEXT("OFF"));
            B->SetTint(EWidgetState::Normal,
                NewVal ? 0.12f : 0.22f,
                NewVal ? 0.38f : 0.22f,
                NewVal ? 0.12f : 0.28f, 1.f);
        }
    }

    // ── 프레임 에디터 토글 ────────────────────────────────────────────────────
    if (auto B = mToggleFrameBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    {
        mShowFrameEditor = !mShowFrameEditor;
        Rebuild();
        return;
    }

    // ── 프레임 Prev / Next ────────────────────────────────────────────────────
    if (auto B = mFramePrevBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    {
        if (mActiveComp < (int)mComps.size())
        {
            auto& CD = mComps[mActiveComp];
            if (CD.Selected >= 0 && CD.Selected < (int)CD.Seqs.size())
            {
                auto& SD = CD.Seqs[CD.Selected];
                if (!SD.Frames.empty())
                {
                    SD.SelectedFrame = (SD.SelectedFrame - 1 + (int)SD.Frames.size()) % (int)SD.Frames.size();
                    if (auto SV = mSpriteViewer.lock()) SV->SelectFrame(SD.SelectedFrame);
                    Rebuild(); return;
                }
            }
        }
    }
    if (auto B = mFrameNextBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    {
        if (mActiveComp < (int)mComps.size())
        {
            auto& CD = mComps[mActiveComp];
            if (CD.Selected >= 0 && CD.Selected < (int)CD.Seqs.size())
            {
                auto& SD = CD.Seqs[CD.Selected];
                if (!SD.Frames.empty())
                {
                    SD.SelectedFrame = (SD.SelectedFrame + 1) % (int)SD.Frames.size();
                    if (auto SV = mSpriteViewer.lock()) SV->SelectFrame(SD.SelectedFrame);
                    Rebuild(); return;
                }
            }
        }
    }

    // ── 프레임 속성 +/- (변경 즉시 CAnimation2D에 적용) ─────────────────────
    {
        bool AnyChanged = false;
        for (auto& P : mFramePropBtns)
        {
            bool Changed = false;
            if (auto B = P.Minus.lock(); B && B->GetWidgetState() == EWidgetState::Release)
            { P.Set(P.Get() - P.Step); Changed = true; }
            if (auto B = P.Plus.lock(); B && B->GetWidgetState() == EWidgetState::Release)
            { P.Set(P.Get() + P.Step); Changed = true; }
            if (Changed)
            {
                if (auto L = P.Lbl.lock())
                {
                    TCHAR Buf[32]; swprintf_s(Buf, 32, L"%.1f", P.Get());
                    L->SetText(Buf);
                }
                AnyChanged = true;
            }
        }
        if (AnyChanged && mActiveComp < (int)mComps.size())
        {
            auto& CD = mComps[mActiveComp];
            if (CD.Selected >= 0)
            {
                ApplyFrames(mActiveComp, CD.Selected);
                SyncSpriteViewer(mActiveComp, CD.Selected);
            }
        }
    }

    // ── 프레임 추가 ──────────────────────────────────────────────────────────
    if (auto B = mAddFrameBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    {
        if (mActiveComp < (int)mComps.size())
        {
            auto& CD = mComps[mActiveComp];
            if (CD.Selected >= 0 && CD.Selected < (int)CD.Seqs.size())
            {
                auto& SD = CD.Seqs[CD.Selected];
                FFrameData NewFD;
                if (!SD.Frames.empty() && SD.SelectedFrame >= 0 && SD.SelectedFrame < (int)SD.Frames.size())
                    NewFD = SD.Frames[SD.SelectedFrame]; // 현재 프레임 복사
                SD.Frames.push_back(NewFD);
                SD.SelectedFrame = (int)SD.Frames.size() - 1;
                ApplyFrames(mActiveComp, CD.Selected);
                SyncSpriteViewer(mActiveComp, CD.Selected);
                Rebuild(); return;
            }
        }
    }

    // ── 선택 프레임 삭제 ─────────────────────────────────────────────────────
    if (auto B = mDelFrameBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    {
        if (mActiveComp < (int)mComps.size())
        {
            auto& CD = mComps[mActiveComp];
            if (CD.Selected >= 0 && CD.Selected < (int)CD.Seqs.size())
            {
                auto& SD = CD.Seqs[CD.Selected];
                int SF = SD.SelectedFrame;
                if (SF >= 0 && SF < (int)SD.Frames.size())
                {
                    SD.Frames.erase(SD.Frames.begin() + SF);
                    // 삭제 후 인덱스 보정
                    if (SD.Frames.empty())
                        SD.SelectedFrame = -1;
                    else if (SF >= (int)SD.Frames.size())
                        SD.SelectedFrame = (int)SD.Frames.size() - 1;
                    ApplyFrames(mActiveComp, CD.Selected);
                    SyncSpriteViewer(mActiveComp, CD.Selected);
                    Rebuild(); return;
                }
            }
        }
    }

    // ── 전체 프레임 삭제 ─────────────────────────────────────────────────────
    if (auto B = mClearFramesBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    {
        if (mActiveComp < (int)mComps.size())
        {
            auto& CD = mComps[mActiveComp];
            if (CD.Selected >= 0 && CD.Selected < (int)CD.Seqs.size())
            {
                CD.Seqs[CD.Selected].Frames.clear();
                CD.Seqs[CD.Selected].SelectedFrame = -1;
                ApplyFrames(mActiveComp, CD.Selected);
                SyncSpriteViewer(mActiveComp, CD.Selected);
                Rebuild(); return;
            }
        }
    }

    // ── 새 애니메이션 만들기 ──────────────────────────────────────────────────
    if (auto B = mNewAnimBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    {
        CreateNewAnim();
        return;
    }

    // ── 텍스처 설정 ───────────────────────────────────────────────────────────
    if (auto B = mSetTextureBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    {
        if (mActiveComp < (int)mComps.size())
        {
            auto& CD = mComps[mActiveComp];
            if (CD.Selected >= 0)
                SetAnimTexture(mActiveComp, CD.Selected);
        }
    }

    // ── TextureType 토글 ──────────────────────────────────────────────────────
    if (auto B = mTypeToggleBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    {
        if (mActiveComp < (int)mComps.size())
        {
            auto& CD = mComps[mActiveComp];
            if (CD.Selected >= 0 && CD.Selected < (int)CD.Seqs.size())
            {
                auto& SD = CD.Seqs[CD.Selected];
                SD.TextureType = (SD.TextureType == EAnimation2DTextureType::Frame)
                    ? EAnimation2DTextureType::SpriteSheet
                    : EAnimation2DTextureType::Frame;

                auto AnimMgr = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D);
                if (AnimMgr) AnimMgr->SetAnimationTextureType(SD.Name, SD.TextureType);

                Rebuild(); return;
            }
        }
    }

    // ── Add Animation 토글 ────────────────────────────────────────────────────
    if (auto B = mAddToggleBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    {
        mShowRegistry = !mShowRegistry;
        Rebuild();
        return;
    }

    // ── 레지스트리에서 선택 ───────────────────────────────────────────────────
    for (auto& R : mRegBtns)
    {
        if (auto B = R.Btn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
        {
            AddSeq(mActiveComp, R.Name);
            return;
        }
    }

    // ── Sprite Viewer 열기 ────────────────────────────────────────────────────
    if (auto B = mOpenViewerBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    {
        if (mActiveComp < (int)mComps.size())
        {
            auto& CD = mComps[mActiveComp];
            if (CD.Selected >= 0 && CD.Selected < (int)CD.Seqs.size())
                OpenSpriteViewer(mActiveComp, CD.Selected);
        }
    }

    // ── 애니메이션 저장 ──────────────────────────────────────────────────────
    if (auto B = mSaveAnimBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    {
        if (mActiveComp < (int)mComps.size())
        {
            auto& CD = mComps[mActiveComp];
            SaveAnim(mActiveComp, CD.Selected);
        }
    }

    // ── 애니메이션 불러오기 ──────────────────────────────────────────────────
    if (auto B = mLoadAnimBtn.lock(); B && B->GetWidgetState() == EWidgetState::Release)
    {
        LoadAnim();
        return;
    }

    // ── 현재 재생 프레임 카운터 (라이브) ─────────────────────────────────────
    if (mActiveComp < (int)mComps.size())
    {
        auto& CD = mComps[mActiveComp];
        if (CD.Selected >= 0 && CD.Selected < (int)CD.Seqs.size())
        {
            if (auto Comp = CD.Comp.lock())
            {
                int Frame = Comp->GetAnimationFrame();
                int Total = (int)CD.Seqs[CD.Selected].Frames.size();

                // 프레임 인덱스 라벨 실시간 업데이트 (에디터 토글 닫혀 있어도 동작)
                if (auto FT = mFrameIdxLbl.lock())
                {
                    TCHAR Buf[64];
                    swprintf_s(Buf, 64, L"Frame %d / %d", Frame + 1, Total);
                    FT->SetText(Buf);
                }
            }
        }
    }
}

CAnimEditorUI* CAnimEditorUI::Clone()
{
    return new CAnimEditorUI(*this);
}

// ── SpriteViewer 연동 ─────────────────────────────────────────────────────────

void CAnimEditorUI::SyncSpriteViewer(int ci, int si)
{
    auto SV = mSpriteViewer.lock();
    if (!SV || !SV->IsEnable()) return;
    if (ci < 0 || ci >= (int)mComps.size()) return;
    if (si < 0 || si >= (int)mComps[ci].Seqs.size()) return;

    auto& SD = mComps[ci].Seqs[si];
    std::vector<CSpriteViewerUI::FFrameRect> Rects;
    for (auto& FD : SD.Frames)
        Rects.push_back({ FD.Start, FD.Size });

    SV->SetFrames(Rects, SD.SelectedFrame);
}

void CAnimEditorUI::OpenSpriteViewer(int ci, int si)
{
    auto SV = mSpriteViewer.lock();
    if (!SV) return;
    if (ci < 0 || ci >= (int)mComps.size()) return;
    if (si < 0 || si >= (int)mComps[ci].Seqs.size()) return;

    auto& SD = mComps[ci].Seqs[si];

    // 텍스처 크기 얻기
    FVector2 TexSize = { 256.f, 256.f };
    auto AnimMgr = CAssetManager::GetInst()->GetSubManager<CAnimationManager>(EAssetType::Animation2D);
    if (AnimMgr)
    {
        auto Anim = AnimMgr->FindAnimation(SD.Name).lock();
        if (Anim)
        {
            auto Tex = Anim->GetTexture().lock();
            if (Tex)
            {
                auto Info = Tex->GetTexture(0);
                if (Info && Info->Width > 0)
                    TexSize = { (float)Info->Width, (float)Info->Height };
            }
        }
    }

    SV->ShowTexture(SD.TextureName, TexSize);

    // 현재 프레임 데이터 전달
    std::vector<CSpriteViewerUI::FFrameRect> Rects;
    for (auto& FD : SD.Frames)
        Rects.push_back({ FD.Start, FD.Size });
    SV->SetFrames(Rects, SD.SelectedFrame);

    // 콜백: SpriteViewer → AnimEditorUI 동기화
    SV->SetOnFrameChanged([this, ci, si](int fi, FVector2 Start, FVector2 Size)
    {
        if (ci >= (int)mComps.size()) return;
        if (si >= (int)mComps[ci].Seqs.size()) return;
        auto& SD2 = mComps[ci].Seqs[si];
        if (fi < 0 || fi >= (int)SD2.Frames.size()) return;
        SD2.Frames[fi].Start = Start;
        SD2.Frames[fi].Size  = Size;
        ApplyFrames(ci, si);
        Rebuild();
    });
        
    SV->SetOnFrameSelected([this, ci, si](int fi)
    {
        if (ci >= (int)mComps.size()) return;
        if (si >= (int)mComps[ci].Seqs.size()) return;
        mComps[ci].Seqs[si].SelectedFrame = fi;
        Rebuild();
    });

    SV->SetOnFrameDeleted([this, ci, si](int fi)
    {
        if (ci >= (int)mComps.size()) return;
        if (si >= (int)mComps[ci].Seqs.size()) return;
        auto& SD2 = mComps[ci].Seqs[si];
        if (fi < 0 || fi >= (int)SD2.Frames.size()) return;
        SD2.Frames.erase(SD2.Frames.begin() + fi);
        if (SD2.Frames.empty())
            SD2.SelectedFrame = -1;
        else if (SD2.SelectedFrame >= (int)SD2.Frames.size())
            SD2.SelectedFrame = (int)SD2.Frames.size() - 1;
        ApplyFrames(ci, si);
        Rebuild();
        // SpriteViewer에 다시 동기화
        if (auto SV2 = mSpriteViewer.lock())
        {
            std::vector<CSpriteViewerUI::FFrameRect> R;
            for (auto& FD2 : SD2.Frames) R.push_back({ FD2.Start, FD2.Size });
            SV2->SetFrames(R, SD2.SelectedFrame);
        }
    });

    SV->SetOnFrameAdded([this, ci, si](FVector2 Start, FVector2 Size)
    {
        if (ci >= (int)mComps.size()) return;
        if (si >= (int)mComps[ci].Seqs.size()) return;
        auto& SD2 = mComps[ci].Seqs[si];
        FFrameData NewFD;
        NewFD.Start = Start;
        NewFD.Size  = Size;
        SD2.Frames.push_back(NewFD);
        SD2.SelectedFrame = (int)SD2.Frames.size() - 1;
        ApplyFrames(ci, si);
        Rebuild();
        // SpriteViewer에 다시 동기화
        if (auto SV2 = mSpriteViewer.lock())
        {
            std::vector<CSpriteViewerUI::FFrameRect> R;
            for (auto& FD2 : SD2.Frames) R.push_back({ FD2.Start, FD2.Size });
            SV2->SetFrames(R, SD2.SelectedFrame);
        }
    });

    SV->SetEnable(true);
    LOG_DEBUG("[AnimEditor] Sprite Viewer opened for: %s", SD.Name.c_str());
}
