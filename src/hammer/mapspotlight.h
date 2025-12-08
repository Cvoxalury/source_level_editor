//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef SPOTLIGHTHELPER_H
#define SPOTLIGHTHELPER_H
#pragma once

#include "MapHelper.h"
#include "tier1/utlstring.h"

class CRender3D;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CSpotlightHelper : public CMapHelper
{
public:
	//
	// Factories.
	//
	static CMapClass* CreateSpotlight( CHelperInfo* pHelperInfo, CMapEntity* pParent );

	//
	// Construction/destruction:
	//
	CSpotlightHelper();
	~CSpotlightHelper() override = default;

	DECLARE_MAPCLASS(CSpotlightHelper, CMapHelper )

	void CalcBounds( BOOL bFullUpdate = FALSE ) override;

	CMapClass* Copy( bool bUpdateDependencies ) override;
	CMapClass* CopyFrom( CMapClass* pFrom, bool bUpdateDependencies ) override;

	void Render2D( CRender2D* pRender ) override;
	void Render3D( CRender3D* pRender ) override;

	bool ShouldRenderLast() override { return true; }
	bool IsVisualElement() override { return true; }
	const char* GetDescription() override { return "Spotlight"; }
	void OnParentKeyChanged( const char* szKey, const char* szValue ) override;

protected:
	void Initialize();
	//
	// Implements CMapAtom transformation functions.
	//
	void DoTransform( const VMatrix& matrix ) override;

	QAngle m_Angles;

	int m_eRenderMode;				// Our render mode (transparency, etc.).
	colorVec m_RenderColor;			// Our render color.
	float m_beamlength_float;
	float m_beamwidth_float;
	CUtlString m_beamtexturename_string;
	
private:
	void ComputeCornerVertices( Vector* pVerts, float length, float width ) const;
	void GetSpriteAxes(const QAngle& Angles, int type, Vector& forward, Vector& right, Vector& up, 
		const Vector& ViewUp, const Vector& ViewRight, const Vector& ViewForward);

	Vector2D m_TexUL, m_TexLR;
	Vector2D m_UL, m_LR;
};

#endif // SPOTLIGHTHELPER_H