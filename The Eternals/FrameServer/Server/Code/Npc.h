#pragma once
#include "Obj.h"
class CNpc :
    public CObj
{
public:
	CNpc();
	virtual ~CNpc();

public:
	virtual DWORD Release();

public:
	float spd;
};
