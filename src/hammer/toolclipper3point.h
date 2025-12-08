//// SLE NEW - 3-point clipping tool, a subclass of normal Clipper

#ifndef CLIPPER3POINT_H
#define CLIPPER3POINT_H
#ifdef _WIN32
#pragma once
#endif

#include "ToolClipper.h"

class Clipper3Point : public Clipper3D
{
public:    
//	Clipper3Point() 
//	{
//	}
//	~Clipper3Point() {}

	virtual bool Is3PointClipper(void) {return true;}

	virtual ToolID_t GetToolID(void) { return TOOL_CLIPPER_3POINT; }
};

#endif // CLIPPER3POINT_H
