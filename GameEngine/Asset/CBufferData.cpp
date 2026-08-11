#include "CBufferData.h"

#include "AssetManager.h"
#include "ShaderManager.h"

CCBufferData::CCBufferData()
{}

CCBufferData::CCBufferData(const CCBufferData & src)
{
	mBuffer = src.mBuffer;
}

CCBufferData::CCBufferData(CCBufferData && src) noexcept
{
	mBuffer = src.mBuffer;

	src.mBuffer.reset();
}

CCBufferData::~CCBufferData()
{
}

void CCBufferData::SetConstantBuffer(const std::string & Name)
{
	auto ShaderManager = CAssetManager::GetInst()->GetSubManager<CShaderManager>(EAssetType::Shader);

	if (ShaderManager)
	{
		auto CBuffer = ShaderManager->FindCBuffer(Name);

		if (!CBuffer.expired())
		{
			mBuffer = CBuffer.lock();
		}
	}
}
