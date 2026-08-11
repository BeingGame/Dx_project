#pragma once

#include "Logger.h"

#include <fstream>

#include <thread>
#include <mutex>

class CLogManager
{
	Singleton(CLogManager);

private:

	//스레드
	//프로세스 내부에서 실제로 작업하고있는 주체
	//작업을 하나만 할수있는게 아니라 여러개가 동시에 실행할수 있다.
	//스레드 여러개를 동작시키는 것, 멀티 스레드
	//스레드끼리 교환할때 컨텍스트 스위칭(문맥 교환) 비용이 좀 들어서
	//많은 스레드끼리 사용하는건 오히려 좋지 않다.

	//같은 프로세스내의 스레드는 메모리를 공유한다.

	//ex) Num1 = 2;
	// thread1 -> Num1 += 10;
	// thread2 -> Num1 += 20;

	//경쟁 상태(race condition)
	//thread1 실행되고 -> thread2 실행 -> thread2가 먼저 종료 Num1 == 22 -> thread1 종료 -> Num1 == 12

	//lock걸어서 현재 lock건 스레드만 데이터를 사용할수 있도록 해주는 것
	//뮤텍스
	CLogger mLogger;
	bool mIsRun = false;

	//로그의 최대 길이를 정해주기 위함
	int mLineCount = 0;

	std::ofstream mFile;

	std::thread mLogThread;

	std::mutex mMutex;

public:
	bool Init();
	void FlushLog();
	void ProcessLog();

private:
	bool OpenNewFile();


public:
	template<typename T, typename ...Args>
	void Debug(const T& Data, const Args&... args)
	{
		std::lock_guard lock(mMutex);

		mLogger.Logging(ELogType::Debug, Data, args...);
	}

	template<typename T, typename ...Args>
	void Warning(const T& Data, const Args&... args)
	{
		std::lock_guard lock(mMutex);

		mLogger.Logging(ELogType::Warning, Data, args...);
	}

	template<typename T, typename ...Args>
	void Error(const T& Data, const Args&... args)
	{
		std::lock_guard lock(mMutex);

		mLogger.Logging(ELogType::Error, Data, args...);
	}

	template<typename T, typename ...Args>
	void Fatal(const T& Data, const Args&... args)
	{
		std::lock_guard lock(mMutex);

		mLogger.Logging(ELogType::Fatal, Data, args...);
	}




};

