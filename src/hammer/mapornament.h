//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef ORNAMENTHELPER_H
#define ORNAMENTHELPER_H
#pragma once

#include "MapHelper.h"
#include "StudioModel.h"
#include "tier1/utlstring.h"

class CRender3D;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class COrnamentHelper : public CMapHelper
{
public:
	//
	// Factories.
	//
	static CMapClass* CreateOrnament(CHelperInfo* pHelperInfo, CMapEntity* pParent);

	//
	// Construction/destruction:
	//
	COrnamentHelper();
	~COrnamentHelper();

	DECLARE_MAPCLASS(CSpotlightHelper, CMapHelper)

	void CalcBounds(BOOL bFullUpdate = FALSE) override;

	CMapClass* Copy(bool bUpdateDependencies) override;
	CMapClass* CopyFrom(CMapClass* pFrom, bool bUpdateDependencies) override;

	bool RenderPreload(CRender3D *pRender, bool bNewContext);
	void Render2D(CRender2D* pRender);
	void Render3D(CRender3D* pRender);

	void GetAngles(QAngle& pfAngles);
	bool ShouldRenderLast() override { return false; }
	bool IsVisualElement() override { return true; }
	const char* GetDescription() override { return "Ornament"; }
	void OnParentKeyChanged(const char* szKey, const char* szValue) override;

protected:
	QAngle m_Angles; // Euler angles of this studio model.
	void GetRenderAngles(QAngle &Angles);
	void Initialize();
	//
	// Implements CMapAtom transformation functions.
	//
	void DoTransform(const VMatrix& matrix) override;

	Vector m_model_pos_vec;
	int m_model_skin_int;
	int m_model_attachment_int;
	float m_model_scale_float;
	CUtlString m_model_name_string;
	CUtlString m_model_attachment_string;
	StudioModel *m_model_handle;
	CMapEntity *m_parent_handle;
};

#endif // ORNAMENTHELPER_H