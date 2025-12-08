//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef MAPSPRITEBEAM_H
#define MAPSPRITEBEAM_H
#pragma once

#include "MapHelper.h"

class CRender3D;

#define MAX_KEYNAME_SIZE	32

class CMapSpriteBeam : public CMapHelper
{
public:

	DECLARE_MAPCLASS(CMapSpriteBeam, CMapHelper)

	//
	// Factory for building from a list of string parameters.
	//
	static CMapClass *Create(CHelperInfo *pInfo, CMapEntity *pParent);

	// laser is a custom case of beam where the start entity is always the parent.
	static CMapClass *CreateLaser(CHelperInfo *pInfo, CMapEntity *pParent);

	//
	// Construction/destruction:
	//
	CMapSpriteBeam(void);
	CMapSpriteBeam(const char *pszStartValueKey, 
		const char *pszEndValueKey, 
		const char *pszMaterialValueKey,
		const char *pszWidthValueKey,
		bool isLaser);
	~CMapSpriteBeam(void);

	void Initialize(void);

	void BuildLine(void);
	void CalcBounds(BOOL bFullUpdate = FALSE);

	virtual CMapClass *Copy(bool bUpdateDependencies);
	virtual CMapClass *CopyFrom(CMapClass *pFrom, bool bUpdateDependencies);

	void Render3D(CRender3D *pRender);
	void Render2D(CRender2D *pRender);

	int SerializeRMF(std::fstream &File, BOOL bRMF);
	int SerializeMAP(std::fstream &File, BOOL bRMF);

	bool IsVisualElement(void) { return( true ); }
	bool ShouldRenderLast() override { return true; }
#ifdef HAMMER2013_PORT_CORDONS
	virtual bool CanBeCulledByCordon() const { return false; }
#else
	bool IsCulledByCordon(const Vector &vecMins, const Vector &vecMaxs) { return false; } // We don't hide unless our parent hides.
#endif
	virtual CMapClass *PrepareSelection(SelectMode_t eSelectMode);

	const char* GetDescription() { return( "Beam helper" ); }

	void OnAddToWorld(CMapWorld *pWorld);
	void OnNotifyDependent(CMapClass *pObject, Notify_Dependent_t eNotifyType);
	void OnParentKeyChanged(const char* key, const char* value);
	void OnRemoveFromWorld(CMapWorld *pWorld, bool bNotifyChildren);

	virtual void UpdateDependencies(CMapWorld *pWorld, CMapClass *pObject);

	bool m_islaser_boolean;

protected:
	//
	// Implements CMapAtom transformation functions.
	//
	void DoTransform(const VMatrix &matrix);

	char m_szStartValueKey[ 80 ];		// The key in our parent entity to look at for our start target.
	char m_szEndValueKey[ 80 ];		// 
	char m_szMaterialValueKey[ MAX_PATH ];
	char m_szMaterialKey[ MAX_PATH ];
	char m_szWidthValueKey[ 16 ];
	char m_szWidthKey[ 16 ];

	float m_colorR, m_colorG, m_colorB;

	CMapEntity *m_pStartEntity;		// Our start target.
	CMapEntity *m_pEndEntity;		// Our end target.
};

class CMapSpriteTesla : public CMapSpriteBeam
{
public:
	DECLARE_MAPCLASS(CMapSpriteTesla, CMapSpriteBeam)

	CMapSpriteTesla(void);
	CMapSpriteTesla(const char *pszStartValueKey,
		const char *pszEndValueKey,
		const char *pszMaterialValueKey,
		const char *pszWidthValueKey);
	~CMapSpriteTesla(void);

	const char* GetDescription() { return( "Tesla helper" ); }

	static CMapClass *Create(CHelperInfo *pInfo, CMapEntity *pParent);
};
#endif // MAPSPRITEBEAM_H