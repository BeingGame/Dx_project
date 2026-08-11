#pragma once

#include "EngineInfo.h"
#include "Asset/PathManager.h"

#include <fstream>
#include <sstream>

class CSVReader
{
public:
	//2차원 배열로 모든 CSV의 텍스트 데이터를 읽어오는 함수
	static std::vector<std::vector<std::wstring>> ReadCSVString(const TCHAR* FileName, const std::string& PathName = "Asset")
	{
		const TCHAR* BasePath = CPathManager::FindPath(PathName);

		TCHAR FullPath[MAX_PATH] = {};
		lstrcpy(FullPath, BasePath);
		lstrcat(FullPath, FileName);

		//C++ 파일 입력 스트림
		std::wifstream File(FullPath);

		if (!File.is_open())
		{
			return std::vector<std::vector<std::wstring>>();
		}

		std::vector<std::vector<std::wstring>> ResultList;
		std::wstring Line;
		bool FirstHeader = true;

		while (std::getline(File, Line))
		{
			if (FirstHeader)
			{
				FirstHeader = false;
				continue;
			}

			if (!Line.empty() && Line.back() == L'\r')
			{
				Line.pop_back();
			}

			if (Line.empty())
			{
				continue;
			}

			std::vector<std::wstring> TokenArray;
			std::wstringstream LineStream(Line);
			std::wstring Cell;

			//구분자 쉼표를 이용해 라인의 한 단어씩 가져온다.
			while (std::getline(LineStream, Cell, L','))
			{
				TokenArray.push_back(Cell);
			}


			ResultList.push_back(TokenArray);
		}

		return ResultList;
	}


	//템플릿을 기반으로 데이터 구조체를 로드하는 함수
	template<typename T>
	static std::vector<T> ReadData(const TCHAR* FileName, const std::string& PathName = "Asset")
	{
		const TCHAR* BasePath = CPathManager::FindPath(PathName);

		TCHAR FullPath[MAX_PATH] = {};
		lstrcpy(FullPath, BasePath);
		lstrcat(FullPath, FileName);

		//C++ 파일 입력 스트림
		std::wifstream File(FullPath);

		if (!File.is_open())
		{
			return std::vector<T>();
		}

		std::vector<T> ResultList;
		std::wstring Line;
		bool FirstHeader = true;

		while (std::getline(File, Line))
		{
			if (FirstHeader)
			{
				FirstHeader = false;
				continue;
			}

			if (!Line.empty() && Line.back() == L'\r')
			{
				Line.pop_back();
			}

			if (Line.empty())
			{
				continue;
			}

			std::vector<std::wstring> TokenArray;
			std::wstringstream LineStream(Line);
			std::wstring Cell;

			//구분자 쉼표를 이용해 라인의 한 단어씩 가져온다.
			while (std::getline(LineStream, Cell, L','))
			{
				TokenArray.push_back(Cell);
			}

			T Data;

			if (!Data.Init(TokenArray))
			{
				return std::vector<T>();
			}

			ResultList.push_back(Data);
		}

		return ResultList;
	}
};

