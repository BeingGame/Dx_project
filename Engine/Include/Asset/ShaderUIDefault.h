#pragma once

#include "GraphicShader.h"

class CShaderUIDefault :
    public CGraphicShader
{
public:
	CShaderUIDefault();
	virtual ~CShaderUIDefault();

public:
	virtual bool Init(const std::string& PathName);
};

