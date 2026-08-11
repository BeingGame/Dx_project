#pragma once
#include "GraphicShader.h"
class CShaderMaterial :
    public CGraphicShader
{
public:
	CShaderMaterial();
	virtual ~CShaderMaterial();

public:
	virtual bool Init(const std::string& PathName);
};

