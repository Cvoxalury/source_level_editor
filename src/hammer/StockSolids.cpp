//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#include "stdafx.h"
#include "hammer.h"
#include "StockSolids.h"
#include "hammer_mathlib.h"
#include "MapSolid.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#pragma warning(disable:4244)

//Vector pmPoints[64];

StockSolid::StockSolid(int nFields)
{
	AllocateDataFields(nFields);
	cofs.Init();
}

StockSolid::~StockSolid()
{
	if ( pFields )
	{
		delete[] pFields;
		pFields = NULL;
	}
}

void StockSolid::AllocateDataFields(int nFields_)
{
	pFields = new STSDATAFIELD[nFields_];
	Assert(pFields);
	iMaxFields = nFields_;
	this->nFields = 0;	// none yet
}

void StockSolid::Serialize(std::fstream& file, BOOL bIsStoring)
{
}

int StockSolid::GetFieldCount() const
{
	return nFields;
}

void StockSolid::SetFieldData(int iIndex, int iData)
{
	Assert(iIndex < nFields);

	STSDATAFIELD& field = pFields[iIndex];
	field.iValue = iData;

	if(field.flags & DFFLAG_RANGED)
	{
		Assert(!(iData < field.iRangeLower || iData > field.iRangeUpper));
	}
}

int StockSolid::GetFieldData(int iIndex, int *piData) const
{
	Assert(iIndex < nFields);

	STSDATAFIELD& field = pFields[iIndex];

	if(piData)
		piData[0] = field.iValue;

	return field.iValue;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void StockSolid::SetOrigin(const Vector &o)
{
	origin = o;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void StockSolid::SetCenterOffset(const Vector &ofs)
{
	cofs = ofs;
}

void StockSolid::AddDataField(STSDF_TYPE type, const char *pszName, int iRangeLower, int iRangeUpper)
{
	Assert(nFields < iMaxFields);
	
	STSDATAFIELD& field = pFields[nFields++];

	field.type = type;
	field.flags = 0;
	strcpy(field.szName, pszName);

	if(iRangeLower != -1)
	{
		field.flags |= DFFLAG_RANGED;
		field.iRangeLower = iRangeLower;
		field.iRangeUpper = iRangeUpper;
	}
}

// ----------------------------------------------------------------------------
// StockBlock()
// ----------------------------------------------------------------------------
StockBlock::StockBlock() :
	StockSolid(3)
{
	AddDataField(DFTYPE_INTEGER, "Width (X)");
	AddDataField(DFTYPE_INTEGER, "Depth (Y)");
	AddDataField(DFTYPE_INTEGER, "Height (Z)");
}

void StockBlock::SetFromBox(BoundBox *pBox)
{
	// round floats before converting to integers
	SetFieldData(fieldWidth, (pBox->bmaxs[0] - pBox->bmins[0])+0.5f );
	SetFieldData(fieldDepth, (pBox->bmaxs[1] - pBox->bmins[1])+0.5f );
	SetFieldData(fieldHeight, (pBox->bmaxs[2] - pBox->bmins[2])+0.5f );

	Vector o;
	pBox->GetBoundsCenter(o);

	SetOrigin(o);
}

void StockBlock::CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eAlignment)
{
	CMapFace Face;

	float fDepth = float(GetFieldData(fieldDepth))/2;
	float fWidth = float(GetFieldData(fieldWidth))/2;
	float fHeight = float(GetFieldData(fieldHeight))/2;

	// create box
	Vector bmins, bmaxs;
	bmins[0] = origin[0] - fWidth + cofs[0];
	bmins[1] = origin[1] - fDepth + cofs[1];
	bmins[2] = origin[2] - fHeight + cofs[2];

	bmaxs[0] = origin[0] + fWidth + cofs[0];
	bmaxs[1] = origin[1] + fDepth + cofs[1];
	bmaxs[2] = origin[2] + fHeight + cofs[2];

	Vector Points[4];

	// x planes - top first
	Points[0][0] = bmins[0];
	Points[0][1] = bmaxs[1];
	Points[0][2] = bmaxs[2];

	Points[1][0] = bmaxs[0];
	Points[1][1] = bmaxs[1];
	Points[1][2] = bmaxs[2];

	Points[2][0] = bmaxs[0];
	Points[2][1] = bmins[1];
	Points[2][2] = bmaxs[2];
	
	Points[3][0] = bmins[0];
	Points[3][1] = bmins[1];
	Points[3][2] = bmaxs[2];

	Face.CreateFace(Points, 4, pSolid->IsCordonBrush());
	pSolid->AddFace(&Face);

	// top - modify heights
	for(int i = 0; i < 4; i++)
	{
		Points[i][2] = bmins[2];
	}

	Face.CreateFace(Points, -4, pSolid->IsCordonBrush());
	pSolid->AddFace(&Face);

	// y planes - left
	Points[0][0] = bmins[0];
	Points[0][1] = bmaxs[1];
	Points[0][2] = bmaxs[2];

	Points[1][0] = bmins[0];
	Points[1][1] = bmins[1];
	Points[1][2] = bmaxs[2];

	Points[2][0] = bmins[0];
	Points[2][1] = bmins[1];
	Points[2][2] = bmins[2];

	Points[3][0] = bmins[0];
	Points[3][1] = bmaxs[1];
	Points[3][2] = bmins[2];

	Face.CreateFace(Points, 4, pSolid->IsCordonBrush());
	pSolid->AddFace(&Face);

	// right - modify xloc
	for(int i = 0; i < 4; i++)
	{
		Points[i][0] = bmaxs[0];
	}

	Face.CreateFace(Points, -4, pSolid->IsCordonBrush());
	pSolid->AddFace(&Face);

	// x planes - farthest
	Points[0][0] = bmaxs[0];
	Points[0][1] = bmaxs[1];
	Points[0][2] = bmaxs[2];

	Points[1][0] = bmins[0];
	Points[1][1] = bmaxs[1];
	Points[1][2] = bmaxs[2];

	Points[2][0] = bmins[0];
	Points[2][1] = bmaxs[1];
	Points[2][2] = bmins[2];

	Points[3][0] = bmaxs[0];
	Points[3][1] = bmaxs[1];
	Points[3][2] = bmins[2];

	Face.CreateFace(Points, 4, pSolid->IsCordonBrush());
	pSolid->AddFace(&Face);

	// nearest - modify yloc
	for(int i = 0; i < 4; i++)
	{
		Points[i][1] = bmins[1];
	}

	Face.CreateFace(Points, -4, pSolid->IsCordonBrush());
	pSolid->AddFace(&Face);

	pSolid->CalcBounds();
	pSolid->InitializeTextureAxes(eAlignment, INIT_TEXTURE_ALL | INIT_TEXTURE_FORCE);
}

// ----------------------------------------------------------------------------
// StockWedge()
// ----------------------------------------------------------------------------
StockWedge::StockWedge() :
	StockSolid(3)
{
	AddDataField(DFTYPE_INTEGER, "Width (X)");
	AddDataField(DFTYPE_INTEGER, "Depth (Y)");
	AddDataField(DFTYPE_INTEGER, "Height (Z)");
#ifdef SLE //// SLE NEW - set start angle for block tool
	AddDataField(DFTYPE_INTEGER, "Start Angle");
	SetFieldData(fieldStartAngle, 0);
#endif
}

void StockWedge::SetFromBox(BoundBox *pBox)
{
	SetFieldData(fieldWidth, pBox->bmaxs[0] - pBox->bmins[0]);
	SetFieldData(fieldDepth, pBox->bmaxs[1] - pBox->bmins[1]);
	SetFieldData(fieldHeight, pBox->bmaxs[2] - pBox->bmins[2]);

	Vector o;
	pBox->GetBoundsCenter(o);

	SetOrigin(o);
}

void StockWedge::CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment)
{
	CMapFace Face;

	float fDepth = float(GetFieldData(fieldDepth))/2;
	float fWidth = float(GetFieldData(fieldWidth))/2;
	float fHeight = float(GetFieldData(fieldHeight))/2;
	
#ifdef SLE //// SLE NEW - set start angle for block tool
//	int startAngle = GetFieldData(fieldStartAngle);
#endif

	Vector Points[4];

	// x planes - top
	Points[0][0] = origin[0] + fWidth;
	Points[0][1] = origin[1] + fDepth;
	Points[0][2] = origin[2] + fHeight;

	Points[1][0] = origin[0] + fWidth;
	Points[1][1] = origin[1] - fDepth;
	Points[1][2] = origin[2] + fHeight;

	Points[2][0] = origin[0] - fWidth;
	Points[2][1] = origin[1] - fDepth;
	Points[2][2] = origin[2] + fHeight;

	Face.CreateFace(Points, 3);
	pSolid->AddFace(&Face);

	// bottom
	for (int i = 0; i < 3; i++)
	{
		Points[i][2] = origin[2] - fHeight;
	}

	Face.CreateFace(Points, -3);
	pSolid->AddFace(&Face);

	// left (slant)
	Points[0][0] = origin[0] + fWidth;
	Points[0][1] = origin[1] + fDepth;
	Points[0][2] = origin[2] - fHeight;

	Points[1][0] = origin[0] + fWidth;
	Points[1][1] = origin[1] + fDepth;
	Points[1][2] = origin[2] + fHeight;

	Points[2][0] = origin[0] - fWidth;
	Points[2][1] = origin[1] - fDepth;
	Points[2][2] = origin[2] + fHeight;
	
	Points[3][0] = origin[0] - fWidth;
	Points[3][1] = origin[1] - fDepth;
	Points[3][2] = origin[2] - fHeight;

	Face.CreateFace(Points, 4);
	pSolid->AddFace(&Face);

	// south
	Points[0][0] = origin[0] + fWidth;
	Points[0][1] = origin[1] - fDepth;
	Points[0][2] = origin[2] + fHeight;

	Points[1][0] = origin[0] + fWidth;
	Points[1][1] = origin[1] - fDepth;
	Points[1][2] = origin[2] - fHeight;

	Points[2][0] = origin[0] - fWidth;
	Points[2][1] = origin[1] - fDepth;
	Points[2][2] = origin[2] - fHeight;
	
	Points[3][0] = origin[0] - fWidth;
	Points[3][1] = origin[1] - fDepth;
	Points[3][2] = origin[2] + fHeight;

	Face.CreateFace(Points, 4);
	pSolid->AddFace(&Face);

	// right
	Points[0][0] = origin[0] + fWidth;
	Points[0][1] = origin[1] + fDepth;
	Points[0][2] = origin[2] + fHeight;

	Points[1][0] = origin[0] + fWidth;
	Points[1][1] = origin[1] + fDepth;
	Points[1][2] = origin[2] - fHeight;

	Points[2][0] = origin[0] + fWidth;
	Points[2][1] = origin[1] - fDepth;
	Points[2][2] = origin[2] - fHeight;
	
	Points[3][0] = origin[0] + fWidth;
	Points[3][1] = origin[1] - fDepth;
	Points[3][2] = origin[2] + fHeight;

	Face.CreateFace(Points, 4);
	pSolid->AddFace(&Face);

	pSolid->CalcBounds();
	pSolid->InitializeTextureAxes(eTextureAlignment, INIT_TEXTURE_ALL | INIT_TEXTURE_FORCE);
}

// ----------------------------------------------------------------------------
// StockCylinder()
// ----------------------------------------------------------------------------
StockCylinder::StockCylinder()
#ifdef SLE //// SLE NEW - set start angle for block tool
	: StockSolid(5)
#else
	: StockSolid(4)
#endif
{
	AddDataField(DFTYPE_INTEGER, "Width (X)");
	AddDataField(DFTYPE_INTEGER, "Depth (Y)");
	AddDataField(DFTYPE_INTEGER, "Height (Z)");
	AddDataField(DFTYPE_INTEGER, "Number of Sides");
	SetFieldData(fieldSideCount, 8);
#ifdef SLE //// SLE NEW - set start angle for block tool
	AddDataField(DFTYPE_INTEGER, "Start Angle");
	SetFieldData(fieldStartAngle, 0);
#endif
}

void StockCylinder::SetFromBox(BoundBox *pBox)
{
	SetFieldData(fieldWidth, pBox->bmaxs[0] - pBox->bmins[0]);
	SetFieldData(fieldDepth, pBox->bmaxs[1] - pBox->bmins[1]);
	SetFieldData(fieldHeight, pBox->bmaxs[2] - pBox->bmins[2]);

	Vector o;
	pBox->GetBoundsCenter(o);

	SetOrigin(o);
}

void StockCylinder::CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment)
{
	CMapFace Face;

	float fDepth = float(GetFieldData(fieldDepth))/2;
	float fWidth = float(GetFieldData(fieldWidth))/2;
	float fHeight = float(GetFieldData(fieldHeight))/2;
	int nSides = GetFieldData(fieldSideCount);
	
#ifdef SLE //// SLE NEW - set start angle for block tool
	int startAngle = GetFieldData(fieldStartAngle);
#endif

	Vector pmPoints[64];
#ifdef SLE
	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, nSides, (float)startAngle, pmPoints );
#else
	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, nSides, 0, pmPoints );
#endif
	// face 0 - top face
	for(int i = 0; i < nSides+1; i++)
	{
		pmPoints[i][2] = origin[2] - fHeight;
	}

	Face.CreateFace( pmPoints, -nSides);
	pSolid->AddFace(&Face);

	// bottom face
	for(int i = 0; i < nSides+1; i++)
	{
		pmPoints[i][2] = origin[2] + fHeight;
	}

	Face.CreateFace( pmPoints, nSides);
	pSolid->AddFace(&Face);

	// other sides
	Vector Points[4];

	for(int i = 0; i < nSides; i++)
	{
		Points[0][0] = pmPoints[i][0];
		Points[0][1] = pmPoints[i][1];
		Points[0][2] = origin[2] - fHeight;

		Points[1][0] = pmPoints[i+1][0];
		Points[1][1] = pmPoints[i+1][1];
		Points[1][2] = origin[2] - fHeight;

		Points[2][0] = pmPoints[i+1][0];
		Points[2][1] = pmPoints[i+1][1];
		Points[2][2] = origin[2] + fHeight;

		Points[3][0] = pmPoints[i][0];
		Points[3][1] = pmPoints[i][1];
		Points[3][2] = origin[2] + fHeight;

		Face.CreateFace(Points, 4);
		Face.texture.smooth = 1.f;
		pSolid->AddFace(&Face);
	}

	pSolid->CalcBounds();
	pSolid->InitializeTextureAxes(eTextureAlignment, INIT_TEXTURE_ALL | INIT_TEXTURE_FORCE);
}

// ----------------------------------------------------------------------------
// StockPyramid()
// ----------------------------------------------------------------------------
StockPyramid::StockPyramid()
#ifdef SLE //// SLE NEW - set start angle for block tool
	: StockSolid(5)
#else
	: StockSolid(4)
#endif
{
	AddDataField(DFTYPE_INTEGER, "Width (X)");
	AddDataField(DFTYPE_INTEGER, "Depth (Y)");
	AddDataField(DFTYPE_INTEGER, "Height (Z)");
	AddDataField(DFTYPE_INTEGER, "Number of Sides");
	SetFieldData(fieldSideCount, 8);
#ifdef SLE //// SLE NEW - set start angle for block tool
	AddDataField(DFTYPE_INTEGER, "Start Angle");
	SetFieldData(fieldStartAngle, 0);
#endif
}

void StockPyramid::SetFromBox(BoundBox *pBox)
{
	SetFieldData(fieldWidth, pBox->bmaxs[0] - pBox->bmins[0]);
	SetFieldData(fieldDepth, pBox->bmaxs[1] - pBox->bmins[1]);
	SetFieldData(fieldHeight, pBox->bmaxs[2] - pBox->bmins[2]);

	Vector o;
	pBox->GetBoundsCenter(o);

	SetOrigin(o);
}

void StockPyramid::CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment)
{
	float fDepth = float(GetFieldData(fieldDepth))/2;
	float fWidth = float(GetFieldData(fieldWidth))/2;
	float fHeight = float(GetFieldData(fieldHeight))/2;
	int nSides = GetFieldData(fieldSideCount);
	CMapFace NewFace;
	
#ifdef SLE //// SLE NEW - set start angle for block tool
	int startAngle = GetFieldData(fieldStartAngle);
#endif

	// create bottom poly
	Vector pmPoints[64];
#ifdef SLE
	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, nSides, (float)startAngle, pmPoints);
#else
	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, nSides, 0, pmPoints);
#endif
	// bottom face
	for(int i = 0; i < nSides+1; i++)
	{
		// YWB rounding???
#ifdef SLE //// SLE CHANGE - no rounding for spikes and spheres
		pmPoints[i][2] = (origin[2] - fHeight);
#else
		pmPoints[i][2] = V_rint(origin[2] - fHeight);
#endif
	}

	NewFace.CreateFace(pmPoints, -nSides);
	pSolid->AddFace(&NewFace);

	// other sides
	Vector Points[3];

	// get centerpoint
	Points[0][0] = origin[0];
	Points[0][1] = origin[1];
	// YWB rounding???
#ifdef SLE //// SLE CHANGE - no rounding for spikes and spheres
	Points[0][2] = (origin[2] + fHeight);
#else
	Points[0][2] = V_rint(origin[2] + fHeight);
#endif
	for(int i = 0; i < nSides; i++)
	{
		Points[1][0] = pmPoints[i][0];
		Points[1][1] = pmPoints[i][1];
		Points[1][2] = pmPoints[i][2];

		Points[2][0] = pmPoints[i+1][0];
		Points[2][1] = pmPoints[i+1][1];
		Points[2][2] = pmPoints[i+1][2];

		NewFace.CreateFace(Points, 3);
		pSolid->AddFace(&NewFace);
	}

	pSolid->CalcBounds();
	pSolid->InitializeTextureAxes(eTextureAlignment, INIT_TEXTURE_ALL | INIT_TEXTURE_FORCE);
}

StockSphere::StockSphere()
#ifdef SLE //// SLE NEW - set start angle for block tool
	: StockSolid(5)
#else
	: StockSolid(4)
#endif
{
	AddDataField(DFTYPE_INTEGER, "Width (X)");
	AddDataField(DFTYPE_INTEGER, "Depth (Y)");
	AddDataField(DFTYPE_INTEGER, "Height (Z)");
	AddDataField(DFTYPE_INTEGER, "Subdivisions");
	SetFieldData(fieldSideCount, 8);
#ifdef SLE //// SLE NEW - set start angle for block tool
	AddDataField(DFTYPE_INTEGER, "Start Angle");
	SetFieldData(fieldStartAngle, 0);
#endif
}

void StockSphere::SetFromBox(BoundBox *pBox)
{
	SetFieldData(fieldWidth, pBox->bmaxs[0] - pBox->bmins[0]);
	SetFieldData(fieldDepth, pBox->bmaxs[1] - pBox->bmins[1]);
	SetFieldData(fieldHeight, pBox->bmaxs[2] - pBox->bmins[2]);

	Vector o;
	pBox->GetBoundsCenter(o);

	SetOrigin(o);
}

//-----------------------------------------------------------------------------
// Purpose: Builds a tesselated sphere.
// Input  : pSolid - Pointer to a solid that will become a sphere.
//-----------------------------------------------------------------------------
void StockSphere::CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment)
{
	CMapFace Face;

	float fDepth = (float)GetFieldData(fieldDepth) / 2;
	float fWidth = (float)GetFieldData(fieldWidth) / 2;
	float fHeight = (float)GetFieldData(fieldHeight) / 2;
	int nSides = GetFieldData(fieldSideCount);
	
#ifdef SLE //// SLE NEW - set start angle for block tool
	int startAngle = GetFieldData(fieldStartAngle);
#endif

	float fAngle = 0;
	float fAngleStep = 180.0 / nSides;

	//
	// Build the sphere by building slices at constant angular intervals.
	// 
	// Each slice is a ring of four-sided faces, except for the top and bottom slices,
	// which are flattened cones.
	//
	// Unrolled, a sphere made with 5 'sides' has 25 faces and looks like this:
	//				
	//			/\  /\  /\  /\  /\
	//		   / 0\/ 1\/ 2\/ 3\/ 4\
	//		  |  5|  6|  7|  8|  9| 	
	//		  | 10| 11| 12| 13| 14| 	
	//		  | 15| 16| 17| 18| 19| 	
	//		   \20/\21/\22/\23/\24/
	//			\/  \/  \/  \/  \/
	//
	for (int nSlice = 0; nSlice < nSides; nSlice++)
	{
		float fAngle1 = fAngle + fAngleStep;

		//
		// Make the upper polygon.
		//
		Vector TopPoints[64];
		float fUpperWidth = fWidth * sin(DEG2RAD(fAngle));
		float fUpperDepth = fDepth * sin(DEG2RAD(fAngle));
#ifdef SLE
		polyMake(origin[0] - fUpperWidth, origin[1] - fUpperDepth, 
				 origin[0] + fUpperWidth, origin[1] + fUpperDepth, nSides, (float)startAngle, TopPoints, false);

#else
		polyMake(origin[0] - fUpperWidth, origin[1] - fUpperDepth, 
				 origin[0] + fUpperWidth, origin[1] + fUpperDepth, nSides, 0, TopPoints, false);
#endif
		//
		// Make the lower polygon.
		//
		Vector BottomPoints[64];
		float fLowerWidth = fWidth * sin(DEG2RAD(fAngle1));
		float fLowerDepth = fDepth * sin(DEG2RAD(fAngle1));
#ifdef SLE
		polyMake(origin[0] - fLowerWidth, origin[1] - fLowerDepth, 
				 origin[0] + fLowerWidth, origin[1] + fLowerDepth, nSides, (float)startAngle, BottomPoints, false);
#else
		polyMake(origin[0] - fLowerWidth, origin[1] - fLowerDepth, 
				 origin[0] + fLowerWidth, origin[1] + fLowerDepth, nSides, 0, BottomPoints, false);
#endif
		//
		// Build the faces that connect the upper and lower polygons.
		//
		Vector Points[4];
		float fUpperHeight = origin[2] + fHeight * cos(DEG2RAD(fAngle));
		float fLowerHeight = origin[2] + fHeight * cos(DEG2RAD(fAngle1));

		for (int i = 0; i < nSides; i++)
		{
			if (nSlice != 0)
			{
				Points[0][0] = TopPoints[i + 1][0];
				Points[0][1] = TopPoints[i + 1][1];
				Points[0][2] = fUpperHeight;
			}
			
			Points[1][0] = TopPoints[i][0];
			Points[1][1] = TopPoints[i][1];
			Points[1][2] = fUpperHeight;

			Points[2][0] = BottomPoints[i][0];
			Points[2][1] = BottomPoints[i][1];
			Points[2][2] = fLowerHeight;

			if (nSlice != nSides - 1)
			{
				Points[3][0] = BottomPoints[i + 1][0];
				Points[3][1] = BottomPoints[i + 1][1];
				Points[3][2] = fLowerHeight;
			}

			//
			// Top and bottom are cones, not rings, so remove one vertex per face.
			//
			if (nSlice == 0)
			{
				Face.CreateFace(&Points[1], 3);
			}
			else if (nSlice == nSides - 1)
			{
				Face.CreateFace(Points, 3);
			}
			else
			{
				Face.CreateFace(Points, 4);
			}

			Face.texture.smooth = 1.f;
			pSolid->AddFace(&Face);
		}
	
		fAngle += fAngleStep;
	}

	pSolid->CalcBounds();
	pSolid->InitializeTextureAxes(eTextureAlignment, INIT_TEXTURE_ALL | INIT_TEXTURE_FORCE);
}
#ifdef SLE /// more stock solids
// Antiprism
StockAntiprism::StockAntiprism()
	: StockSolid(5)
{
	AddDataField(DFTYPE_INTEGER, "Width (X)");
	AddDataField(DFTYPE_INTEGER, "Depth (Y)");
	AddDataField(DFTYPE_INTEGER, "Height (Z)");
	SetFieldData(fieldSideCount, 8);
	AddDataField(DFTYPE_INTEGER, "Start Angle");
	SetFieldData(fieldStartAngle, 0);
}

void StockAntiprism::SetFromBox(BoundBox *pBox)
{
	SetFieldData(fieldWidth, pBox->bmaxs[0] - pBox->bmins[0]);
	SetFieldData(fieldDepth, pBox->bmaxs[1] - pBox->bmins[1]);
	SetFieldData(fieldHeight, pBox->bmaxs[2] - pBox->bmins[2]);

	Vector o;
	pBox->GetBoundsCenter(o);

	SetOrigin(o);
}

void StockAntiprism::CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment)
{
	float fDepth = float(GetFieldData(fieldDepth))/2;
	float fWidth = float(GetFieldData(fieldWidth))/2;
	float fHeight = float(GetFieldData(fieldHeight))/2;
	int nSides = GetFieldData(fieldSideCount); // for antiprism, number of faces is the base (top/bottom polygon sides), not total of faces
	int startAngle = GetFieldData(fieldStartAngle);

	CMapFace NewFace;

	Vector pmPointsTop[256];
	Vector pmPointsBot[256];

	// top face
	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, nSides, startAngle, pmPointsTop );

	for(int i = 0; i < nSides + 1; i++)
	{
		pmPointsTop[i][2] = (origin[2] + fHeight);
	}

	if ( nSides > 2 ) // antiprism can exist with 2D polygon at top and bottom, but the editor will need it to become a 4-sided solid
	{
		NewFace.CreateFace(pmPointsTop, nSides);
		pSolid->AddFace(&NewFace);
	}

	// bottom face
	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, nSides, startAngle + (180 / nSides), pmPointsBot );
	for(int i = 0; i < nSides + 1; i++)
	{
		pmPointsBot[i][2] = (origin[2] - fHeight);
	}
	
	if ( nSides > 2 ) // see above
	{
		NewFace.CreateFace(pmPointsBot, -nSides);
		pSolid->AddFace(&NewFace);
	}

	// middle band of triangles
	Vector midPoints[3];
	midPoints[0][2] = pmPointsTop[0][2];
	midPoints[1][2] = pmPointsTop[0][2];
	midPoints[2][2] = pmPointsBot[0][2];
	for ( int i = 0; i < nSides; i++ )
	{
		midPoints[0][0] = pmPointsTop[i+1][0];
		midPoints[0][1] = pmPointsTop[i+1][1]; 
		midPoints[1][0] = pmPointsTop[i][0];
		midPoints[1][1] = pmPointsTop[i][1];
		midPoints[2][0] = pmPointsBot[i][0];
		midPoints[2][1] = pmPointsBot[i][1];

		NewFace.CreateFace(midPoints, 3);
		pSolid->AddFace(&NewFace);
	}	
	midPoints[0][2] = pmPointsBot[0][2];
	midPoints[1][2] = pmPointsBot[0][2];
	midPoints[2][2] = pmPointsTop[0][2];
	for ( int i = 0; i < nSides; i++ )
	{
		midPoints[0][0] = pmPointsBot[i][0];
		midPoints[0][1] = pmPointsBot[i][1];
		midPoints[1][0] = pmPointsBot[i+1][0];
		midPoints[1][1] = pmPointsBot[i+1][1]; 
		midPoints[2][0] = pmPointsTop[i+1][0];
		midPoints[2][1] = pmPointsTop[i+1][1];

		NewFace.CreateFace(midPoints, 3);
		pSolid->AddFace(&NewFace);
	}
	pSolid->CalcBounds();
	pSolid->InitializeTextureAxes(eTextureAlignment, INIT_TEXTURE_ALL | INIT_TEXTURE_FORCE);
}

// Bipyramid
StockBipyramid::StockBipyramid()
	: StockSolid(5)
{
	AddDataField(DFTYPE_INTEGER, "Width (X)");
	AddDataField(DFTYPE_INTEGER, "Depth (Y)");
	AddDataField(DFTYPE_INTEGER, "Height (Z)");
	AddDataField(DFTYPE_INTEGER, "Number of Sides");
	SetFieldData(fieldSideCount, 8);
	AddDataField(DFTYPE_INTEGER, "Start Angle");
	SetFieldData(fieldStartAngle, 0);
}

void StockBipyramid::SetFromBox(BoundBox *pBox)
{
	SetFieldData(fieldWidth, pBox->bmaxs[0] - pBox->bmins[0]);
	SetFieldData(fieldDepth, pBox->bmaxs[1] - pBox->bmins[1]);
	SetFieldData(fieldHeight, pBox->bmaxs[2] - pBox->bmins[2]);

	Vector o;
	pBox->GetBoundsCenter(o);

	SetOrigin(o);
}

void StockBipyramid::CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment)
{
	float fDepth = float(GetFieldData(fieldDepth))/2;
	float fWidth = float(GetFieldData(fieldWidth))/2;
	float fHeight = float(GetFieldData(fieldHeight))/2;
	int nSides = GetFieldData(fieldSideCount)/2; // divide because it's a symmetrical polyhedron
	int startAngle = GetFieldData(fieldStartAngle);
	CMapFace NewFace;

	// create bottom poly
	Vector pmPoints[64];
	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, nSides, startAngle, pmPoints, false);

	// mid plane
	for(int i = 0; i < nSides+1; i++)
	{
		pmPoints[i][2] = (origin[2]);
	}
	
	// top sides
	Vector Points[3];

	// get centerpoint
	Points[0][0] = origin[0];
	Points[0][1] = origin[1];

	Points[0][2] = (origin[2] + fHeight);

	for(int i = 0; i < nSides; i++)
	{
		Points[1][0] = pmPoints[i][0];
		Points[1][1] = pmPoints[i][1];
		Points[1][2] = pmPoints[i][2];

		Points[2][0] = pmPoints[i+1][0];
		Points[2][1] = pmPoints[i+1][1];
		Points[2][2] = pmPoints[i+1][2];

		NewFace.CreateFace(Points, 3);
		pSolid->AddFace(&NewFace);
	}

	// get centerpoint
	Points[0][0] = origin[0];
	Points[0][1] = origin[1];

	Points[0][2] = (origin[2] - fHeight);

	for(int i = 0; i < nSides; i++)
	{
		Points[2][0] = pmPoints[i][0];
		Points[2][1] = pmPoints[i][1];
		Points[2][2] = pmPoints[i][2];

		Points[1][0] = pmPoints[i+1][0];
		Points[1][1] = pmPoints[i+1][1];
		Points[1][2] = pmPoints[i+1][2];

		NewFace.CreateFace(Points, 3);
		pSolid->AddFace(&NewFace);
	}

	pSolid->CalcBounds();
	pSolid->InitializeTextureAxes(eTextureAlignment, INIT_TEXTURE_ALL | INIT_TEXTURE_FORCE);
}

// Elongated bipyramid
StockElongatedBipyramid::StockElongatedBipyramid()
	: StockSolid(5)
{
	AddDataField(DFTYPE_INTEGER, "Width (X)");
	AddDataField(DFTYPE_INTEGER, "Depth (Y)");
	AddDataField(DFTYPE_INTEGER, "Height (Z)");
	AddDataField(DFTYPE_INTEGER, "Number of Sides");
	SetFieldData(fieldSideCount, 8);
	AddDataField(DFTYPE_INTEGER, "Start Angle");
	SetFieldData(fieldStartAngle, 0);
}

void StockElongatedBipyramid::SetFromBox(BoundBox *pBox)
{
	SetFieldData(fieldWidth, pBox->bmaxs[0] - pBox->bmins[0]);
	SetFieldData(fieldDepth, pBox->bmaxs[1] - pBox->bmins[1]);
	SetFieldData(fieldHeight, pBox->bmaxs[2] - pBox->bmins[2]);

	Vector o;
	pBox->GetBoundsCenter(o);

	SetOrigin(o);
}

void StockElongatedBipyramid::CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment)
{
	float fDepth = float(GetFieldData(fieldDepth))/2;
	float fWidth = float(GetFieldData(fieldWidth))/2;
	float fHeight = float(GetFieldData(fieldHeight))/2;	
	int nSides = GetFieldData(fieldSideCount); // unlike bipyramid, this defines the base, not total side count
	int startAngle = GetFieldData(fieldStartAngle);

	CMapFace NewFace;

	// bottom of the top piramid
	Vector pmPointsTop[128];
	Vector pmPointsBot[128];

	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, nSides, startAngle, pmPointsTop );

	for(int i = 0; i < nSides + 1; i++)
	{
		pmPointsTop[i][2] = (origin[2] + fHeight * 0.5f); // unlike icosahedron, here we can be more arbitrary with height; set the mid part to be half of total height
	}
	
	// top penta
	Vector topPoints[3];

	// get centerpoint
	topPoints[0][0] = origin[0];
	topPoints[0][1] = origin[1];

	topPoints[0][2] = (origin[2] + fHeight);

	for(int i = 0; i < nSides; i++)
	{
		topPoints[1] = pmPointsTop[i];
		topPoints[2] = pmPointsTop[i+1];

		NewFace.CreateFace(topPoints, 3);
		pSolid->AddFace(&NewFace);
	}

	// top of the bottom pyramid
	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, nSides, startAngle, pmPointsBot );
	for(int i = 0; i < nSides + 1; i++)
	{
		pmPointsBot[i][2] = (origin[2] - fHeight * 0.5f);
	}

	Vector botPoints[3];

	// get centerpoint
	botPoints[0][0] = origin[0];
	botPoints[0][1] = origin[1];

	botPoints[0][2] = (origin[2] - fHeight);

	// bottom piramyd
	for(int i = 0; i < nSides; i++)
	{
		botPoints[2][0] = pmPointsBot[i][0];
		botPoints[2][1] = pmPointsBot[i][1];
		botPoints[2][2] = pmPointsBot[i][2];

		botPoints[1][0] = pmPointsBot[i+1][0];
		botPoints[1][1] = pmPointsBot[i+1][1];
		botPoints[1][2] = pmPointsBot[i+1][2];

		NewFace.CreateFace(botPoints, 3);
		pSolid->AddFace(&NewFace);
	}

	// mid band of rectangles
	Vector midPoints[4];
	midPoints[0][2] = pmPointsTop[0][2];
	midPoints[1][2] = pmPointsTop[0][2];
	midPoints[2][2] = pmPointsBot[0][2];
	midPoints[3][2] = pmPointsBot[0][2];
	for ( int i = 0; i < nSides; i++ )
	{
		midPoints[0][0] = pmPointsTop[i+1][0];
		midPoints[0][1] = pmPointsTop[i+1][1]; 
		midPoints[1][0] = pmPointsTop[i][0];
		midPoints[1][1] = pmPointsTop[i][1];
		midPoints[2][0] = pmPointsBot[i][0];
		midPoints[2][1] = pmPointsBot[i][1];
		midPoints[3][0] = pmPointsBot[i+1][0];
		midPoints[3][1] = pmPointsBot[i+1][1];

		NewFace.CreateFace(midPoints, 4);
		pSolid->AddFace(&NewFace);
	}

	pSolid->CalcBounds();
	pSolid->InitializeTextureAxes(eTextureAlignment, INIT_TEXTURE_ALL | INIT_TEXTURE_FORCE);
}

// Gyroelongated bipyramid
StockGyroElongatedBipyramid::StockGyroElongatedBipyramid()
	: StockSolid(5)
{
	AddDataField(DFTYPE_INTEGER, "Width (X)");
	AddDataField(DFTYPE_INTEGER, "Depth (Y)");
	AddDataField(DFTYPE_INTEGER, "Height (Z)");
	AddDataField(DFTYPE_INTEGER, "Number of Sides");
	SetFieldData(fieldSideCount, 8);
	AddDataField(DFTYPE_INTEGER, "Start Angle");
	SetFieldData(fieldStartAngle, 0);
}

void StockGyroElongatedBipyramid::SetFromBox(BoundBox *pBox)
{
	SetFieldData(fieldWidth, pBox->bmaxs[0] - pBox->bmins[0]);
	SetFieldData(fieldDepth, pBox->bmaxs[1] - pBox->bmins[1]);
	SetFieldData(fieldHeight, pBox->bmaxs[2] - pBox->bmins[2]);

	Vector o;
	pBox->GetBoundsCenter(o);

	SetOrigin(o);
}

void StockGyroElongatedBipyramid::CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment)
{
	float fDepth = float(GetFieldData(fieldDepth))/2;
	float fWidth = float(GetFieldData(fieldWidth))/2;
	float fHeight = float(GetFieldData(fieldHeight))/2;	
	int nSides = GetFieldData(fieldSideCount); // unlike bipyramid, this defines the base, not total side count
	int startAngle = GetFieldData(fieldStartAngle);

	CMapFace NewFace;

	// bottom of the top piramid
	Vector pmPointsTop[128];
	Vector pmPointsBot[128];

	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, nSides, startAngle, pmPointsTop );

	for(int i = 0; i < nSides + 1; i++)
	{
		pmPointsTop[i][2] = (origin[2] + fHeight * 0.5f); // unlike icosahedron, here we can be more arbitrary with height; set the mid part to be half of total height
	}
	
	// top penta
	Vector topPoints[3];

	// get centerpoint
	topPoints[0][0] = origin[0];
	topPoints[0][1] = origin[1];

	topPoints[0][2] = (origin[2] + fHeight);

	for(int i = 0; i < nSides; i++)
	{
		topPoints[1] = pmPointsTop[i];
		topPoints[2] = pmPointsTop[i+1];

		NewFace.CreateFace(topPoints, 3);
		pSolid->AddFace(&NewFace);
	}

	// top of the bottom pyramid, rotated by 180/nSides degrees
	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, nSides, startAngle + (180 / nSides), pmPointsBot );
	for(int i = 0; i < nSides + 1; i++)
	{
		pmPointsBot[i][2] = (origin[2] - fHeight * 0.5f);
	}

	Vector botPoints[3];

	// get centerpoint
	botPoints[0][0] = origin[0];
	botPoints[0][1] = origin[1];

	botPoints[0][2] = (origin[2] - fHeight);

	// bottom piramyd
	for(int i = 0; i < nSides; i++)
	{
		botPoints[2] = pmPointsBot[i];
		botPoints[1] = pmPointsBot[i+1];

		NewFace.CreateFace(botPoints, 3);
		pSolid->AddFace(&NewFace);
	}

	// mid band of triangles
	Vector midPoints[3];
	midPoints[0][2] = pmPointsTop[0][2];
	midPoints[1][2] = pmPointsTop[0][2];
	midPoints[2][2] = pmPointsBot[0][2];
	for ( int i = 0; i < nSides; i++ )
	{
		midPoints[0][0] = pmPointsTop[i+1][0];
		midPoints[0][1] = pmPointsTop[i+1][1]; 
		midPoints[1][0] = pmPointsTop[i][0];
		midPoints[1][1] = pmPointsTop[i][1];
		midPoints[2][0] = pmPointsBot[i][0];
		midPoints[2][1] = pmPointsBot[i][1];

		NewFace.CreateFace(midPoints, 3);
		pSolid->AddFace(&NewFace);
	}	
	midPoints[0][2] = pmPointsBot[0][2];
	midPoints[1][2] = pmPointsBot[0][2];
	midPoints[2][2] = pmPointsTop[0][2];
	for ( int i = 0; i < nSides; i++ )
	{
		midPoints[0][0] = pmPointsBot[i][0];
		midPoints[0][1] = pmPointsBot[i][1];
		midPoints[1][0] = pmPointsBot[i+1][0];
		midPoints[1][1] = pmPointsBot[i+1][1]; 
		midPoints[2][0] = pmPointsTop[i+1][0];
		midPoints[2][1] = pmPointsTop[i+1][1];

		NewFace.CreateFace(midPoints, 3);
		pSolid->AddFace(&NewFace);
	}

	pSolid->CalcBounds();
	pSolid->InitializeTextureAxes(eTextureAlignment, INIT_TEXTURE_ALL | INIT_TEXTURE_FORCE);
}

// Icosahedron
StockIcosahedron::StockIcosahedron()
	: StockSolid(4)
{
	AddDataField(DFTYPE_INTEGER, "Width (X)");
	AddDataField(DFTYPE_INTEGER, "Depth (Y)");
	AddDataField(DFTYPE_INTEGER, "Height (Z)");
	AddDataField(DFTYPE_INTEGER, "Start Angle");
	SetFieldData(fieldStartAngle, 0);
}

void StockIcosahedron::SetFromBox(BoundBox *pBox)
{
	SetFieldData(fieldWidth, pBox->bmaxs[0] - pBox->bmins[0]);
	SetFieldData(fieldDepth, pBox->bmaxs[1] - pBox->bmins[1]);
	SetFieldData(fieldHeight, pBox->bmaxs[2] - pBox->bmins[2]);

	Vector o;
	pBox->GetBoundsCenter(o);

	SetOrigin(o);
}

void StockIcosahedron::CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment)
{
	float fDepth = float(GetFieldData(fieldDepth))/2;
	float fWidth = float(GetFieldData(fieldWidth))/2;
	float fHeight = float(GetFieldData(fieldHeight))/2;
	int startAngle = GetFieldData(fieldStartAngle);

	CMapFace NewFace;

	// bottom of the top penta
	Vector pmPointsTop[64];
	Vector pmPointsBot[64];

	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, 5, startAngle, pmPointsTop, false);

	for(int i = 0; i < 6; i++)
	{
		pmPointsTop[i][2] = (origin[2] + fHeight / 2); // the gap between the two pentagonal planes equals the radius of the circle around them, which is equal to the half width of the solid
	}
	
	// top penta
	Vector topPoints[3];

	// get centerpoint
	topPoints[0][0] = origin[0];
	topPoints[0][1] = origin[1];

	topPoints[0][2] = (origin[2] + fHeight);

	for(int i = 0; i < 5; i++)
	{
		topPoints[1] = pmPointsTop[i];
		topPoints[2] = pmPointsTop[i+1];

		NewFace.CreateFace(topPoints, 3);
		pSolid->AddFace(&NewFace);
	}

	// top of the bottom penta	
	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, 5, startAngle + 36, pmPointsBot, false); // rotate bottom by 180 / 5 deg
	for(int i = 0; i < 6; i++)
	{
		pmPointsBot[i][2] = (origin[2] - fHeight / 2); // see above
	}

	Vector botPoints[3];

	// get centerpoint
	botPoints[0][0] = origin[0];
	botPoints[0][1] = origin[1];

	botPoints[0][2] = (origin[2] - fHeight);

	// bottom penta
	for(int i = 0; i < 5; i++)
	{
		botPoints[2][0] = pmPointsBot[i][0];
		botPoints[2][1] = pmPointsBot[i][1];
		botPoints[2][2] = pmPointsBot[i][2];

		botPoints[1][0] = pmPointsBot[i+1][0];
		botPoints[1][1] = pmPointsBot[i+1][1];
		botPoints[1][2] = pmPointsBot[i+1][2];

		NewFace.CreateFace(botPoints, 3);
		pSolid->AddFace(&NewFace);
	}

	// mid band of triangles
	Vector midPoints[3];
	midPoints[0][2] = pmPointsTop[0][2];
	midPoints[1][2] = pmPointsTop[0][2];
	midPoints[2][2] = pmPointsBot[0][2];
	for ( int i = 0; i < 5; i++ )
	{
		midPoints[0][0] = pmPointsTop[i+1][0];
		midPoints[0][1] = pmPointsTop[i+1][1]; 
		midPoints[1][0] = pmPointsTop[i][0];
		midPoints[1][1] = pmPointsTop[i][1];
		midPoints[2][0] = pmPointsBot[i][0];
		midPoints[2][1] = pmPointsBot[i][1];

		NewFace.CreateFace(midPoints, 3);
		pSolid->AddFace(&NewFace);
	}	
	midPoints[0][2] = pmPointsBot[0][2];
	midPoints[1][2] = pmPointsBot[0][2];
	midPoints[2][2] = pmPointsTop[0][2];
	for ( int i = 0; i < 5; i++ )
	{
		midPoints[0][0] = pmPointsBot[i][0];
		midPoints[0][1] = pmPointsBot[i][1];
		midPoints[1][0] = pmPointsBot[i+1][0];
		midPoints[1][1] = pmPointsBot[i+1][1]; 
		midPoints[2][0] = pmPointsTop[i+1][0];
		midPoints[2][1] = pmPointsTop[i+1][1];

		NewFace.CreateFace(midPoints, 3);
		pSolid->AddFace(&NewFace);
	}
	pSolid->CalcBounds();
	pSolid->InitializeTextureAxes(eTextureAlignment, INIT_TEXTURE_ALL | INIT_TEXTURE_FORCE);
}

// Cupola
StockCupola::StockCupola()
	: StockSolid(4)
{
	AddDataField(DFTYPE_INTEGER, "Width (X)");
	AddDataField(DFTYPE_INTEGER, "Depth (Y)");
	AddDataField(DFTYPE_INTEGER, "Height (Z)");
	AddDataField(DFTYPE_INTEGER, "Number of Sides");
	SetFieldData(fieldSideCount, 8);
}

void StockCupola::SetFromBox(BoundBox *pBox)
{
	SetFieldData(fieldWidth, pBox->bmaxs[0] - pBox->bmins[0]);
	SetFieldData(fieldDepth, pBox->bmaxs[1] - pBox->bmins[1]);
	SetFieldData(fieldHeight, pBox->bmaxs[2] - pBox->bmins[2]);

	Vector o;
	pBox->GetBoundsCenter(o);

	SetOrigin(o);
}

void StockCupola::CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment)
{
	float fDepth = float(GetFieldData(fieldDepth))/2;
	float fWidth = float(GetFieldData(fieldWidth))/2;
	float fHeight = float(GetFieldData(fieldHeight))/2;
	int topSides = GetFieldData(fieldSideCount); // for cupola, number of faces is the top side, not total of faces
	int botSides = topSides * 2;

	CMapFace NewFace;

	Vector pmPointsTop[128];
	Vector pmPointsBot[256];

	// bottom face
	polyMake(origin[0] - fWidth, origin[1] - fDepth, origin[0] + fWidth, origin[1] + fDepth, botSides, 0, pmPointsBot );

	for(int i = 0; i < botSides + 1; i++)
	{
		pmPointsTop[i][2] = (origin[2] - fHeight);
	}

	NewFace.CreateFace(pmPointsBot, -botSides);
	pSolid->AddFace(&NewFace);

	// top face, make it half as large in area as the bottom (bottom is based on the bbox)
	polyMake(origin[0] - fWidth / 2, origin[1] - fDepth / 2, origin[0] + fWidth / 2, origin[1] + fDepth / 2, topSides, 0, pmPointsTop );
	for(int i = 0; i < topSides + 1; i++)
	{
		pmPointsTop[i][2] = (origin[2] + fHeight);
	}
	
	NewFace.CreateFace(pmPointsTop, topSides);
	pSolid->AddFace(&NewFace);

	// connect the bottom to the top with alternating triangles and rectangles
	// triangles connect to top corners, rectangles connect to top sides
	// ...

	pSolid->CalcBounds();
	pSolid->InitializeTextureAxes(eTextureAlignment, INIT_TEXTURE_ALL | INIT_TEXTURE_FORCE);
}
#endif //// SLE
