#include "LogManager.h"
#include "Asset/PathManager.h"

#include <tchar.h>

#define MAX_LINE 1000

CLogManager* CLogManager::mInstance = nullptr;

CLogManager::CLogManager()
{

}

CLogManager::~CLogManager()
{
	mIsRun = false;

	//현재 스레드가 Os에게 빌려온 상태인지 체크하는 함수
	//빌려온 상태라면 true를 반환
	if (mLogThread.joinable())
	{
		//반납을 시도한다.
		//thread가 아직 실행중이라면 끝날때까지 대기했다가
		//끝나면 즉시 스레드를 os에게 반납한다.
		mLogThread.join();
	}

	//최종적으로 파일에 저장해준다.
	FlushLog();

}

bool CLogManager::Init()
{
	mIsRun = true;

	//스레드의 시작위치를 지정해주기 위해
	//함수를 실행해서 분기된 흐름을 실행시켜준다.
	mLogThread = std::thread(&CLogManager::ProcessLog, this);

	//로그 매니저가 실행되면 새로운 파일을 만든다.
	OpenNewFile();

	return true;
}

void CLogManager::FlushLog()
{
	std::lock_guard lock(mMutex);

	bool IsWrite = false;

	while (!mLogger.mLog.empty())
	{
		//로그 큐에 있는 데이터들을 가져와서 출력하고, 파일에 저장한다.
		std::string Message = mLogger.mLog.front();
		mLogger.mLog.pop();

		if (!mFile.is_open())
		{
			continue;
		}

		Message += "\n";

		OutputDebugStringA(Message.c_str());

		mFile << Message;
		++mLineCount;
		IsWrite = true;

		if (mLineCount > MAX_LINE)
		{
			mLineCount = 0;
			OpenNewFile();
		}
	}


	//실시간으로 로그를 저장하기위해 강제적으로 디스크에 저장하도록 해주는 함수
	if (IsWrite)
	{
		mFile.flush();
	}

}

void CLogManager::ProcessLog()
{
	while (mIsRun)
	{
		FlushLog();
	}
}

bool CLogManager::OpenNewFile()
{
	const TCHAR* BasePath = CPathManager::FindPath("Log");

	TCHAR FullPath[MAX_PATH] = {};
	TCHAR FileName[MAX_PATH] = {};

	//현재 시간을 가져온다.
	SYSTEMTIME Time;
	GetLocalTime(&Time);

	_stprintf_s(FileName,MAX_PATH,TEXT("Log_%04d_%02d_%02d_%02d_%02d_%02d.txt"),
		Time.wYear, Time.wMonth,Time.wDay, Time.wHour, Time.wMinute, Time.wSecond);

	lstrcpy(FullPath, BasePath);
	lstrcat(FullPath, FileName);


	if (mFile.is_open())
	{
		mFile.close();
	}

	mFile.open(FullPath, std::ios::out | std::ios::app);

	return true;
}