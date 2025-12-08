//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef WORLDTEXTHELPER_H
#define WORLDTEXTHELPER_H
#pragma once


#include "MapHelper.h"
#include "tier1/utlstring.h"

class CRender3D;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CWorldTextHelper : public CMapHelper
{
public:
	//
	// Factories.
	//
	static CMapClass* CreateWorldText( CHelperInfo* pHelperInfo, CMapEntity* pParent );

	//
	// Construction/destruction:
	//
	CWorldTextHelper();
	~CWorldTextHelper() override = default;

	DECLARE_MAPCLASS( CWorldTextHelper, CMapHelper )

	void CalcBounds( BOOL bFullUpdate = FALSE ) override;

	CMapClass* Copy( bool bUpdateDependencies ) override;
	CMapClass* CopyFrom( CMapClass* pFrom, bool bUpdateDependencies ) override;

	void Render2D( CRender2D* pRender ) override;
	void Render3D( CRender3D* pRender ) override;

	bool ShouldRenderLast() override { return false; }
	bool IsVisualElement() override { return true; }
	const char* GetDescription() override { return "WorldText"; }
	void OnParentKeyChanged( const char* szKey, const char* szValue ) override;

protected:
	void SetText( const char* pNewText );
	void Initialize();
	void Render3DText( CRender3D *pRender, const char* szText, const float flTextSize );

	//
	// Implements CMapAtom transformation functions.
	//
	void DoTransform( const VMatrix& matrix ) override;
	void SetRenderMode( int mode );

	QAngle m_Angles;

	int m_eRenderMode;				// Our render mode (transparency, etc.).
	colorVec m_RenderColor;			// Our render color.
	float m_flTextSize;
	CUtlString m_pText;

private:
	void ComputeCornerVertices( Vector* pVerts, float flBloat = 0.0f ) const;
};

#endif // WORLDTEXTHELPER_H