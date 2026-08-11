#pragma once

#include "EngineInfo.h"

#include <sstream>

//로그 타입별로 메세지를 보낼수 있도록 열거형 설정
enum class ELogType
{
	Debug,
	Warning,
	Error,
	Fatal,
	End
};

class CLogger
{
	friend class CLogManager;

private:
	std::queue<std::string> mLog;

public:
	//가변인자 템플릿
	//인자를 정하지 않고, 여러가지인자를 받을수 있도록 템플릿을 만들어준다.
	//선언하는 방법은 템플릿 타입 앞에 ...를 붙여준다.

	//로깅 함수에서 로그를 타입별로 나눠준다.
	template<typename T, typename ...Args>
	void Logging(ELogType Type, const T& Data, const Args&... args)
	{
		switch (Type)
		{
		case ELogType::Debug:
			WriteLog("Debug", Data, args...);
			break;
		case ELogType::Warning:
			WriteLog("Warning", Data, args...);
			break;
		case ELogType::Error:
			WriteLog("Error", Data, args...);
			break;
		case ELogType::Fatal:
			WriteLog("Fatal", Data, args...);
			break;
		case ELogType::End:
			break;
		default:
			break;
		}
	}

	//로그를 큐에 저장하는 함수
	template<typename T, typename ...Args>
	void WriteLog(const std::string& LogTypeString, const T& Data, const Args&... args)
	{
		SYSTEMTIME Time;
		GetLocalTime(&Time);

		char TimeText[128] = {};

		sprintf_s(TimeText, "[%04d.%02d.%02d.%02d.%02d.%02d]",
			Time.wYear, Time.wMonth, Time.wDay, Time.wHour, Time.wMinute, Time.wSecond);

		std::ostringstream Stream;
		Stream << TimeText;
		Stream << LogTypeString;
		Stream << " : ";

		//매개변수에 들어온 데이트들을 Stream 객체에 넣어준다.
		WriteLog(Stream, Data, args...);

		mLog.push(Stream.str());
	}

	//폴드 표현식
	//C++17버전 이상부터 사용가능
	//간단하게 말하면 복사 붙여넣기
	template<typename T, typename ...Args>
	void WriteLog(std::ostringstream& os, const T& Data, const Args&... args)
	{
		os << Data;

		//폴드 표현식
		//((우리가 필요한 식),...);
		((os << args), ...);

		//ex) Test, Test1 ,Test2;
		//((Test), (Test1), (Test2));
	}




};

