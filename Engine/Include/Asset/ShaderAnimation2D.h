#pragma once

#include "GraphicShader.h"

class CShaderAnimation2D :
    public CGraphicShader
{
public:
	CShaderAnimation2D();
	virtual ~CShaderAnimation2D();

public:
	virtual bool Init(const std::string& PathName);
};

