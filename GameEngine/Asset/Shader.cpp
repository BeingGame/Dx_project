#include "Shader.h"

CShader::CShader()
{}

CShader::CShader(const CShader & src)
	:CAsset(src)
{}

CShader::CShader(CShader && src) noexcept
	:CAsset(std::move(src))
{}

CShader::~CShader()
{}
