#include "World.h"
#include "CameraManager.h"
#include "Input.h"
#include "WorldCollision.h"
#include "UIManager.h"

#include "MeshComponent.h"
#include "CameraComponent.h"
#include "Animation2DComponent.h"
#include "ColliderBox2D.h"
#include "ColliderSphere2D.h"
#include "CharacterMovementComponent.h"
#include "ProjectileMovementComponent.h"
#include "AIComponent.h"
#include "SoundComponent.h"
#include "WidgetComponent.h"
#include "TileMapComponent.h"

#include "../LogManager.h"

#include <fstream>
#include <sstream>

std::unordered_map<std::string, CWorld::ActorFactory> CWorld::sActorFactoryMap;
std::unordered_map<std::string, CWorld::CompFactory>  CWorld::sCompFactoryMap;

CWorld::CWorld()
{}

CWorld::~CWorld()
{
	for (FTimerHandle& Handle : mTimerHandleList)
	{
		CTimeManager::ClearTimer(Handle);
	}

	mTimerHandleList.clear();

}

void CWorld::InputActive()
{
	mInput->DeviceAcquire();
}

void CWorld::InputDeActive()
{
	mInput->DeviceUnAcquire();
}

bool CWorld::Init()
{
	mCameraManager = std::make_shared<CCameraManager>();

	if (!mCameraManager->Init())
	{
		return false;
	}

	mInput = std::make_shared<CInput>();
	mInput->SetWorld(GetThisPtr());

	if (!mInput->Init())
	{
		return false;
	}

	mCollision = std::make_shared<CWorldCollision>();
	mCollision->SetWorld(GetThisPtr());

	if (!mCollision->Init())
	{
		return false;
	}

	mUIManager = std::make_shared<CUIManager>();
	mUIManager->SetWorld(GetThisPtr());

	if (!mUIManager->Init())
	{
		return false;
	}

	// 엔진 컴포넌트 타입 등록 (프로세스당 한 번)
	static bool sEngineTypesRegistered = false;
	if (!sEngineTypesRegistered)
	{
		sEngineTypesRegistered = true;
		RegisterComponentType<CMeshComponent>("CMeshComponent");
		RegisterComponentType<CCameraComponent>("CCameraComponent");
		RegisterComponentType<CAnimation2DComponent>("CAnimation2DComponent");
		RegisterComponentType<CColliderBox2D>("CColliderBox2D");
		RegisterComponentType<CColliderSphere2D>("CColliderSphere2D");
		RegisterComponentType<CCharacterMovementComponent>("CCharacterMovementComponent");
		RegisterComponentType<CProjectileMovementComponent>("CProjectileMovementComponent");
		RegisterComponentType<CAIComponent>("CAIComponent");
		RegisterComponentType<CSoundComponent>("CSoundComponent");
		RegisterComponentType<CWidgetComponent>("CWidgetComponent");
		RegisterComponentType<CTileMapComponent>("CTileMapComponent");
	}

	return true;
}

void CWorld::Update(float DeltaTime)
{
	mInput->Update(DeltaTime);

	auto iter = mActorList.begin();
	auto iterEnd = mActorList.end();

	for (; iter != iterEnd;)
	{
		if (!iter->second->IsAlive())
		{
			iter = mActorList.erase(iter);
			iterEnd = mActorList.end();
			continue;
		}
		else if (!iter->second->IsEnable())
		{
			++iter;
			continue;
		}

		iter->second->Update(DeltaTime);
		++iter;
	}

	//유효한 카메라들만 남기도록 업데이트
	mCameraManager->Update(DeltaTime);

	mUIManager->Update(DeltaTime);
}

void CWorld::PostUpdate(float DeltaTime)
{
	mCollision->UpdateInfo();

	auto iter = mActorList.begin();
	auto iterEnd = mActorList.end();

	for (; iter != iterEnd;)
	{
		if (!iter->second->IsAlive())
		{
			iter = mActorList.erase(iter);
			iterEnd = mActorList.end();
			continue;
		}
		else if (!iter->second->IsEnable())
		{
			++iter;
			continue;
		}

		iter->second->PostUpdate(DeltaTime);
		++iter;
	}

	mCollision->Update(DeltaTime);

	iter = mActorList.begin();
	iterEnd = mActorList.end();

	for (; iter != iterEnd;)
	{
		if (!iter->second->IsAlive())
		{
			iter = mActorList.erase(iter);
			iterEnd = mActorList.end();
			continue;
		}
		else if (!iter->second->IsEnable())
		{
			++iter;
			continue;
		}

		iter->second->UpdateTransform();
		++iter;
	}


}

void CWorld::Render()
{
	auto iter = mActorList.begin();
	auto iterEnd = mActorList.end();

	for (; iter != iterEnd;)
	{
		if (!iter->second->IsAlive())
		{
			iter = mActorList.erase(iter);
			iterEnd = mActorList.end();
			continue;
		}
		else if (!iter->second->IsEnable())
		{
			++iter;
			continue;
		}

		iter->second->Render();
		++iter;
	}

#ifdef _DEBUG

	mCollision->Render();

#endif

	//연산이 종료된 충돌체의 오브젝트를 풀에 반환한다.
	mCollision->ReturnNodePool();

}

void CWorld::PostRender()
{
	auto iter = mActorList.begin();
	auto iterEnd = mActorList.end();

	for (; iter != iterEnd;)
	{
		if (!iter->second->IsAlive())
		{
			iter = mActorList.erase(iter);
			iterEnd = mActorList.end();
			continue;
		}
		else if (!iter->second->IsEnable())
		{
			++iter;
			continue;
		}

		iter->second->PostRender();
		++iter;
	}

}

void CWorld::RenderUI()
{
	mUIManager->Render();
}

// ---- 월드 저장 ----

void CWorld::SaveScene(const std::string& FilePath) const
{
	std::ofstream File(FilePath);
	if (!File.is_open())
	{
		LOG_ERROR("[World] SaveScene failed: %s", FilePath.c_str());
		return;
	}

	File << "SceneVersion=1\n";
	File << "ActorCount=" << mActorList.size() << "\n\n";

	int Idx = 0;
	for (const auto& Pair : mActorList)
	{
		Pair.second->SaveScene(File, Idx++);
	}

	LOG_DEBUG("[World] Scene saved: %s (%zu actors)", FilePath.c_str(), mActorList.size());
}

// ---- 월드 불러오기 ----

static std::unordered_map<std::string, std::string> ParseSection(
	const std::vector<std::string>& Lines, size_t Start, size_t End)
{
	std::unordered_map<std::string, std::string> Props;
	for (size_t i = Start; i < End; ++i)
	{
		const std::string& Line = Lines[i];
		if (Line.empty() || Line[0] == '[') continue;
		size_t Eq = Line.find('=');
		if (Eq == std::string::npos) continue;
		Props[Line.substr(0, Eq)] = Line.substr(Eq + 1);
	}
	return Props;
}

bool CWorld::LoadScene(const std::string& FilePath)
{
	std::ifstream File(FilePath);
	if (!File.is_open())
	{
		LOG_ERROR("[World] LoadScene failed: %s", FilePath.c_str());
		return false;
	}

	// 전체 라인 읽기
	std::vector<std::string> Lines;
	std::string Line;
	while (std::getline(File, Line))
	{
		// Windows 줄바꿈(\r) 제거
		if (!Line.empty() && Line.back() == '\r')
			Line.pop_back();
		Lines.push_back(Line);
	}

	// 섹션 경계 탐색: "[섹션명]" 형태의 헤더 라인 기준
	struct FSection { std::string Header; size_t Start; size_t End; };
	std::vector<FSection> Sections;

	for (size_t i = 0; i < Lines.size(); ++i)
	{
		if (!Lines[i].empty() && Lines[i][0] == '[')
		{
			std::string Header = Lines[i].substr(1, Lines[i].size() - 2);
			Sections.push_back({ Header, i + 1, 0 });
			if (Sections.size() > 1)
				Sections[Sections.size() - 2].End = i;
		}
	}
	if (!Sections.empty())
		Sections.back().End = Lines.size();

	// 첫 섹션 이전의 루트 키-값 파싱 (SceneVersion, ActorCount)
	size_t FirstSectionLine = Sections.empty() ? Lines.size() : Sections[0].Start - 1;
	auto RootProps = ParseSection(Lines, 0, FirstSectionLine);

	int ActorCount = 0;
	{
		auto it = RootProps.find("ActorCount");
		if (it != RootProps.end()) ActorCount = std::stoi(it->second);
	}

	// 헤더 문자열 → 섹션 인덱스 맵 구성
	std::unordered_map<std::string, size_t> SectionMap;
	for (size_t i = 0; i < Sections.size(); ++i)
		SectionMap[Sections[i].Header] = i;

	// 액터 복원
	for (int ai = 0; ai < ActorCount; ++ai)
	{
		std::string ActorHeader = "Actor:" + std::to_string(ai);
		auto it = SectionMap.find(ActorHeader);
		if (it == SectionMap.end()) continue;

		const auto& ASec = Sections[it->second];
		auto AProps = ParseSection(Lines, ASec.Start, ASec.End);

		std::string TypeName = "CActor";
		std::string ActorName, Tag;
		FVector3 WorldPos(0,0,0), WorldScale(1,1,1), WorldRot(0,0,0);
		int SceneCompCount = 0, ActorCompCount = 0;

		{
			auto i2 = AProps.find("TypeName"); if (i2 != AProps.end()) TypeName = i2->second;
		}
		{
			auto i2 = AProps.find("Name"); if (i2 != AProps.end()) ActorName = i2->second;
		}
		{
			auto i2 = AProps.find("Tag"); if (i2 != AProps.end()) Tag = i2->second;
		}
		{
			auto i2 = AProps.find("WorldPos");
			if (i2 != AProps.end())
				sscanf_s(i2->second.c_str(), "%f %f %f", &WorldPos.x, &WorldPos.y, &WorldPos.z);
		}
		{
			auto i2 = AProps.find("WorldScale");
			if (i2 != AProps.end())
				sscanf_s(i2->second.c_str(), "%f %f %f", &WorldScale.x, &WorldScale.y, &WorldScale.z);
		}
		{
			auto i2 = AProps.find("WorldRot");
			if (i2 != AProps.end())
				sscanf_s(i2->second.c_str(), "%f %f %f", &WorldRot.x, &WorldRot.y, &WorldRot.z);
		}
		{
			auto i2 = AProps.find("SceneCompCount"); if (i2 != AProps.end()) SceneCompCount = std::stoi(i2->second);
		}
		{
			auto i2 = AProps.find("ActorCompCount"); if (i2 != AProps.end()) ActorCompCount = std::stoi(i2->second);
		}

		// 팩토리에서 액터 생성 (등록되지 않은 타입은 기본 CActor로 대체)
		std::shared_ptr<CActor> Actor;
		auto FactIt = sActorFactoryMap.find(TypeName);
		if (FactIt != sActorFactoryMap.end())
			Actor = FactIt->second();
		else
			Actor = std::make_shared<CActor>();

		Actor->SetName(ActorName);
		Actor->SetWorld(GetThisPtr());
		Actor->SetActorTag(Tag);

		if (!Actor->Init())
			continue;

		// generic CActor만 파일에서 컴포넌트를 복원 (서브클래스는 Init()에서 직접 생성)
		if (TypeName == "CActor")
		{
			for (int ci = 0; ci < SceneCompCount; ++ci)
			{
				std::string CompHeader = "SceneComp:" + std::to_string(ai) + ":" + std::to_string(ci);
				auto cit = SectionMap.find(CompHeader);
				if (cit == SectionMap.end()) continue;

				const auto& CSec = Sections[cit->second];
				auto CProps = ParseSection(Lines, CSec.Start, CSec.End);

				std::string CompType, CompName, Parent;
				{
					auto i2 = CProps.find("TypeName"); if (i2 != CProps.end()) CompType = i2->second;
				}
				{
					auto i2 = CProps.find("Name"); if (i2 != CProps.end()) CompName = i2->second;
				}
				{
					auto i2 = CProps.find("Parent"); if (i2 != CProps.end()) Parent = i2->second;
				}

				auto CFIt = sCompFactoryMap.find(CompType);
				if (CFIt == sCompFactoryMap.end()) continue;

				auto CompObj = CFIt->second();
				auto SceneComp = std::dynamic_pointer_cast<CSceneComponent>(CompObj);
				if (!SceneComp) continue;

				SceneComp->SetName(CompName);
				Actor->LoadSceneComp(SceneComp, Parent);
				SceneComp->Load(CProps);
			}

			for (int ci = 0; ci < ActorCompCount; ++ci)
			{
				std::string CompHeader = "ActorComp:" + std::to_string(ai) + ":" + std::to_string(ci);
				auto cit = SectionMap.find(CompHeader);
				if (cit == SectionMap.end()) continue;

				const auto& CSec = Sections[cit->second];
				auto CProps = ParseSection(Lines, CSec.Start, CSec.End);

				std::string CompType, CompName;
				{
					auto i2 = CProps.find("TypeName"); if (i2 != CProps.end()) CompType = i2->second;
				}
				{
					auto i2 = CProps.find("Name"); if (i2 != CProps.end()) CompName = i2->second;
				}

				auto CFIt = sCompFactoryMap.find(CompType);
				if (CFIt == sCompFactoryMap.end()) continue;

				auto CompObj = CFIt->second();
				auto ActorComp = std::dynamic_pointer_cast<CActorComponent>(CompObj);
				if (!ActorComp) continue;

				ActorComp->SetName(CompName);
				Actor->LoadActorComp(ActorComp);
				ActorComp->Load(CProps);
			}
		}

		// 컴포넌트 구성 완료 후 트랜스폼 적용
		Actor->SetWorldPos(WorldPos);
		Actor->SetWorldScale(WorldScale);
		Actor->SetWorldRotation(WorldRot);

		mActorList.insert(std::make_pair(ActorName, Actor));
	}

	LOG_DEBUG("[World] Scene loaded: %s (%d actors)", FilePath.c_str(), ActorCount);
	return true;
}