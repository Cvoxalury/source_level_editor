//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef MAPDIRVISUALIZER_H
#define MAPDIRVISUALIZER_H
#ifdef _WIN32
#pragma once
#endif

#include "MapHelper.h"
#include "toolinterface.h"

class CHelperInfo;
class CRender2D;
class CRender3D;

class CMapDirectionVisualizer : public CMapHelper
{
public:
	DECLARE_MAPCLASS( CMapDirectionVisualizer, CMapHelper )

	//
	// Factory for building from a list of string parameters.
	//
	static CMapClass* Create( CHelperInfo* pInfo, CMapEntity* pParent );

	//
	// Construction/destruction:
	//
	CMapDirectionVisualizer();
	CMapDirectionVisualizer(const char *pszKey);
	~CMapDirectionVisualizer() override;

	void CalcBounds( BOOL bFullUpdate = FALSE ) override;

	CMapClass* Copy( bool bUpdateDependencies ) override;
	CMapClass* CopyFrom( CMapClass* pFrom, bool bUpdateDependencies ) override;

	void Render2D( CRender2D* pRender ) override;
	void Render3D( CRender3D* pRender ) override;

	// Overridden because origin helpers don't take the color of their parent entity.
	void SetRenderColor( unsigned char red, unsigned char green, unsigned char blue ) override;
	void SetRenderColor( color32 rgbColor ) override;

	virtual bool IsVisualElement() { return true; } // Only visible if our parent is selected.
	virtual bool IsClutter() const { return true; }
	virtual bool CanBeCulledByCordon() const { return false; } // We don't hide unless our parent hides.

	const char* GetDescription() override { return "Direction visualizer"; }

	void OnParentKeyChanged( const char* key, const char* value ) override;

private:
	void Initialize();

	char m_szKeyName[32];
	Vector m_Dir;
};

#endif // MAPDIRVISUALIZER_H
