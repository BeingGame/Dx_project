#include "PrefabManager.h"

#include "World/Actor.h"
#include "World/World.h"

// 엔진 컴포넌트
#include "World/MeshComponent.h"
#include "World/CameraComponent.h"
#include "World/Animation2DComponent.h"
#include "World/ColliderBox2D.h"
#include "World/ColliderSphere2D.h"
#include "World/CharacterMovementComponent.h"
#include "World/ProjectileMovementComponent.h"
#include "World/AIComponent.h"
#include "World/SoundComponent.h"
#include "World/WidgetComponent.h"
#include "World/TileMapComponent.h"

// 클라이언트 컴포넌트
#include "Component/StatusComponent.h"
#include "Component/TurretSkillComp.h"
#include "Component/DirectionInputComponent.h"
#include "Component/AnimStateComponent.h"

#include "LogManager.h"

#include "DialogUtil.h"

#include <fstream>

CPrefabManager* CPrefabManager::mInstance = nullptr;

CPrefabManager::CPrefabManager()
{}

CPrefabManager::~CPrefabManager()
{}

bool CPrefabManager::Init()
{
    std::string AssetDir = DialogUtil::GetExeDir() + "Asset\\";
    CreateDirectoryA(AssetDir.c_str(), nullptr);
    mPrefabDir = AssetDir + "Prefab\\";
    CreateDirectoryA(mPrefabDir.c_str(), nullptr);
    RegisterComponents();
    return true;
}

void CPrefabManager::RegisterComponents()
{
    auto Reg = [&](const std::string& TypeName, SpawnFunc Fn)
    {
        mFactoryMap[TypeName] = std::move(Fn);
    };

    // 씬 컴포넌트 (계층 구조 지원, Parent 이름 전달)
    Reg("MeshComponent", [](std::shared_ptr<CActor> A, const std::string& N, const std::string& P) {
        A->CreateComponent<CMeshComponent>(N, P.empty() ? "Root" : P);
    });
    Reg("CameraComponent", [](std::shared_ptr<CActor> A, const std::string& N, const std::string& P) {
        A->CreateComponent<CCameraComponent>(N, P.empty() ? "Root" : P);
    });
    Reg("Animation2DComponent", [](std::shared_ptr<CActor> A, const std::string& N, const std::string& P) {
        A->CreateComponent<CAnimation2DComponent>(N, P.empty() ? "Root" : P);
    });
    Reg("ColliderBox2D", [](std::shared_ptr<CActor> A, const std::string& N, const std::string& P) {
        A->CreateComponent<CColliderBox2D>(N, P.empty() ? "Root" : P);
    });
    Reg("ColliderSphere2D", [](std::shared_ptr<CActor> A, const std::string& N, const std::string& P) {
        A->CreateComponent<CColliderSphere2D>(N, P.empty() ? "Root" : P);
    });
    Reg("WidgetComponent", [](std::shared_ptr<CActor> A, const std::string& N, const std::string& P) {
        A->CreateComponent<CWidgetComponent>(N, P.empty() ? "Root" : P);
    });
    Reg("TileMapComponent", [](std::shared_ptr<CActor> A, const std::string& N, const std::string& P) {
        A->CreateComponent<CTileMapComponent>(N, P.empty() ? "Root" : P);
    });

    // 액터 컴포넌트 (계층 구조 없음, Parent 무시)
    Reg("CharacterMovement", [](std::shared_ptr<CActor> A, const std::string& N, const std::string&) {
        A->CreateComponent<CCharacterMovementComponent>(N);
    });
    Reg("ProjectileMovement", [](std::shared_ptr<CActor> A, const std::string& N, const std::string&) {
        A->CreateComponent<CProjectileMovementComponent>(N);
    });
    Reg("AIComponent", [](std::shared_ptr<CActor> A, const std::string& N, const std::string&) {
        A->CreateComponent<CAIComponent>(N);
    });
    Reg("SoundComponent", [](std::shared_ptr<CActor> A, const std::string& N, const std::string&) {
        A->CreateComponent<CSoundComponent>(N);
    });
    Reg("StatusComponent", [](std::shared_ptr<CActor> A, const std::string& N, const std::string&) {
        A->CreateComponent<CStatusComponent>(N);
    });
    Reg("TurretSkillComp", [](std::shared_ptr<CActor> A, const std::string& N, const std::string&) {
        A->CreateComponent<CTurretSkillComp>(N);
    });
    Reg("DirectionInput", [](std::shared_ptr<CActor> A, const std::string& N, const std::string&) {
        A->CreateComponent<CDirectionInputComponent>(N);
    });
    Reg("AnimState", [](std::shared_ptr<CActor> A, const std::string& N, const std::string&) {
        A->CreateComponent<CAnimStateComponent>(N);
    });

    // CComponent::GetTypeName()이 돌려주는 엔진 타입명으로도 찾을 수 있게 별칭을 건다.
    // 액터에서 컴포넌트 구성을 그대로 읽어 저장할 때 이 이름이 쓰이고,
    // 예전 프리팹의 짧은 이름도 그대로 남아 있어 둘 다 열린다.
    auto Alias = [&](const std::string& EngineName, const std::string& ShortName)
    {
        auto It = mFactoryMap.find(ShortName);
        if (It != mFactoryMap.end())
            mFactoryMap[EngineName] = It->second;
    };

    Alias("CMeshComponent",                "MeshComponent");
    Alias("CCameraComponent",              "CameraComponent");
    Alias("CAnimation2DComponent",         "Animation2DComponent");
    Alias("CColliderBox2D",                "ColliderBox2D");
    Alias("CColliderSphere2D",             "ColliderSphere2D");
    Alias("CWidgetComponent",              "WidgetComponent");
    Alias("CTileMapComponent",             "TileMapComponent");
    Alias("CCharacterMovementComponent",   "CharacterMovement");
    Alias("CProjectileMovementComponent",  "ProjectileMovement");
    Alias("CAIComponent",                  "AIComponent");
    Alias("CSoundComponent",               "SoundComponent");
    Alias("CStatusComponent",              "StatusComponent");
    Alias("CTurretSkillComp",              "TurretSkillComp");
    Alias("CDirectionInputComponent",      "DirectionInput");
    Alias("CAnimStateComponent",           "AnimState");
}

void CPrefabManager::BuildPrefabData(const std::shared_ptr<CActor>& Actor, FPrefabData& OutData)
{
    if (!Actor) return;

    OutData.ActorTag   = Actor->GetActorTag();
    OutData.WorldPos   = Actor->GetWorldPos();
    OutData.WorldScale = Actor->GetWorldScale();
    OutData.WorldRot   = Actor->GetWorldRot();
    OutData.Components.clear();

    // 씬 컴포넌트를 먼저 담는다.
    // 스폰할 때 이 순서대로 붙으므로 부모가 자식보다 앞에 와야 하고,
    // 애니메이션 컴포넌트가 메시를 찾을 수 있으려면 메시가 먼저여야 한다.
    auto AddSceneComp = [&OutData](const std::shared_ptr<CSceneComponent>& Comp)
    {
        FPrefabComponentEntry Entry;
        Entry.TypeName = Comp->GetTypeName();
        Entry.Name     = Comp->GetName();

        if (auto Parent = Comp->GetParent().lock())
            Entry.Parent = Parent->GetName();

        OutData.Components.push_back(Entry);
    };

    //루트를 맨 앞에 둔다. 스폰할 때 처음 붙는 씬 컴포넌트가 루트가 되기 때문이다.
    //루트를 지우고 자식이 승격된 액터는 mSceneCompList의 0번이 루트가 아닐 수 있다.
    auto Root = Actor->GetRootComponent().lock();

    if (Root)
        AddSceneComp(Root);

    for (const auto& Comp : Actor->GetSceneCompList())
    {
        if (!Comp || Comp == Root) continue;

        AddSceneComp(Comp);
    }

    for (const auto& Comp : Actor->GetActorCompList())
    {
        if (!Comp) continue;

        FPrefabComponentEntry Entry;
        Entry.TypeName = Comp->GetTypeName();
        Entry.Name     = Comp->GetName();

        OutData.Components.push_back(Entry);
    }
}

std::string CPrefabManager::GetPrefabFilePath(const std::string& PrefabName) const
{
    return mPrefabDir + PrefabName + ".prefab";  // mPrefabDir은 '\\'로 끝남
}

// ---- 컴포넌트 조회 ----

std::shared_ptr<CComponent> CPrefabManager::FindComponentByName(
    const std::shared_ptr<CActor>& Actor, const std::string& CompName)
{
    if (!Actor || CompName.empty())
        return nullptr;

    for (const auto& Comp : Actor->GetSceneCompList())
    {
        if (Comp && Comp->GetName() == CompName)
            return Comp;
    }

    for (const auto& Comp : Actor->GetActorCompList())
    {
        if (Comp && Comp->GetName() == CompName)
            return Comp;
    }

    return nullptr;
}

// ---- 저장 ----

void CPrefabManager::SavePrefab(const std::string& PrefabName, const FPrefabData& Data,
                                std::shared_ptr<CActor> Actor)
{
    std::ofstream File(GetPrefabFilePath(PrefabName));
    if (!File.is_open())
    {
        LOG_ERROR("[PrefabManager] Save failed: %s", PrefabName.c_str());
        return;
    }

    // 액터 정보
    File << "Tag=" << Data.ActorTag << "\n";
    File << "PX=" << Data.WorldPos.x
         << " PY=" << Data.WorldPos.y
         << " PZ=" << Data.WorldPos.z << "\n";
    File << "SX=" << Data.WorldScale.x
         << " SY=" << Data.WorldScale.y
         << " SZ=" << Data.WorldScale.z << "\n";
    File << "RX=" << Data.WorldRot.x
         << " RY=" << Data.WorldRot.y
         << " RZ=" << Data.WorldRot.z << "\n";

    // 컴포넌트 목록 (TypeName|CompName|ParentName)
    File << "Count=" << Data.Components.size() << "\n";
    for (size_t i = 0; i < Data.Components.size(); ++i)
    {
        const auto& C = Data.Components[i];
        File << "C" << i << "=" << C.TypeName << "|" << C.Name << "|" << C.Parent << "\n";
    }

    // 컴포넌트별 상태 블록.
    // 월드 저장과 같은 CComponent::Save()를 그대로 쓰므로,
    // 컴포넌트가 Save/Load를 구현해두면 프리팹에도 자동으로 따라온다.
    int SavedBlocks = 0;

    if (Actor)
    {
        for (size_t i = 0; i < Data.Components.size(); ++i)
        {
            auto Comp = FindComponentByName(Actor, Data.Components[i].Name);
            if (!Comp) continue;

            File << "[Comp:" << i << "]\n";
            Comp->Save(File);
            File << "\n";
            ++SavedBlocks;
        }
    }

    LOG_DEBUG("[PrefabManager] Saved: %s (%zu components, %d data blocks)",
              PrefabName.c_str(), Data.Components.size(), SavedBlocks);
}

// ---- 불러오기 ----

bool CPrefabManager::LoadPrefabData(const std::string& PrefabName, FPrefabData& OutData) const
{
    return LoadPrefabDataFromPath(GetPrefabFilePath(PrefabName), OutData);
}

bool CPrefabManager::LoadPrefabDataFromPath(const std::string& FullPath, FPrefabData& OutData) const
{
    std::ifstream File(FullPath);
    if (!File.is_open())
        return false;

    // -1이면 헤더 영역, 0 이상이면 그 인덱스의 [Comp:i] 블록을 읽는 중
    int CurrentBlock = -1;

    std::string Line;
    while (std::getline(File, Line))
    {
        // 윈도우 줄바꿈(\r) 제거 — 텍스트 모드라 보통 없지만 방어적으로
        if (!Line.empty() && Line.back() == '\r')
            Line.pop_back();

        if (Line.empty()) continue;

        // 섹션 헤더: [Comp:3]
        if (Line[0] == '[')
        {
            CurrentBlock = -1;

            size_t Colon = Line.find(':');
            size_t Close = Line.find(']');

            if (Line.compare(0, 6, "[Comp:") == 0 &&
                Colon != std::string::npos && Close != std::string::npos && Close > Colon)
            {
                try { CurrentBlock = std::stoi(Line.substr(Colon + 1, Close - Colon - 1)); }
                catch (...) { CurrentBlock = -1; }
            }
            continue;
        }

        size_t Eq = Line.find('=');
        if (Eq == std::string::npos) continue;

        std::string Key = Line.substr(0, Eq);
        std::string Val = Line.substr(Eq + 1);

        // 컴포넌트 블록 안이면 그 항목의 Props에 쌓는다.
        if (CurrentBlock >= 0)
        {
            if (CurrentBlock < (int)OutData.Components.size())
                OutData.Components[CurrentBlock].Props[Key] = Val;
            continue;
        }

        if (Key == "Tag")
        {
            OutData.ActorTag = Val;
        }
        else if (Key == "PX")
        {
            sscanf_s(Val.c_str(), "%f PY=%f PZ=%f",
                     &OutData.WorldPos.x, &OutData.WorldPos.y, &OutData.WorldPos.z);
        }
        else if (Key == "SX")
        {
            sscanf_s(Val.c_str(), "%f SY=%f SZ=%f",
                     &OutData.WorldScale.x, &OutData.WorldScale.y, &OutData.WorldScale.z);
        }
        else if (Key == "RX")
        {
            sscanf_s(Val.c_str(), "%f RY=%f RZ=%f",
                     &OutData.WorldRot.x, &OutData.WorldRot.y, &OutData.WorldRot.z);
        }
        else if (Key == "Count")
        {
            OutData.Components.reserve(std::stoi(Val));
        }
        else if (!Key.empty() && Key[0] == 'C' && Key.size() > 1 && std::isdigit((unsigned char)Key[1]))
        {
            // "TypeName|CompName|ParentName" 형식 파싱
            FPrefabComponentEntry Entry;
            size_t P1 = Val.find('|');
            size_t P2 = (P1 != std::string::npos) ? Val.find('|', P1 + 1) : std::string::npos;

            Entry.TypeName = (P1 != std::string::npos) ? Val.substr(0, P1) : Val;
            Entry.Name     = (P1 != std::string::npos && P2 != std::string::npos)
                                 ? Val.substr(P1 + 1, P2 - P1 - 1)
                                 : (P1 != std::string::npos ? Val.substr(P1 + 1) : "");
            Entry.Parent   = (P2 != std::string::npos) ? Val.substr(P2 + 1) : "";

            OutData.Components.push_back(Entry);
        }
    }

    return true;
}

// ---- 스폰 ----

std::weak_ptr<CActor> CPrefabManager::SpawnPrefab(const std::string& PrefabName,
                                                    std::weak_ptr<CWorld> World,
                                                    const FVector3& SpawnPos)
{
    return SpawnPrefabFromFile(GetPrefabFilePath(PrefabName), World, SpawnPos);
}

std::weak_ptr<CActor> CPrefabManager::SpawnPrefabFromFile(const std::string& FullPath,
                                                    std::weak_ptr<CWorld> World,
                                                    const FVector3& SpawnPos)
{
    std::string PrefabName = DialogUtil::ExtractBaseName(FullPath);

    FPrefabData Data;
    if (!LoadPrefabDataFromPath(FullPath, Data))
    {
        LOG_ERROR("[PrefabManager] Prefab not found: %s", FullPath.c_str());
        return {};
    }

    auto WorldPtr = World.lock();
    if (!WorldPtr) return {};

    std::string ActorName = PrefabName + "_" + std::to_string(mSpawnCount++);
    auto Actor = WorldPtr->CreateActor<CActor>(ActorName);
    auto ActorPtr = Actor.lock();
    if (!ActorPtr) return {};

    ActorPtr->SetActorTag(Data.ActorTag);

    for (const auto& Entry : Data.Components)
        AttachComponent(Entry.TypeName, ActorPtr, Entry.Name, Entry.Parent);

    // 컴포넌트를 전부 붙인 뒤에 상태를 복원한다.
    // 순서가 중요하다 — Animation2DComponent::Load는 소유 액터에서 메시를 찾아 물리므로
    // 그 시점에 다른 컴포넌트들이 이미 존재해야 한다.
    for (const auto& Entry : Data.Components)
    {
        if (Entry.Props.empty()) continue;

        auto Comp = FindComponentByName(ActorPtr, Entry.Name);
        if (Comp) Comp->Load(Entry.Props);
    }

    // 트랜스폼 적용 (SceneComponent가 하나라도 있어야 효과 있음)
    FVector3 FinalPos = (SpawnPos.x == 0.f && SpawnPos.y == 0.f && SpawnPos.z == 0.f)
                        ? Data.WorldPos
                        : SpawnPos;
    ActorPtr->SetWorldPos(FinalPos);
    ActorPtr->SetWorldScale(Data.WorldScale);
    ActorPtr->SetWorldRotation(Data.WorldRot);

    LOG_DEBUG("[PrefabManager] Spawned '%s' from '%s'", ActorName.c_str(), PrefabName.c_str());
    return Actor;
}

// ---- 조회 ----

bool CPrefabManager::HasPrefab(const std::string& PrefabName) const
{
    std::ifstream File(GetPrefabFilePath(PrefabName));
    return File.is_open();
}

std::vector<std::string> CPrefabManager::GetPrefabNames() const
{
    std::vector<std::string> Names;

    std::string SearchPath = mPrefabDir + "*.prefab";
    WIN32_FIND_DATAA FindData = {};
    HANDLE hFind = FindFirstFileA(SearchPath.c_str(), &FindData);
    if (hFind == INVALID_HANDLE_VALUE) return Names;

    do
    {
        std::string FileName = FindData.cFileName;
        size_t Dot = FileName.rfind(".prefab");
        if (Dot != std::string::npos)
            Names.push_back(FileName.substr(0, Dot));
    }
    while (FindNextFileA(hFind, &FindData));

    FindClose(hFind);
    return Names;
}

// ---- 컴포넌트 부착 ----

bool CPrefabManager::AttachComponent(const std::string& TypeName,
                                      std::shared_ptr<CActor> Actor,
                                      const std::string& CompName,
                                      const std::string& Parent)
{
    auto It = mFactoryMap.find(TypeName);
    if (It == mFactoryMap.end())
    {
        LOG_WARNING("[PrefabManager] Unknown component type: %s", TypeName.c_str());
        return false;
    }
    It->second(Actor, CompName, Parent);
    return true;
}
