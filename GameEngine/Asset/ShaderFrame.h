#pragma once
#include "GraphicShader.h"
class CShaderFrame :
    public CGraphicShader
{
public:
	CShaderFrame();
	virtual ~CShaderFrame();

public:
	virtual bool Init(const std::string& PathName);
};

