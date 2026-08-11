#pragma once

#define Singleton(ClassName) \
private:\
ClassName();\
~ClassName();\
ClassName(const ClassName&) = delete;\
ClassName(ClassName&&) = delete;\
ClassName& operator=(const ClassName&) = delete;\
ClassName& operator=(ClassName&&) = delete;\
static ClassName* mInstance;\
public:\
static ClassName* GetInst()\
{\
if(!mInstance)\
{\
 mInstance = new ClassName();\
\
}\
return mInstance;\
}\
static void DestroyInst()\
{\
 SAFE_DELETE(mInstance)\
}\
\

#define SAFE_DELETE(ptr) if(ptr) { delete ptr ; ptr = nullptr;}

//로그매니저 매크로 생성
#define LOG_DEBUG(Data, ...) CLogManager::GetInst()->Debug(Data, ##__VA_ARGS__)
#define LOG_WARNING(Data, ...) CLogManager::GetInst()->Warning(Data, ##__VA_ARGS__)
#define LOG_ERROR(Data, ...) CLogManager::GetInst()->Error(Data, ##__VA_ARGS__)
#define LOG_FATAL(Data, ...) CLogManager::GetInst()->Fatal(Data, ##__VA_ARGS__)

