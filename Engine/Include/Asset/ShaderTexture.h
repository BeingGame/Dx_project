#pragma once
#include "GraphicShader.h"
class CShaderTexture :
    public CGraphicShader
{
public:
	CShaderTexture();
	virtual ~CShaderTexture();

public:
	virtual bool Init(const std::string& PathName);
};

