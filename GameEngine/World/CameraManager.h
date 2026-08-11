#pragma once

#include "../EngineInfo.h"

//카메라 매니저는 싱글톤이 아닌
//월드에서 가지고있는 하나의 객체로 사용된다.

class CCameraManager
{
public:
	CCameraManager();
	~CCameraManager();

private:
	std::unordered_map<std::string, std::weak_ptr<class CCameraComponent>> mCameraMap;
	std::weak_ptr<class CCameraComponent> mMainCamera;

public:
	const FMatrix& GetViewMat() const;
	const FMatrix& GetProjMat() const;

	const FVector3& GetMainCameraPos() const;

	std::weak_ptr<class CCameraComponent> GetMainCamera() const
	{
		return mMainCamera;
	}

public:
	bool Init();
	void Update(float DeltaTime);

	void AddCamera(std::string Name, const std::weak_ptr<class CCameraComponent>& Camera);
	void ChangeMainCamera(const std::weak_ptr<class CCameraComponent>& Camera);
	void ChangeMainCamera(const std::string& Name);
};

