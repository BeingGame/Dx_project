#pragma once

#include "GraphicShader.h"

class CShaderColor2D :
    public CGraphicShader
{
public:
	CShaderColor2D();
	virtual ~CShaderColor2D();

public:
    virtual bool Init(const std::string& PathName);
};

