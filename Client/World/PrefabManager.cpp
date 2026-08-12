#include "PrefabManager.h"

#include "World/Actor.h"
#include "World/World.h"

// Engine components
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

// Client components
#include "Component/StatusComponent.h"
#include "Component/TurretSkillComp.h"

#include "LogManager.h"

#include <fstream>

CPrefabManager* CPrefabManager::mInstance = nullptr;

bool CPrefabManager::Init()
{
    mPrefabDir = "Prefab/";
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

    // SceneComponents (계층 구조 지원, Parent 전달)
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

    // ActorComponents (계층 구조 없음, Parent 무시)
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
}

std::string CPrefabManager::GetPrefabFilePath(const std::string& PrefabName) const
{
    return mPrefabDir + PrefabName + ".prefab";
}

// ---- Save ----

void CPrefabManager::SavePrefab(const std::string& PrefabName, const FPrefabData& Data)
{
    std::ofstream File(GetPrefabFilePath(PrefabName));
    if (!File.is_open())
    {
        LOG_ERROR("[PrefabManager] Save failed: %s", PrefabName.c_str());
        return;
    }

    // Actor info
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

    // Component list  (TypeName|CompName|ParentName)
    File << "Count=" << Data.Components.size() << "\n";
    for (size_t i = 0; i < Data.Components.size(); ++i)
    {
        const auto& C = Data.Components[i];
        File << "C" << i << "=" << C.TypeName << "|" << C.Name << "|" << C.Parent << "\n";
    }

    LOG_DEBUG("[PrefabManager] Saved: %s (%zu components)",
              PrefabName.c_str(), Data.Components.size());
}

// ---- Load ----

bool CPrefabManager::LoadPrefabData(const std::string& PrefabName, FPrefabData& OutData) const
{
    std::ifstream File(GetPrefabFilePath(PrefabName));
    if (!File.is_open())
        return false;

    std::string Line;
    while (std::getline(File, Line))
    {
        if (Line.empty()) continue;

        size_t Eq = Line.find('=');
        if (Eq == std::string::npos) continue;

        std::string Key = Line.substr(0, Eq);
        std::string Val = Line.substr(Eq + 1);

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
            // Parse "TypeName|CompName|ParentName"
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

// ---- Spawn ----

std::weak_ptr<CActor> CPrefabManager::SpawnPrefab(const std::string& PrefabName,
                                                    std::weak_ptr<CWorld> World,
                                                    const FVector3& SpawnPos)
{
    FPrefabData Data;
    if (!LoadPrefabData(PrefabName, Data))
    {
        LOG_ERROR("[PrefabManager] Prefab not found: %s", PrefabName.c_str());
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

// ---- Query ----

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

// ---- Component attach helper ----

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
