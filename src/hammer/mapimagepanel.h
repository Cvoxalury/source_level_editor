//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef IMAGEPANELHELPER_H
#define IMAGEPANELHELPER_H
#pragma once

#include "MapEntity.h"
#include "MapHelper.h"
#include "tier1/utlstring.h"
#include "../materialsystem/itextureinternal.h"

class CRender3D;

//#define SLE_IMAGEPANEL_USE_STBI
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CImagePanelHelper : public CMapHelper
{
public:
	//
	// Factories.
	//
	static CMapClass* CreateImagePanel(CHelperInfo* pHelperInfo, CMapEntity* pParent);

	//
	// Construction/destruction:
	//
	CImagePanelHelper();
	~CImagePanelHelper();

	DECLARE_MAPCLASS(CImagePanelHelper, CMapHelper)

	void CalcBounds(BOOL bFullUpdate = FALSE);

	CMapClass* Copy(bool bUpdateDependencies);
	CMapClass* CopyFrom(CMapClass* pFrom, bool bUpdateDependencies);

	void Render2D(CRender2D* pRender);
	void Render3D(CRender3D* pRender);

	bool ShouldRenderLast() { return true; }
	bool IsVisualElement() { return true; }
	const char* GetDescription() { return "ImagePanel"; }
	void OnParentKeyChanged(const char* szKey, const char* szValue);

protected:
	void SetImageFilename(const char* pNewText);
	void RenderImagePanel(CRender2D *pRender2D, CRender3D *pRender3D, CString &filename);
	bool ReadImageData( CString &filename);
	//
	// Implements CMapAtom transformation functions.
	//
	void DoTransform(const VMatrix& matrix);
	void SetRenderMode(int mode);

	QAngle m_Angles;

	int m_eRenderMode;				// Our render mode (transparency, etc.).
	colorVec m_RenderColor;			// Our render color.
	CUtlString m_pFilename;
	Vector m_corners[4];

	bool m_show2D_bool;				// Display this world text in 2d views (false by default)
	bool m_show3D_bool;				// Display this world text in 3d views (true by default)

private:
	void ComputeCornerVertices(Vector* pVerts, float flBloat = 0.0f) const;

	// texture regenerators and procedural textures for drawing TGA backgrounds as rescalable VTFs.	
	unsigned char		*m_bg_pixels;
	int					m_bg_width_int, m_bg_height_int;
#ifdef SLE_IMAGEPANEL_USE_STBI
	ITexture			*m_bg_tex;
#else
	ITextureInternal	*m_bg_tex;
#endif
	IMaterial			*m_bg_mat;
	bool				m_bg_loaded_bool;

public:
	//// SLE NEW - background images
	struct {
		bool is_enabled_bool;
		bool is_nocull_bool;
		bool is_flipped_x;
		bool is_flipped_y;
		int width_int;
		int height_int;
		float alpha_float; // 0-1, gets turned into 0-255 in render code
		int center_x_int; // note - called x, y regardless of bg axis
		int center_y_int;
		CString filename_string;
		unsigned char		*image_char;
		int image_width_int;
		int image_height_int;
		ITextureInternal	*texture_itex;
		IMaterial			*material_imat;
	} m_bg;
};

#endif // IMAGEPANELHELPER_H