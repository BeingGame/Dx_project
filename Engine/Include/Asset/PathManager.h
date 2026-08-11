#pragma once

#include "../EngineInfo.h"

class CPathManager
{
private:
	static std::unordered_map<std::string, TCHAR*> mPathMap;

public:
	static bool Init();
	//경로 정리 함수
	static void Clear();
	static bool CreatePath(const std::string& Name, const TCHAR* Path, const std::string& BasePathName = "Root");
	static const TCHAR* FindPath(const std::string& Name);
	static const char* FindPathMultiByte(const std::string& Name);
};

