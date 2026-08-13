#pragma once

#include "EngineInfo.h"

struct FPrefabComponentEntry
{
    std::string TypeName;  // "MeshComponent", "ColliderBox2D" 등
    std::string Name;      // 컴포넌트 이름
    std::string Parent;    // 부모 컴포넌트 이름 ("" = 자동 루트)
};

struct FPrefabData
{
    std::string ActorTag = "Actor";
    FVector3    WorldPos;
    FVector3    WorldScale = FVector3(1.f, 1.f, 1.f);
    FVector3    WorldRot;
    std::vector<FPrefabComponentEntry> Components;
};

class CPrefabManager
{
    Singleton(CPrefabManager)

public:
    using SpawnFunc = std::function<void(std::shared_ptr<class CActor>,
                                        const std::string& CompName,
                                        const std::string& Parent)>;

private:
    std::string mPrefabDir;
    std::unordered_map<std::string, SpawnFunc> mFactoryMap;
    int mSpawnCount = 0;

public:
    bool Init();

    // 선택된 Actor의 컴포넌트 구성을 파일로 저장
    void SavePrefab(const std::string& PrefabName, const FPrefabData& Data);

    // 파일에서 FPrefabData를 읽어옴
    bool LoadPrefabData(const std::string& PrefabName, FPrefabData& OutData) const;

    // 프리팹을 월드에 스폰
    std::weak_ptr<class CActor> SpawnPrefab(const std::string& PrefabName,
                                            std::weak_ptr<class CWorld> World,
                                            const FVector3& SpawnPos = FVector3(0.f, 0.f, 0.f));

    bool HasPrefab(const std::string& PrefabName) const;

    // Prefab/ 폴더에 있는 모든 프리팹 이름 목록 반환
    std::vector<std::string> GetPrefabNames() const;

    // 등록된 타입명으로 컴포넌트를 Actor에 붙임 (EditorMenuBar에서도 사용)
    bool AttachComponent(const std::string& TypeName,
                         std::shared_ptr<class CActor> Actor,
                         const std::string& CompName,
                         const std::string& Parent = "");

private:
    void RegisterComponents();
    std::string GetPrefabFilePath(const std::string& PrefabName) const;
};
