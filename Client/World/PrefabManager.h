#pragma once

#include "EngineInfo.h"

struct FPrefabComponentEntry
{
    std::string TypeName;  // "MeshComponent", "ColliderBox2D" 등
    std::string Name;      // 컴포넌트 이름
    std::string Parent;    // 부모 컴포넌트 이름 ("" = 자동 루트)

    // 컴포넌트가 CComponent::Save()로 남긴 키=값들.
    // 스폰할 때 그대로 CComponent::Load()에 넘겨서 상태를 복원한다.
    // (애니메이션 시퀀스, 메시/머티리얼, 콜라이더 크기 등)
    std::unordered_map<std::string, std::string> Props;
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

    // 선택된 Actor의 컴포넌트 구성을 파일로 저장.
    // Actor를 같이 넘기면 각 컴포넌트의 Save()를 호출해 내부 상태까지 기록한다.
    // (안 넘기면 예전처럼 구성 목록만 저장된다)
    void SavePrefab(const std::string& PrefabName, const FPrefabData& Data,
                    std::shared_ptr<class CActor> Actor = nullptr);

    // 액터에서 이름으로 컴포넌트를 찾는다. (씬/액터 컴포넌트 양쪽 탐색)
    static std::shared_ptr<class CComponent> FindComponentByName(
        const std::shared_ptr<class CActor>& Actor, const std::string& CompName);

    // 살아 있는 액터를 그대로 훑어 저장용 데이터를 만든다.
    // 에디터가 따로 추적하던 목록에 의존하지 않으므로,
    // 월드에서 불러오거나 프리팹으로 스폰한 액터도 그대로 다시 저장된다.
    static void BuildPrefabData(const std::shared_ptr<class CActor>& Actor, FPrefabData& OutData);

    // Asset\Prefab\ 안의 프리팹 이름으로 FPrefabData를 읽어옴
    bool LoadPrefabData(const std::string& PrefabName, FPrefabData& OutData) const;

    // 전체 경로로 읽어옴. 파일 다이얼로그로 폴더 밖의 파일을 고른 경우에 쓴다.
    bool LoadPrefabDataFromPath(const std::string& FullPath, FPrefabData& OutData) const;

    // 프리팹을 월드에 스폰
    std::weak_ptr<class CActor> SpawnPrefab(const std::string& PrefabName,
                                            std::weak_ptr<class CWorld> World,
                                            const FVector3& SpawnPos = FVector3(0.f, 0.f, 0.f));

    // 전체 경로로 스폰. Asset\Prefab\ 밖에 있는 파일도 열린다.
    std::weak_ptr<class CActor> SpawnPrefabFromFile(const std::string& FullPath,
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
