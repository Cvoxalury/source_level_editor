//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef MAPROTATIONDISTANCEVISUALIZER_H
#define MAPROTATIONDISTANCEVISUALIZER_H
#ifdef _WIN32
#pragma once
#endif

#include "MapHelper.h"
#include "toolinterface.h"

class CHelperInfo;
class CRender2D;
class CRender3D;

class CMapRotationDistanceVisualizer : public CMapHelper
{
public:
	DECLARE_MAPCLASS( CMapRotationDistanceVisualizer, CMapHelper )

	//
	// Factory for building from a list of string parameters.
	//
	static CMapClass* Create( CHelperInfo* pInfo, CMapEntity* pParent );

	//
	// Construction/destruction:
	//
	CMapRotationDistanceVisualizer();
	CMapRotationDistanceVisualizer(const char *pszOriginKey, const char *pszDirKey, const char *pszArcKey);
	~CMapRotationDistanceVisualizer();

	void CalcBounds( BOOL bFullUpdate = FALSE );

	CMapClass* Copy( bool bUpdateDependencies );
	CMapClass* CopyFrom( CMapClass* pFrom, bool bUpdateDependencies );

	void Render2D( CRender2D* pRender );
	void Render3D( CRender3D* pRender );

	// Overridden because origin helpers don't take the color of their parent entity.
	void SetRenderColor( unsigned char red, unsigned char green, unsigned char blue );
	void SetRenderColor( color32 rgbColor );
	
	bool ShouldRenderLast() { return true; }
	virtual bool IsVisualElement() { return true; } // Only visible if our parent is selected.
	virtual bool IsClutter() const { return true; }
	virtual bool CanBeCulledByCordon() const { return false; } // We don't hide unless our parent hides.

	const char* GetDescription() { return "Rotation Distance visualizer"; }

	void OnParentKeyChanged( const char* key, const char* value );

private:
	void Initialize();

	char m_szOriginKeyName[32];
	char m_szDirKeyName[32];
	char m_szArcKeyName[32];
	Vector m_dir_vector;
	float m_arc_float;

	bool m_x_axis_bool;
	bool m_y_axis_bool;
};

#endif // MAPROTATIONDISTANCEVISUALIZER_H
