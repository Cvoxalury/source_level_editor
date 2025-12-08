//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef SUNHELPER_H
#define SUNHELPER_H
#pragma once

#include "MapHelper.h"
#include "tier1/utlstring.h"

class CRender3D;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CSunHelper : public CMapHelper
{
public:
	//
	// Factories.
	//
	static CMapClass* CreateSun(CHelperInfo* pHelperInfo, CMapEntity* pParent);

	//
	// Construction/destruction:
	//
	CSunHelper();
	~CSunHelper() override = default;

	DECLARE_MAPCLASS(CSunHelper, CMapHelper)

	void CalcBounds(BOOL bFullUpdate = FALSE) override;

	CMapClass* Copy(bool bUpdateDependencies) override;
	CMapClass* CopyFrom(CMapClass* pFrom, bool bUpdateDependencies) override;

	void Render3D(CRender3D* pRender) override;

	bool ShouldRenderLast() override { return true; }
	bool IsVisualElement() override { return true; }
	const char* GetDescription() override { return "Sun"; }
	void OnParentKeyChanged(const char* szKey, const char* szValue) override;

protected:
	void Initialize();
	//
	// Implements CMapAtom transformation functions.
	//
	void DoTransform(const VMatrix& matrix) override;

	QAngle m_Angles;

	int m_eRenderMode;				// Our render mode (transparency, etc.).
	colorVec m_RenderColor;			// Our render color.
	float m_sunsize_float;
	CUtlString m_suntexturename_string;
};

#endif // SUNHELPER_H