#pragma once

#include "GraphicShader.h"

class CShaderTileMap :
    public CGraphicShader
{
public:
	CShaderTileMap();
	virtual ~CShaderTileMap();

public:
	virtual bool Init(const std::string& PathName);
};

