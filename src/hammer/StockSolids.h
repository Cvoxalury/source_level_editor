//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef STOCKSOLIDS_H
#define STOCKSOLIDS_H
#ifdef _WIN32
#pragma once
#endif

#pragma warning(push, 1)
#pragma warning(disable:4701 4702 4530)
#include <fstream>
#pragma warning(pop)
#include "mathlib/vector.h"
#include "MapFace.h"

class BoundBox;
class CMapSolid;
class Vector;

class StockSolid
{
	public:
		void Serialize(std::fstream& file, BOOL bIsStoring);
		int GetFieldCount() const;
		void SetFieldData(int iIndex, int iData);
		int GetFieldData(int iIndex, int *piData = NULL) const;
		void GetFieldRange(int iIndex, int *piRangeLower, int *piRangeUpper);
		void SetOrigin(const Vector &o);
		void SetCenterOffset(const Vector &ofs);
		
		virtual void SetFromBox(BoundBox * pBox) {}

		virtual void CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eAlignment) = 0;

		~StockSolid();

	protected:
		StockSolid(int nFields);

		typedef enum
		{
			DFTYPE_INTEGER,
			DFTYPE_BOOLEAN
		} STSDF_TYPE;

		void AddDataField(STSDF_TYPE type, const char *pszName, int iRangeLower = -1,
			int iRangeUpper = -1);

		Vector origin;
		Vector cofs;

	private:
		void AllocateDataFields(int nFields);

		enum
		{
			DFFLAG_RANGED = 0x01
		};

		typedef struct
		{
			unsigned flags;
			STSDF_TYPE type;
			char szName[128];

		// range:
			int iRangeLower;
			int iRangeUpper;

		// value:
			int iValue;

		} STSDATAFIELD;

		STSDATAFIELD *pFields;
		int nFields;
		int iMaxFields;
};

class StockBlock : public StockSolid
{
	public:
		StockBlock();

		void CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment);
		void SetFromBox(BoundBox *pBox);

		enum { fieldWidth, fieldDepth, fieldHeight };
};

class StockWedge : public StockSolid
{
	public:
		StockWedge();

		void CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment);
		void SetFromBox(BoundBox *pBox);
		
#ifdef SLE //// SLE NEW - set start angle for block tool
		enum { fieldWidth, fieldDepth, fieldHeight, fieldStartAngle  };
#else
		enum { fieldWidth, fieldDepth, fieldHeight };
#endif
};

class StockCylinder : public StockSolid
{
	public:
		StockCylinder();

		void CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment);
		void SetFromBox(BoundBox *pBox);
		
#ifdef SLE //// SLE NEW - set start angle for block tool
		enum { fieldWidth, fieldDepth, fieldHeight, fieldSideCount, fieldStartAngle };
#else
		enum { fieldWidth, fieldDepth, fieldHeight, fieldSideCount };
#endif
};

class StockPyramid : public StockSolid
{
	public:
		StockPyramid();

		void CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment);
		void SetFromBox(BoundBox *pBox);
		
#ifdef SLE //// SLE NEW - set start angle for block tool
		enum { fieldWidth, fieldDepth, fieldHeight, fieldSideCount, fieldStartAngle };
#else
		enum { fieldWidth, fieldDepth, fieldHeight, fieldSideCount };
#endif
};

class StockSphere : public StockSolid
{
	public:
		StockSphere();

		void CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment);
		void SetFromBox(BoundBox *pBox);
		
#ifdef SLE //// SLE NEW - set start angle for block tool
		enum { fieldWidth, fieldDepth, fieldHeight, fieldSideCount, fieldStartAngle };
#else
		enum { fieldWidth, fieldDepth, fieldHeight, fieldSideCount };
#endif
};
#ifdef SLE /// SLE NEW - more stock solids
class StockAntiprism : public StockSolid
{
	public:
		StockAntiprism();

		void CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment);
		void SetFromBox(BoundBox *pBox);
		
		enum { fieldWidth, fieldDepth, fieldHeight, fieldSideCount, fieldStartAngle  };
};

class StockBipyramid : public StockSolid // things like a regular octahedron
{
	public:
		StockBipyramid();

		void CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment);
		void SetFromBox(BoundBox *pBox);

		enum { fieldWidth, fieldDepth, fieldHeight, fieldSideCount, fieldStartAngle  };
};

class StockElongatedBipyramid : public StockSolid
{
	public:
		StockElongatedBipyramid();

		void CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment);
		void SetFromBox(BoundBox *pBox);

		enum { fieldWidth, fieldDepth, fieldHeight, fieldSideCount, fieldStartAngle  };
};

class StockGyroElongatedBipyramid : public StockSolid
{
	public:
		StockGyroElongatedBipyramid();

		void CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment);
		void SetFromBox(BoundBox *pBox);

		enum { fieldWidth, fieldDepth, fieldHeight, fieldSideCount, fieldStartAngle  };
};

class StockIcosahedron : public StockSolid // convex regular icosahedron
{
public:
	StockIcosahedron();

	void CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment);
	void SetFromBox(BoundBox *pBox);

	enum { fieldWidth, fieldDepth, fieldHeight, fieldStartAngle  };
};

class StockCupola : public StockSolid
{
public:
	StockCupola();

	void CreateMapSolid(CMapSolid *pSolid, TextureAlignment_t eTextureAlignment);
	void SetFromBox(BoundBox *pBox);

	enum { fieldWidth, fieldDepth, fieldHeight, fieldSideCount };
};
#endif
#endif // STOCKSOLIDS_H
