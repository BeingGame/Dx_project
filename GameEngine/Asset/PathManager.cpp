#include "PathManager.h"

std::unordered_map<std::string, TCHAR*> CPathManager::mPathMap;

bool CPathManager::Init()
{
	//루트 패스를 만들어준다.
	//먼저 실행파일 있는 경로(출력 디렉토리)를 가져와서 실행파일의 이름을 제거한다.
	TCHAR* Path = new TCHAR[MAX_PATH];

	//GetModuleFileName 함수
	//현재 출력 디렉토리에 있는 실행파일까지의 경로를 가져오는 함수
	//다만 실행파일까지 포함되기 때문에 ../Binary/*.exe 형태로 가져오게된다.
	GetModuleFileName(nullptr, Path, MAX_PATH);

	// 디렉토리의 기호가 \\ 혹은 /가 나오는 위치까지 찾은뒤 0으로 변경해준다.
	//따라서 역순으로 경로를 따라가면서 실행파일의 이름을 제거하게 된다.

	int Length = lstrlen(Path);

	for (int i = Length - 1; i >= 0; --i)
	{
		if (Path[i] == '\\' || Path[i] == '/')
		{
			//조건이 알맞으면 i + 1 만큼의 위치부터, Length -1의 위치까지 0을 넣어준다.
			//Length - i - 1인 이유는 길이에서 i만큼 위치를 이동했기 때문
			memset(&Path[i + 1], 0, sizeof(TCHAR) * (Length - i - 1));
			break;
		}
	}

	mPathMap.insert(std::make_pair("Root", Path));


	//엔진의 경로
	CreatePath("EngineAsset", TEXT("EngineAsset\\"));
	CreatePath("EngineShader", TEXT("Shader\\"), "EngineAsset");
	CreatePath("EngineFont", TEXT("Font\\"), "EngineAsset");

	//클라이언트의 경로
	CreatePath("Asset", TEXT("Asset\\"));
	CreatePath("Texture", TEXT("Texture\\"), "Asset");
	CreatePath("Sound", TEXT("Sound\\"), "Asset");

	CreatePath("Log", TEXT("Log\\"));


	return true;
}

void CPathManager::Clear()
{
	auto iter = mPathMap.begin();
	auto iterEnd = mPathMap.end();

	for (; iter != iterEnd; ++iter)
	{
		delete[] iter->second;
	}
}

bool CPathManager::CreatePath(const std::string& Name, const TCHAR* Path, const std::string& BasePathName)
{
	//이미 존재하는 Path는 더이상 만들필요가 없기 때문에 false를 반환한다.
	if (FindPath(Name))
	{
		return false;
	}

	const TCHAR* BasePath = FindPath(BasePathName);

	//베이스가 될 Path가 없으면 false를 반환한다.
	if (!BasePath)
	{
		return false;
	}

	TCHAR* FullPath = new TCHAR[MAX_PATH];

	//0으로 초기화
	memset(FullPath, 0, sizeof(TCHAR) * MAX_PATH);

	//FullPath에 BasePath를 복사한다.
	//첫번째 인자가 복사할 위치, 두번째 인자가 복사할 문자열
	lstrcpy(FullPath, BasePath);

	//FullPath에 저장된 기본경로뒤에 Path를 붙여준다.
	lstrcat(FullPath, Path);

	mPathMap.insert(std::make_pair(Name, FullPath));

	return true;
}

const TCHAR* CPathManager::FindPath(const std::string& Name)
{
	auto iter = mPathMap.find(Name);

	if (iter == mPathMap.end())
	{
		return nullptr;
	}

	return iter->second;
}

const char* CPathManager::FindPathMultiByte(const std::string& Name)
{
	auto iter = mPathMap.find(Name);

	if (iter == mPathMap.end())
	{
		return nullptr;
	}

	//Path 문자열을 반환하기 위해서 static을 사용한다.
	static char Path[MAX_PATH] = {};

	//Path 0으로 초기화
	memset(Path, 0, MAX_PATH);

	int Count = WideCharToMultiByte(CP_ACP, 0, iter->second, -1, nullptr, 0, nullptr, nullptr);
	WideCharToMultiByte(CP_ACP, 0, iter->second, -1, Path, Count, nullptr, nullptr);

	return Path;
}
