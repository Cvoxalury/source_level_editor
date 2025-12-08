//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Defines options that are written to the registry.
//
//=============================================================================//

#ifndef OPTIONS_H
#define OPTIONS_H
#pragma once

#pragma warning(push, 1)
#pragma warning(disable:4701 4702 4530)
#include <fstream>
#pragma warning(pop)
#include <afxtempl.h>
#ifdef SLE
#include "IHammer.h"
#endif
class CGameConfig;
class KeyValues;
enum TextureAlignment_t;

class COptionsGeneral
{
public:
	int nMaxCameras;
	int iUndoLevels;
	BOOL bLockingTextures;
	BOOL bScaleLockingTextures;
	TextureAlignment_t eTextureAlignment;
	BOOL bLoadwinpos;
	BOOL bIndependentwin;
	BOOL bGroupWhileIgnore;
	BOOL bStretchArches;
	BOOL bShowHelpers;
	BOOL bCheckVisibleMapErrors;	
	int iTimeBetweenSaves;
	int iMaxAutosaveSpace;
	int iMaxAutosavesPerMap;
	BOOL bClosedCorrectly;
	char szAutosaveDir[MAX_PATH];
	BOOL bUseVGUIModelBrowser;
	BOOL bShowCollisionModels;
	BOOL bShowDetailObjects;
	BOOL bShowNoDrawBrushes;

	BOOL bEnableAutosave;
	BOOL bShowHiddenTargetsAsBroken;
	BOOL bRadiusCulling;
#ifdef SLE
	BOOL bEnableInstancesLoading = true; //// SLE NEW - control to disable loading instances
	BOOL bRadiusCullingFollowCamera = true; //// SLE NEW - if disabled, radius culling won't follow the camera

	BOOL bShowToolBrushFaces; //// SLE NEW: Tool Brush Texture display filter/toogle.
	BOOL bShowEditorObjects; //// SLE NEW: Editor objects display filter/toggle.
	BOOL bShowToolsSkyFaces; //// SLE NEW: ToolsSky Texture display filter/toogle.
	BOOL bShowMapRestorePrompt; //// SLE NEW - option to not show map restore prompt after a crash
	BOOL bEasterEggSplashes; //// SLE NEW - easter egg splash screens
	BOOL bDispVertexLockEnabled; //// SLE NEW: button toggle, works as holding shift w/ moving vertices in disp tool. (locks selection on vertex until you let go lmb)

	int iDeselectFacesThreshold; //// SLE NEW - safeguard against accidentally clicking and losing a lot of selected faces
#endif
}; 

class COptionsTextures
{
public:
	CStringArray TextureFiles;
	int nTextureFiles;
#ifndef SLE //// SLE REMOVE - deprecated, Quake 2
	float fBrightness;
#endif
};

class COptionsColors
{
public:

	bool bUseCustom;				// Whether to use the custom colors or not.
	bool bScaleAxisColor;			// Whether to use the intensity slider to scale the axis color in the 2D view.
	bool bScaleGridColor;			// Whether to use the intensity slider to scale the grid color in the 2D view.
	bool bScaleGridDotColor;		// Whether to use the intensity slider to scale the dotted grid color in the 2D view.
	bool bScaleGrid10Color;			// Whether to use the intensity slider to scale the 10 grid color in the 2D view.
	bool bScaleGrid1024Color;		// Whether to use the intensity slider to scale the 1024 grid color in the 2D view.

	COLORREF clrAxis;				// The 2D view axis color.
	COLORREF clrGrid;				// The 2D view grid color.
	COLORREF clrGridDot;			// The 2D view dotted grid color.
	COLORREF clrGrid10;				// The 2D view grid color for every 10th line.
	COLORREF clrGrid1024;			// The 2D view grid color for every 1024 units line.
#ifdef SLE //// renamed for clarity
	COLORREF clr2DBackground;		// The 2D view background color.
#else
	COLORREF clrBackground;			// The 2D view background color.
#endif
	COLORREF clrBrush;				// The color of brushes.
	COLORREF clrEntity;				// The default color of point entities & brush entities, can be overridden by the FGD.
#ifdef SLE //// renamed for clarity
	COLORREF clr2DSelection;		// The color of selected objects.
#else
	COLORREF clrSelection;			// The color of selected objects.
#endif
	COLORREF clrVertex;				// The color of vertices.
	COLORREF clrToolHandle;			// The color of tool handles.
	COLORREF clrToolBlock;			// The color of the block tool.
	COLORREF clrToolSelection;		// The color of the selection tool.
	COLORREF clrToolMorph;			// The color of the morph tool.
	COLORREF clrToolPath;			// The color of the path tool.
	COLORREF clrToolDrag;			// The color of tool bounds while it is being dragged.
	COLORREF clrModelCollisionWireframe;			// The color of a model's collision wireframe
	COLORREF clrModelCollisionWireframeDisabled;	// The color of a model's collision wireframe when set to "Not Solid" via the entity properties
#ifdef SLE
	COLORREF clr3DBackground;		//// SLE NEW: The selectable colour of background void
	COLORREF clr3DSelection;		//// SLE NEW: Configure the tint on selected objects
	COLORREF clr3DEdges;			//// SLE NEW: Configure the tint on selected objects
	COLORREF clrModelSelection;		//// SLE NEW: Configure the tint on selected studiomodels
	COLORREF clrMapBounds;			//// SLE NEW: Configure the tint of maximum map bounds in 2d viewport
	COLORREF clrInstances;			//// SLE NEW: Configure the tint of instances
	COLORREF clrInstancesSel;		//// SLE NEW: Configure the tint of instances (selected)
#endif
};

class COptionsView2D
{
public:

	BOOL bCrosshairs;
	BOOL bGroupCarve;
	BOOL bScrollbars;
	BOOL bRotateConstrain;
#ifdef SLE //// SLE NEW - rotation snap setting
	int  iRotateConstrainAngle;
#endif
	BOOL bDrawVertices;
	BOOL bDrawModels;
	BOOL bWhiteOnBlack;
	BOOL bGridHigh1024;
	BOOL bGridHigh10;
	BOOL bHideSmallGrid;
	BOOL bNudge;
	BOOL bOrientPrimitives;
	BOOL bAutoSelect;
	BOOL bSelectbyhandles;
	BOOL bKeepclonegroup;
#ifdef SLE //// SLE NEW - option to also keep visgroup when copy-pasting
	BOOL bKeepPasteGroup;
#endif
	BOOL bGridHigh64;
	BOOL bCenteroncamera;
	BOOL bUsegroupcolors;
	BOOL bGridDots;
	int iDefaultGrid;
	int iGridIntensity;
	int iGridHighSpec;
};

class COptionsView3D
{
public:
	BOOL bHardware;			// Whether to use hardware acceleration (disabled for OpenGL).
	BOOL bReverseY;			// Wether to reverse the mouse's Y axis when mouse looking.
	BOOL bUseMouseLook;		// Whether to use the engine's default movement controls.
	int iBackPlane;			// Distance to far clipping plane in world units.
	int nModelDistance;		// Distance in world units within which studio models render.
	int nDetailDistance;	// Distance in world units within which detail props render.
	BOOL bAnimateModels;	// Whether to animate studio models.
	int nForwardSpeedMax;	// Max forward speed in world units per second.
	int nTimeToMaxSpeed;	// Time to max forward speed in milliseconds.
	BOOL bFilterTextures;	// Whether to filter textures.
	BOOL bReverseSelection;	// Whether to animate studio models.
	bool bPreviewModelFade; // Whether to preview model fade in the 3D view.
	float fFOV;				// FOV of 3D Camera
#ifdef SLE
	BOOL bAnimateModelsToggle;	//// fast toggle for the View menu, not saved 
	bool bRenderSprites; //// SLE NEW: allow to hide sprites in 3d (only show the little cube and not the glowing texture)
	int nLOD; //// SLE NEW - LOD control option
	int nPicmip; //// SLE NEW - Picmip control option
	bool bSolidsEdgesNoZ;	//// SLE NEW - option to render selected solids' edges in wireframe noz
	int iMaterialCacheSize; //// SLE NEW - customisable material cache size for the mat browser
	BOOL bMissingMatAsError; //// SLE NEW - ported from sdk-2013-hammer - display missing texture as emo checkerboard
#else
	float fLightConeLength;  // Multiplier for light_spot cone length //// SLE REMOVE: Does anybody ever use this setting? 
#endif
};

class COptionsConfigs
{
public:

	COptionsConfigs(void);
	virtual ~COptionsConfigs(void);

	CGameConfig *AddConfig(void);

	int		LoadGameConfigs();
	void	SaveGameConfigs();
	bool	ResetGameConfigs( bool bOverwrite );

	inline int GetGameConfigCount();
	inline CGameConfig *GetGameConfig(int nIndex);

	// find a game config based on ID:
	CGameConfig *FindConfig(DWORD dwID, int *piIndex = NULL);
	CGameConfig *FindConfigForGame(const char *szGame);

	int nConfigs;
	CTypedPtrArray<CPtrArray, CGameConfig*> Configs;
	CString m_strConfigDir;

private:

	int		LoadGameConfigsBlock( KeyValues *pBlock );
	int		ImportOldGameConfigs(const char *pszFileName);
};

//-----------------------------------------------------------------------------
// Purpose: Functions for iterating the game configs.
//-----------------------------------------------------------------------------
int COptionsConfigs::GetGameConfigCount()
{
	return nConfigs;
}

CGameConfig *COptionsConfigs::GetGameConfig(int nIndex)
{
	return Configs.GetAt(nIndex);
}

class COptions
{
public:

	COptions();

	bool Init();
	bool Read();
	void SetDefaults();
	void Write( BOOL fOverwrite, BOOL fSaveConfigs );
	void SetClosedCorrectly(BOOL bClosed);

	// This happens if it can't initialize the file system, or if they're missing gameinfo.txt.
	// It forces them to choose or create a config that works.
	bool RunConfigurationDialog();

	void PerformChanges(DWORD = 0xffffffff);

	// Accessors:
	TextureAlignment_t GetTextureAlignment(void);
	TextureAlignment_t SetTextureAlignment(TextureAlignment_t eTextureAlignment);

	bool GetShowHelpers(void);
	void SetShowHelpers(bool bShow);

	BOOL SetLockingTextures(BOOL b);
	BOOL IsLockingTextures(void);

	BOOL SetScaleLockingTextures(BOOL b);
	BOOL IsScaleLockingTextures(void);
#ifdef SLE
	BOOL SetDispVertexLock(BOOL b);  //// SLE NEW: button toggle, works as holding shift w/ moving vertices in disp tool. (locks selection on vertex until you let go lmb)
	BOOL IsDispVertexLockEnabled(void);
#endif
	bool IsVGUIModelBrowserEnabled();

	// Attributes:
	UINT uDaysSinceInstalled;

	// Flags for PerformChanges:
	enum
	{
		secTextures = 0x01,
		secGeneral = 0x02,
		secView2D = 0x04,
		secView3D = 0x08,
		secConfigs = 0x10,
#ifdef SLE
		secColors = 0x20
#endif
	};

	COptionsGeneral general;
	COptionsView2D view2d;
	COptionsView3D view3d;
	COptionsTextures textures;
	COptionsConfigs configs;
	COptionsColors colors;
#ifdef SLE //// SLE NEW - disp free drag brush, used for sampling mouse pos to determine mouse movement
	POINT m_storedMousePos;
#endif
private:

	void ReadColorSettings(void);
#ifdef SLE //// SLE NEW: Custom colour scheme support.
	void WriteColorSettings(void);
#endif
};

extern COptions Options;

#endif // OPTIONS_H