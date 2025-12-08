//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include <stdafx.h>
#include "hammer.h"
#include "IEditorTexture.h"
#include "FaceEditSheet.h"
#include "MapFace.h"
#include "MapWorld.h"
#include "MainFrm.h"
#include "History.h"
#include "GlobalFunctions.h"
#include "mathlib/vmatrix.h"
#include "MapSolid.h"
#include "TextureBrowser.h"
#include "TextureSystem.h"
#include "MapView3D.h"
#include "ReplaceTexDlg.h"
#include "WADTypes.h"
#include "FaceEdit_MaterialPage.h"
#include "Camera.h"
#include "MapDoc.h"
#include "MapDisp.h"
#include "ToolManager.h"
#include "Selection.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

//=============================================================================

IMPLEMENT_DYNAMIC( CFaceEditMaterialPage, CPropertyPage )

BEGIN_MESSAGE_MAP( CFaceEditMaterialPage, CPropertyPage )
	//{{AFX_MSG_MAP( CFaceEditMaterialPage )
	ON_BN_CLICKED( ID_FACEEDIT_APPLY, OnButtonApply )
	ON_COMMAND_EX( IDC_ALIGN_WORLD, OnAlign )
	ON_COMMAND_EX( IDC_ALIGN_FACE, OnAlign )
	ON_BN_CLICKED( IDC_HIDEMASK, OnHideMask )
	ON_COMMAND_EX( IDC_JUSTIFY_TOP, OnJustify )
	ON_COMMAND_EX( IDC_JUSTIFY_BOTTOM, OnJustify )
	ON_COMMAND_EX( IDC_JUSTIFY_LEFT, OnJustify )
	ON_COMMAND_EX( IDC_JUSTIFY_CENTER, OnJustify )
	ON_COMMAND_EX( IDC_JUSTIFY_RIGHT, OnJustify )
#ifndef SLE //// SLE CHANGE - Fit now works as an expand list for extra Fit modes
	ON_COMMAND_EX( IDC_JUSTIFY_FITTOFACE, OnJustify )
#else
	ON_BN_CLICKED(IDC_JUSTIFY_FITTOFACE, OnFitExpand)
#endif
#ifdef SLE //// SLE NEW - randomise texture shift buttons
	ON_BN_CLICKED( IDC_SHIFTXRANDOM, OnShiftXRandom )
	ON_BN_CLICKED( IDC_SHIFTYRANDOM, OnShiftYRandom )
#endif
	ON_BN_CLICKED( IDC_MODE, OnMode )
	ON_WM_VSCROLL()
	ON_NOTIFY( UDN_DELTAPOS, IDC_SPINSCALEX, OnDeltaPosFloatSpin )
	ON_NOTIFY( UDN_DELTAPOS, IDC_SPINSCALEY, OnDeltaPosFloatSpin )
	ON_WM_SIZE()
	ON_CBN_SELCHANGE( IDC_TEXTURES, OnSelChangeTexture )
#ifndef SLE //// SLE REMOVE - unlikely to be used, but could return if Q2 maps are going to be more supported
	ON_BN_CLICKED( IDC_Q2_LIGHT, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_SLICK, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_SKY, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_WARP, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_TRANS33, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_TRANS66, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_FLOWING, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_NODRAW, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_SOLID, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_WINDOW, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_AUX, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_LAVA, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_SLIME, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_WATER, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_MIST, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_CURRENT90, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_CURRENT180, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_CURRENT270, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_CURRENTUP, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_CURRENTDN, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_ORIGIN, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_MONSTER, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_CORPSE, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_DETAIL, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_TRANSLUCENT, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_LADDER, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_PLAYERCLIP, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_MONSTERCLIP, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_CURRENT0, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_HINT, OnCheckUnCheck )
	ON_BN_CLICKED( IDC_Q2_SPLITTER, OnCheckUnCheck )
#endif
	ON_COMMAND( IDC_TREAT_AS_ONE, OnTreatAsOne )
	ON_BN_CLICKED( IDC_REPLACE, OnReplace )
	ON_COMMAND_EX_RANGE( CFaceEditSheet::id_SwitchModeStart, CFaceEditSheet::id_SwitchModeEnd, OnSwitchMode )
#ifdef SLE //// SLE NEW - extra Fit modes
	ON_COMMAND_EX_RANGE( CFaceEditSheet::id_FitModeStart, CFaceEditSheet::id_FitModeEnd, OnFitMode )
#endif
	ON_CBN_SELCHANGE( IDC_TEXTUREGROUPS, OnChangeTextureGroup )
	ON_BN_CLICKED( IDC_BROWSE, OnBrowse )
	ON_BN_CLICKED( ID_BUTTON_SMOOTHING_GROUPS, OnButtonSmoothingGroups )
#ifdef SLE
	ON_BN_CLICKED(IDC_FACEEDIT_APPLYNODRAW, OnButtonApplyNodraw) //// SLE NEW - button to texture selected faces as nodraw 
	ON_BN_CLICKED(IDC_FACEEDIT_MARK, OnButtonMarkAll) //// SLE NEW - mark button on texture tool, without needing to open texture browser every time
	ON_BN_CLICKED(IDC_FACEEDIT_GROW, OnGrowSelection) //// SLE NEW - grow/shrink face selection
	ON_BN_CLICKED(IDC_FACEEDIT_SHRINK, OnShrinkSelection) //// SLE NEW - grow/shrink face selection
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPINDOT, OnDeltaPosFloatSpin)
	ON_COMMAND(IDC_FACEEDIT_PICKBYANGLE, OnPickByAngle)
	//// SLE NEW - texture shear
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPINSHEARX, OnDeltaPosFloatSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPINSHEARY, OnDeltaPosFloatSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPINSHEARZ, OnDeltaPosFloatSpin)
	ON_COMMAND(IDC_FACEEDIT_SHEARLOCK, OnToggleShearLock)
	//// SLE NEW - texture shift step control
	ON_NOTIFY( UDN_DELTAPOS, IDC_SPINSHIFTX, OnDeltaPosFloatSpin )
	ON_NOTIFY( UDN_DELTAPOS, IDC_SPINSHIFTY, OnDeltaPosFloatSpin )
#endif
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define	CONTENTS_AREAPORTAL		0x8000
#define	CONTENTS_PLAYERCLIP		0x10000
#define	CONTENTS_MONSTERCLIP	0x20000

// I don't think we need these currents.  We'll stick to triggers for this
#define	CONTENTS_CURRENT_0		0x40000
#define	CONTENTS_CURRENT_90		0x80000
#define	CONTENTS_CURRENT_180	0x100000
#define	CONTENTS_CURRENT_270	0x200000
#define	CONTENTS_CURRENT_UP		0x400000
#define	CONTENTS_CURRENT_DOWN	0x800000
#define	CONTENTS_ORIGIN			0x1000000	// removed before bsping an entity
#define	CONTENTS_MONSTER		0x2000000	// should never be on a brush, only in game
#define	CONTENTS_DEBRIS			0x4000000
#define	CONTENTS_DETAIL			0x8000000	// brushes to be added after vis leafs
#define	CONTENTS_TRANSLUCENT	0x10000000	// auto set if any surface has trans
#define	CONTENTS_LADDER			0x20000000

const int NOT_INIT = -99999;

unsigned int CFaceEditMaterialPage::m_FaceContents = 0;
unsigned int CFaceEditMaterialPage::m_FaceSurface = 0;

//-----------------------------------------------------------------------------
// This table defines the mapping of checkbox controls to flags which are set
// in certain face attributes values.
//-----------------------------------------------------------------------------
CFaceEditMaterialPage::FaceAttributeInfo_t FaceAttributes[] =
{
	//
	// Contents.
	//
	{ IDC_CONTENTS_AREAPORTAL,	&CFaceEditMaterialPage::m_FaceContents,	CONTENTS_AREAPORTAL },
	{ IDC_CONTENTS_PLAYERCLIP,	&CFaceEditMaterialPage::m_FaceContents,	CONTENTS_PLAYERCLIP },
	{ IDC_CONTENTS_MONSTERCLIP,	&CFaceEditMaterialPage::m_FaceContents,	CONTENTS_MONSTERCLIP },
	{ IDC_CONTENTS_ORIGIN,		&CFaceEditMaterialPage::m_FaceContents,	CONTENTS_ORIGIN },
	{ IDC_CONTENTS_DETAIL,		&CFaceEditMaterialPage::m_FaceContents,	CONTENTS_DETAIL },
	{ IDC_CONTENTS_TRANSLUCENT,	&CFaceEditMaterialPage::m_FaceContents,	CONTENTS_TRANSLUCENT },
	{ IDC_CONTENTS_LADDER,		&CFaceEditMaterialPage::m_FaceContents,	CONTENTS_LADDER },

	//
	// Surface attributes.
	//
	{ IDC_SURF_NODRAW,			&CFaceEditMaterialPage::m_FaceSurface,		SURF_NODRAW },
	{ IDC_SURF_HINT,			&CFaceEditMaterialPage::m_FaceSurface,		SURF_HINT },
	{ IDC_SURF_SKIP,			&CFaceEditMaterialPage::m_FaceSurface,		SURF_SKIP }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CFaceEditMaterialPage::CFaceEditMaterialPage() : CPropertyPage( IDD )
{
	m_bHideMask = FALSE;
	m_bInitialized = FALSE;
	m_bIgnoreResize = FALSE;
	m_bTreatAsOneFace = FALSE;
#ifdef SLE
	m_growIgnoreList.RemoveAll();
	m_growAngle_float = 0.0f;
	m_pickByAngle_bool = FALSE;
	m_shearLock_bool = FALSE;
#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CFaceEditMaterialPage::~CFaceEditMaterialPage()
{
#ifdef SLE
	m_growIgnoreList.RemoveAll();
#endif
}
#ifdef SLE
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::OnClose()
{
#ifdef SLE
	m_growIgnoreList.RemoveAll();
#endif
	DestroyWindow();
}

BOOL CFaceEditMaterialPage::OnKillActive()
{
	m_cHideMask.SetCheck(FALSE);
	if (true) OnHideMask();
#ifdef SLE
	m_bHideMask = FALSE; //// SLE NEW: upon closing texture tool dialogue, reset Hide Mask toggle.
									   //// doing this helps with things like cubemap face picker,
									   //// because often you will texture things with
									   //// hide mask on, then switch tools, later want to pick 
									   //// faces with cubemap or overlay tool, and won't see 
									   //// what you're picking, and need to open texture tool again,
									   //// but that closes your active tool (cubemap/overlay)... in short, ANNOYING.
	
	m_growIgnoreList.RemoveAll();
#endif
	CMapFace::SetShowSelection(true);
	return CPropertyPage::OnKillActive();
}
#endif
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL CFaceEditMaterialPage::PreTranslateMessage( MSG *pMsg )
{
#ifdef SLE
	if ( pMsg->message == WM_KEYDOWN )
	{
		if( m_shearLock_bool )
			Apply(NULL, FACE_APPLY_ALL | FACE_APPLY_SHEAR);
		else
			Apply(NULL, FACE_APPLY_ALL );
	}
#endif
	HACCEL hAccel = GetMainWnd()->GetAccelTable();
	if( !(hAccel && ::TranslateAccelerator( GetMainWnd()->m_hWnd, hAccel, pMsg ) ) )
	{
		return CPropertyPage::PreTranslateMessage( pMsg );
	}
	else
	{
		return TRUE;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::Init( void )
{
	//
	// Connect dialog control objects to their control IDs.
	//
	m_shiftX.SubclassDlgItem( IDC_SHIFTX, this );
	m_shiftY.SubclassDlgItem( IDC_SHIFTY, this );
	m_scaleX.SubclassDlgItem( IDC_SCALEX, this );
	m_scaleY.SubclassDlgItem( IDC_SCALEY, this );
	m_rotate.SubclassDlgItem( IDC_ROTATE, this );
	m_cHideMask.SubclassDlgItem( IDC_HIDEMASK, this );
	m_cExpand.SubclassDlgItem( IDC_EXPAND, this );
	m_cLightmapScale.SubclassDlgItem( IDC_LIGHTMAP_SCALE, this );

#ifdef SLE //// SLE NEW - texture shear
	m_shearX.SubclassDlgItem(IDC_FACE_SHEARX, this);
	m_shearY.SubclassDlgItem(IDC_FACE_SHEARY, this);
	m_shearZ.SubclassDlgItem(IDC_FACE_SHEARZ, this);
	
	//// SLE NEW - grow/shrink face selection
	m_cGrowAngle.SubclassDlgItem(IDC_FACEEDIT_DOT, this);
	m_cGrowButton.SubclassDlgItem(IDC_FACEEDIT_GROW, this);
	m_cShrinkButton.SubclassDlgItem(IDC_FACEEDIT_SHRINK, this);
	
	//// SLE NEW - texture shift step control
	m_shiftStepX.SubclassDlgItem(IDC_SHIFTSTEPX, this);
	m_shiftStepY.SubclassDlgItem(IDC_SHIFTSTEPY, this);

	m_shiftStepX.SetWindowText("1"); // todo - hook up to registry-saved
	m_shiftStepY.SetWindowText("1");
#endif

	//
	// Set spin ranges.
	//
	CWnd *pWnd = GetDlgItem(IDC_SPINSHIFTX);
	::PostMessage(pWnd->m_hWnd, UDM_SETRANGE, 0, MAKELONG(4096, -4096)); //// SLE TODO - bump these up to 8192?

	pWnd = GetDlgItem(IDC_SPINSHIFTY);
	::PostMessage(pWnd->m_hWnd, UDM_SETRANGE, 0, MAKELONG(4096, -4096));

	pWnd = GetDlgItem(IDC_SPINROTATE);
	::PostMessage(pWnd->m_hWnd, UDM_SETRANGE, 0, MAKELONG(359, -359));

	pWnd = GetDlgItem(IDC_SPINSCALEX);
	::PostMessage(pWnd->m_hWnd, UDM_SETRANGE, 0, MAKELONG(UD_MAXVAL, UD_MINVAL));

	pWnd = GetDlgItem(IDC_SPINSCALEY);
	::PostMessage(pWnd->m_hWnd, UDM_SETRANGE, 0, MAKELONG(UD_MAXVAL, UD_MINVAL));

	pWnd = GetDlgItem(IDC_SPIN_LIGHTMAP_SCALE);
	::PostMessage(pWnd->m_hWnd, UDM_SETRANGE, 0, MAKELONG(256, 1));

#ifdef SLE	//// SLE NEW - grow/shrink face selection
	pWnd = GetDlgItem(IDC_SPINDOT);
	::PostMessage(pWnd->m_hWnd, UDM_SETRANGE, 0, MAKELONG(180, -180));
	
	//// SLE NEW - texture shear
	pWnd = GetDlgItem(IDC_SPINSHEARX);
	::PostMessage(pWnd->m_hWnd, UDM_SETRANGE, 0, MAKELONG(100, -100));

	pWnd = GetDlgItem(IDC_SPINSHEARY);
	::PostMessage(pWnd->m_hWnd, UDM_SETRANGE, 0, MAKELONG(100, -100));

	pWnd = GetDlgItem(IDC_SPINSHEARZ);
	::PostMessage(pWnd->m_hWnd, UDM_SETRANGE, 0, MAKELONG(100, -100));
#endif
	// set the initial switch mode
	OnSwitchMode( CFaceEditSheet::ModeLiftSelect );

	//
	// set up controls
	//
	m_TextureGroupList.SubclassDlgItem( IDC_TEXTUREGROUPS, this );
	m_TextureList.SubclassDlgItem( IDC_TEXTURES, this );
	m_TexturePic.SubclassDlgItem( IDC_TEXTUREPIC, this );

	m_pCurTex = NULL;

	//
	// initially update the texture controls
	//
	NotifyGraphicsChanged();
	UpdateTexture();

	// initialized!
	m_bInitialized = TRUE;
}

//-----------------------------------------------------------------------------
// NOTE: clean this up and make a global function!!!
// Purpose: 
// Input  : fValue - 
//			*pSpin - 
//			bMantissa - 
// Output : static void
//-----------------------------------------------------------------------------
void FloatToSpin(float fValue, CSpinButtonCtrl *pSpin, BOOL bMantissa)
{
	char szNew[128];

	CWnd *pEdit = pSpin->GetBuddy();

	if (fValue == NOT_INIT)
	{
		szNew[0] = 0;
	}
	else
	{
		if(bMantissa)
			sprintf(szNew, "%.2f", fValue);
		else
			sprintf(szNew, "%.0f", fValue);
	}

	pSpin->SetPos(atoi(szNew));

	char szCurrent[128];
	pEdit->GetWindowText(szCurrent, 128);
	if (strcmp(szNew, szCurrent))
	{
		pEdit->SetWindowText(szNew);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nValue - 
//			pSpin - 
// Output : static void
//-----------------------------------------------------------------------------
void IntegerToSpin(int nValue, CSpinButtonCtrl *pSpin)
{
	char szNew[128];

	CWnd *pEdit = pSpin->GetBuddy();

	if (nValue == NOT_INIT)
	{
		szNew[0] = 0;
	}
	else
	{
		sprintf(szNew, "%d", abs(nValue));
	}

	pSpin->SetPos(atoi(szNew));

	char szCurrent[128];
	pEdit->GetWindowText(szCurrent, 128);
	if (strcmp(szNew, szCurrent) != 0)
	{
		pEdit->SetWindowText(szNew);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : fValue - 
//			*pWnd - 
// Output : static void
//-----------------------------------------------------------------------------
void FloatToWnd(float fValue, CWnd *pWnd)
{
	char szCurrent[128];
	char szNew[128];

	if(fValue == NOT_INIT)
	{
		szNew[0] = 0;
	}
	else
	{
		sprintf(szNew, "%g", fValue);
	}

	pWnd->GetWindowText(szCurrent, 128);
	if(strcmp(szNew, szCurrent))
		pWnd->SetWindowText(szNew);
}

//-----------------------------------------------------------------------------
// Purpose: Fetches a string value from a window and places it in a float. The
//			float is only modified if there is text in the window.
// Input  : pWnd - Window from which to get text.
//			fValue - Float in which to place value.
//-----------------------------------------------------------------------------
void TransferToFloat( CWnd *pWnd, float &fValue )
{
	CString str;
	pWnd->GetWindowText( str );

	if( !str.IsEmpty() )
	{
		fValue = ( float )atof( str );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Fetches a string value from a window and places it in an integer. The
//			integer is only modified if there is text in the window.
// Input  : pWnd - Window from which to get text.
//			nValue - Float in which to place value.
//-----------------------------------------------------------------------------
void TransferToInteger( CWnd *pWnd, int &nValue )
{
	CString str;
	pWnd->GetWindowText( str );

	if( !str.IsEmpty() )
	{
		nValue = abs( atoi( str ) );
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifdef SLE //// SLE CHANGE - add argument for manual vs automatic clicking, because of m_pickByAngle_bool
void CFaceEditMaterialPage::ClickFace( CMapSolid *pSolid, int faceIndex, int cmd, int clickMode, bool manual )
#else
void CFaceEditMaterialPage::ClickFace( CMapSolid *pSolid, int faceIndex, int cmd, int clickMode )
#endif
{
	// get the face
	CMapFace	*pFace = pSolid->GetFace( faceIndex );
	bool		bIsEditable = pSolid->IsEditable();

	//
	// are updates enabled?
	//
	CFaceEditSheet *pSheet = ( CFaceEditSheet* )GetParent();
	bool bEnableUpdate = pSheet->HasUpdateEnabled();
	
	SetReadOnly( !bIsEditable );

#ifdef SLE //// SLE NEW - grow/shrink face selection
	if( manual) OnClickPickByAngle(pFace, cmd, clickMode);
#endif

	//
	// find the behavior of the page based on the "click mode"
	//
	switch( clickMode )
	{
		case CFaceEditSheet::ModeAlignToView:
		{
			if ( bIsEditable )
			{
				AlignToView( pFace );
			}
			break;
		}

		case CFaceEditSheet::ModeLift:
		{
			if( bEnableUpdate )
			{
				UpdateDialogData( pFace );			
			}
			break;
		}

		case CFaceEditSheet::ModeLiftSelect:
		{
			if ( bEnableUpdate )
			{
				UpdateDialogData();			
			}
			break;
		}

		case CFaceEditSheet::ModeApplyLightmapScale:
		{
			// Apply the lightmap scale only. Leave everything else alone.
			if ( bIsEditable )
			{
				Apply(pFace, FACE_APPLY_LIGHTMAP_SCALE);
			}
			break;
		}

		case CFaceEditSheet::ModeApply:
		case CFaceEditSheet::ModeApplyAll:
		{
			if ( bIsEditable )
			{
				int flags = 0;

				if (cmd & CFaceEditSheet::cfEdgeAlign)
				{
					// Adust the mapping to align with a reference face.
					flags |= FACE_APPLY_ALIGN_EDGE;
				}

				if (clickMode == CFaceEditSheet::ModeApplyAll)
				{
					// Apply the material, mapping, lightmap scale, etc.
					flags |= FACE_APPLY_ALL;
				}
				else
				{
					// Apply the material only. Leave everything else alone.
					flags |= FACE_APPLY_MATERIAL;
				}

				Apply(pFace, flags);
			}
			break;
		}
	}
}
#ifdef SLE //// currently these don't do anything, but may get context menus later
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nFlags - 
//			point - 
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::OnRButtonUp(UINT nFlags, CPoint point)
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nFlags - 
//			point - 
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::OnRButtonDown(UINT nFlags, CPoint point)
{
}
#endif
//-----------------------------------------------------------------------------
// Maps the texture onto the face using the 3D view's up and right vectors.
// This can be useful for mapping curvy things like hills because if you don't
// move the 3D view, the texture will line up on any polies you map this way.
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::AlignToView( CMapFace *pFace )
{
	CView		*pActiveView;
	CMapView3D	*pView3D;
	CFrameWnd	*pFrame;
	Vector		vView;
	
	if((pFrame = GetMainWnd()->GetActiveFrame()) != NULL)
	{
		if((pActiveView = pFrame->GetActiveView()) != NULL)
		{
			if(pActiveView->IsKindOf(RUNTIME_CLASS(CMapView3D)))
			{
				pView3D = dynamic_cast<CMapView3D*>(pActiveView);
				if(pView3D)
				{
					const CCamera *pCamera = pView3D->GetCamera();
					if(pCamera)
					{
						Vector right, up;
						pCamera->GetViewRight(right);
						pCamera->GetViewUp(up);
						pCamera->GetViewPoint(vView);

						pFace->texture.UAxis.AsVector3D() = right;
						pFace->texture.VAxis.AsVector3D() = up;
						pFace->texture.UAxis[3] = DotProduct( right, vView);
						pFace->texture.VAxis[3] = DotProduct( up, vView);
						pFace->NormalizeTextureShifts();

						pFace->texture.rotate = 0.0f;
						pFace->texture.scale[0] = g_pGameConfig->GetDefaultTextureScale();
						pFace->texture.scale[1] = g_pGameConfig->GetDefaultTextureScale();

						pFace->CalcTextureCoords();
					}
				}
			}	
		}
	}
} 

//-----------------------------------------------------------------------------
// Copies the texture coordinate system from pFrom into pTo. Then it rotates 
// the texture around the edge until it's as close to pTo's normal as possible.
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::CopyTCoordSystem( const CMapFace *pFrom, CMapFace *pTo )
{
	Vector		axis[2], vEdge, vEdgePt, vOrigin;
	Vector		vFromPt, vNextFromPt;
	Vector		vToPt, vPrevToPt;
	Vector		vTestTextureNormal, vTextureNormal;
	VMatrix		mEdgeRotation, mOriginRotation, mTranslation;
	float		fAngle, fDot;
	bool		bRotate;
	float		fShift[2];
	Vector		vProjTexNormal;
	Vector		vProjPolyNormal;

	// The edge vector lies on both planes.
	vEdge = pFrom->plane.normal.Cross(pTo->plane.normal);
	VectorNormalize( vEdge );

	// To find a point on the plane, we make a plane from the edge vector and find the intersection
	// between the three planes (without the third plane, there are an infinite number of solutions).
	if( PlaneIntersection( VPlane(pFrom->plane.normal, pFrom->plane.dist),
		                   VPlane(pTo->plane.normal, pTo->plane.dist),
		                   VPlane(vEdge, 0.0f), vEdgePt ) )
	{
		bRotate = true;
	}
	else
	{
		// Ok, in this case, the planes are parallel so we don't need to rotate around the edge anyway!
		bRotate = false;
	}

	// Copy the texture coordinate system.
	axis[0] = pFrom->texture.UAxis.AsVector3D();
	axis[1] = pFrom->texture.VAxis.AsVector3D();
	fShift[0] = pFrom->texture.UAxis[3];
	fShift[1] = pFrom->texture.VAxis[3];
	vOrigin = axis[0]*fShift[0]*pFrom->texture.scale[0] + axis[1]*fShift[1]*pFrom->texture.scale[1];
	
	vTextureNormal = axis[0].Cross(axis[1]);
	VectorNormalize(vTextureNormal);
	if(bRotate)
	{
		// Project texture normal and poly normal into the plane of rotation
		// to get the angle between them.
		vProjTexNormal = vTextureNormal - vEdge * vEdge.Dot(vTextureNormal);
		vProjPolyNormal = pTo->plane.normal - vEdge * vEdge.Dot(pTo->plane.normal);

		VectorNormalize( vProjTexNormal );
		VectorNormalize( vProjPolyNormal );

		fDot = vProjTexNormal.Dot(vProjPolyNormal);
		fAngle = (float)(acos(fDot) * (180.0f / M_PI));
		if(fDot < 0.0f)
			fAngle = 180.0f - fAngle;

		// Ok, rotate them for the final values.
		mEdgeRotation = SetupMatrixAxisRot(vEdge, fAngle);
		axis[0] = mEdgeRotation.ApplyRotation(axis[0]);
		axis[1] = mEdgeRotation.ApplyRotation(axis[1]);

		// Origin needs translation and rotation to rotate around the edge.
		mTranslation = SetupMatrixTranslation(vEdgePt);
		mOriginRotation = ~mTranslation * mEdgeRotation * mTranslation;
		vOrigin = mOriginRotation * vOrigin;
	}

	pTo->texture.UAxis.AsVector3D() = axis[0];
	pTo->texture.VAxis.AsVector3D() = axis[1];

	pTo->texture.UAxis[3] = axis[0].Dot(vOrigin) / pFrom->texture.scale[0];
	pTo->texture.VAxis[3] = axis[1].Dot(vOrigin) / pFrom->texture.scale[1];
	pTo->NormalizeTextureShifts();

	pTo->texture.scale[0] = pFrom->texture.scale[0];
	pTo->texture.scale[1] = pFrom->texture.scale[1];

	// rotate is only for UI purposes, it doesn't actually do anything.
	pTo->texture.rotate = 0.0f;

	pTo->CalcTextureCoords();
}

//-----------------------------------------------------------------------------
// Purpose: Applies dialog data to the list of selected faces.
// Input  : *pOnlyFace - 
//			bAll - 
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::Apply( CMapFace *pOnlyFace, int flags )
{
	int			i;
	CString		str;
	float		fshiftX = NOT_INIT;
	float		fshiftY = NOT_INIT;
	float		fscaleX = NOT_INIT;
	float		fscaleY = NOT_INIT;
	float		frotate = NOT_INIT;
	int			material = NOT_INIT;
	int			nLightmapScale = NOT_INIT;

#ifdef SLE //// SLE NEW - texture shear
	float fshearX = NOT_INIT;
	float fshearY = NOT_INIT;
	float fshearZ = NOT_INIT;

	TransferToFloat(&m_cGrowAngle, m_growAngle_float);
#endif
	
	IEditorTexture	*pTex = m_TexturePic.GetTexture();
	CMapDoc		*pMapDoc = CMapDoc::GetActiveMapDoc();

	//
	// Get numeric data.
	//
	if (flags & FACE_APPLY_MAPPING)
	{
		TransferToFloat( &m_shiftX, fshiftX );
		TransferToFloat( &m_shiftY, fshiftY );
		TransferToFloat( &m_scaleX, fscaleX );
		TransferToFloat( &m_scaleY, fscaleY );
		TransferToFloat( &m_rotate, frotate );
#ifdef SLE //// SLE NEW - grow/shrink face selection
		TransferToFloat(&m_cGrowAngle, m_growAngle_float);
#endif
	}

	if ( flags & FACE_APPLY_SHEAR )
	{
#ifdef SLE //// SLE NEW - texture shear
		TransferToFloat(&m_shearX, fshearX);
		TransferToFloat(&m_shearY, fshearY);
		TransferToFloat(&m_shearZ, fshearZ);
#endif
	}

	if (flags & FACE_APPLY_LIGHTMAP_SCALE)
	{
		TransferToInteger( &m_cLightmapScale, nLightmapScale );
	}

	if ( !pOnlyFace )
	{
		GetHistory()->MarkUndoPosition( NULL, "Apply Face Attributes" );

		// make sure we apply everything in this case.
		flags |= FACE_APPLY_ALL;

		// Keep the solids that we are about to change.
		// In the pOnlyFace case we do the Keep before calling ClickFace. Why?
		CUtlVector<CMapSolid *> kept;
		CFaceEditSheet *pSheet = ( CFaceEditSheet* )GetParent();
		for( i = 0; i < pSheet->GetFaceListCount(); i++ )
		{
			CMapSolid *pSolid = pSheet->GetFaceListDataSolid( i );
			if ( kept.Find( pSolid ) == -1 )
			{
				GetHistory()->Keep( pSolid );
				kept.AddToTail( pSolid );
			}
		}
	}
		
	//
	// Run thru stored faces & apply.
	//
	CFaceEditSheet *pSheet = ( CFaceEditSheet* )GetParent();
	int faceCount = pSheet->GetFaceListCount();
	for( i = 0; i < faceCount || pOnlyFace; i++ )
	{
		CMapFace *pFace;
		if( pOnlyFace )
		{
			pFace = pOnlyFace;
		}
		else
		{
			pFace = pSheet->GetFaceListDataFace( i );
		}

#ifdef SLE //// SLE NEW - texture shear
		if (( flags & FACE_APPLY_MAPPING ) && ( ( flags & FACE_APPLY_SHEAR ) ) && faceCount == 1 ) // broken for multiple faces right now
		{
			if ( fshearX != NOT_INIT )
			{
				pFace->texture.UAxis[2] = fshearX;
			}
			if ( fshearY != NOT_INIT )
			{
				pFace->texture.VAxis[1] = fshearY;
			}
			if ( fshearZ != NOT_INIT )
			{
				pFace->texture.VAxis[0] = fshearZ;
			}
		}
#endif
		//
		// Get values for texture shift, scale, rotate, and material.
		//
		if ((flags & FACE_APPLY_MAPPING) && (!(flags & FACE_APPLY_ALIGN_EDGE)))
		{
			if ( fshiftX != NOT_INIT )
			{
				pFace->texture.UAxis[3] = fshiftX;
			}

			if ( fshiftY != NOT_INIT )
			{
				pFace->texture.VAxis[3] = fshiftY;
			}

			if ( fscaleX != NOT_INIT )
			{
				pFace->texture.scale[0] = fscaleX;
			}
			
			if ( fscaleY != NOT_INIT )
			{
				pFace->texture.scale[1] = fscaleY;
			}

			if ( frotate != NOT_INIT )
			{
				pFace->RotateTextureAxes( frotate - pFace->texture.rotate );
				pFace->texture.rotate = frotate;
			}
		}

		if (flags & FACE_APPLY_CONTENTS_DATA)
		{
			if ( material != NOT_INIT )
			{
				pFace->texture.material = material;
			}
		}

		if (flags & FACE_APPLY_LIGHTMAP_SCALE)
		{
			if (nLightmapScale != NOT_INIT)
			{
				pFace->texture.nLightmapScale = max( nLightmapScale, 1 );
			}
		}

		//
		// Update the texture and recalculate texture coordinates.
		//
		if ((flags & FACE_APPLY_MATERIAL) && (pTex != NULL))
		{
			char szCurrentTexName[MAX_PATH];
			char szNewTexName[MAX_PATH];

			pFace->GetTextureName( szCurrentTexName );
			pTex->GetShortName( szNewTexName );

			if( stricmp( szCurrentTexName, szNewTexName ) != 0 )
			{
				pFace->SetTexture( szNewTexName );

				CMapClass	*pParent = dynamic_cast< CMapClass * >( pFace->GetParent() );
				if ( pParent )
				{
					pMapDoc->RemoveFromAutoVisGroups( pParent );
					pMapDoc->AddToAutoVisGroup( pParent );
				}

#ifdef SLE	//// SLE NEW: apply %defaultscale if it is defined in the vmt.
				if ( g_pGameConfig->GetTextureFormat() == tfVMT )
				{
					bool findVar;
					IMaterialVar *pMaterialVar = pTex->GetMaterial()->FindVar("%defaultscale", &findVar, false);
					if ( findVar && pMaterialVar )
					{
						if ( fscaleX != NOT_INIT )
						{
							pFace->texture.scale[ 0 ] = pMaterialVar->GetFloatValueFast();
							pFace->texture.scale[ 1 ] = pMaterialVar->GetFloatValueFast();
						}
					}
				}
#endif
			}
		}

		//
		// Copy texture coordinate system.
		//
		if ((flags & FACE_APPLY_ALIGN_EDGE) && (faceCount >= 1))
		{
			CopyTCoordSystem( pSheet->GetFaceListDataFace( faceCount - 1 ), pFace );
		}

		//
		// Recalculate texture coordinates.
		//
		pFace->CalcTextureCoords();
		
		//
		// Update the face flags.
		//

		if (flags & FACE_APPLY_CONTENTS_DATA)
		{
			//
			// Copy the bits from this face into our variables.
			//
			m_FaceContents = pFace->texture.q2contents;
			m_FaceSurface = pFace->texture.q2surface;

			//
			// Update our variables based on the state of the checkboxes.
			//
			for( int nItem = 0; nItem < sizeof( FaceAttributes ) / sizeof( FaceAttributes[0] ); nItem++ )
			{
				CButton *pButton = ( CButton* )GetDlgItem( FaceAttributes[nItem].uControlID );
				if( pButton != NULL )
				{
					int nSet = pButton->GetCheck();

					if (nSet == 0)
					{
						*FaceAttributes[nItem].puAttribute &= ~FaceAttributes[nItem].uFlag;
					}
					else if (nSet == 1)
					{
						*FaceAttributes[nItem].puAttribute |= FaceAttributes[nItem].uFlag;
					}
				}	
			}

			//
			// Copy our variables back into this face.
			//
			pFace->texture.q2contents = m_FaceContents;
			pFace->texture.q2surface = m_FaceSurface;
		}

		if( pOnlyFace )
		{
			break;
		}
	}

	pMapDoc->SetModifiedFlag();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOnlyFace - 
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::UpdateDialogData( CMapFace *pOnlyFace )
{
	BOOL	bFirst;
	int		nFaceAlignCount;
	int		nWorldAlignCount;
	float	fshiftX = NOT_INIT;
	float	fshiftY = NOT_INIT;
	float	fscaleX = NOT_INIT;
	float	fscaleY = NOT_INIT;
	float	frotate = NOT_INIT;
	//float fsmooth = NOT_INIT;
	int		material = NOT_INIT;
	int		nLightmapScale = NOT_INIT;
	CString strTexture;

#ifdef SLE //// SLE NEW - texture shear
	float fshearX = NOT_INIT;
	float fshearY = NOT_INIT;
	float fshearZ = NOT_INIT;
#endif

	bFirst = TRUE;
	nFaceAlignCount = 0;
	nWorldAlignCount = 0;

	CFaceEditSheet *pSheet = ( CFaceEditSheet* )GetParent();
	int faceCount = pSheet->GetFaceListCount();

	for( int i = 0; i < faceCount || pOnlyFace; i++ )
	{
		CMapFace *pFace;

		if( pOnlyFace )
		{
			pFace = pOnlyFace;
		}
		else
		{
			pFace = pSheet->GetFaceListDataFace( i );
		}
		
		TEXTURE &t = pFace->texture;

		//
		// Gather statistics about the texture alignment of all the selected faces.
		// This is used later to set the state of the alignment checkboxes.
		//
		int nAlignment = pFace->GetTextureAlignment();
		if (nAlignment & TEXTURE_ALIGN_FACE)
		{
			nFaceAlignCount++;
		}

		if (nAlignment & TEXTURE_ALIGN_WORLD)
		{
			nWorldAlignCount++;
		}

		//
		// First update - copy first face's stuff into edit fields.
		//
		if (bFirst)
		{
			fshiftX = t.UAxis[3];
			fshiftY = t.VAxis[3];
			fscaleX = t.scale[0];
			fscaleY = t.scale[1];
			frotate = t.rotate;
			material = t.material;
			strTexture = t.texture;
			nLightmapScale = t.nLightmapScale;

#ifdef SLE //// SLE NEW - texture shear
			fshearX = t.UAxis[2];
			fshearY = t.VAxis[1];
			fshearZ = t.VAxis[0];
#endif
			//
			// Get the face's orientation. This is used by Apply to make intelligent decisions.
			//
			m_eOrientation = pFace->GetOrientation();
			Assert(m_eOrientation != FACE_ORIENTATION_INVALID);

			//
			// Set the appropriate checkbox state for the face attributes.
			//
			m_FaceContents = t.q2contents;
			m_FaceSurface = t.q2surface;

			for (int nItem = 0; nItem < sizeof(FaceAttributes) / sizeof(FaceAttributes[0]); nItem++)
			{
				int nSet = ((*FaceAttributes[nItem].puAttribute & FaceAttributes[nItem].uFlag) != 0);
				CButton *pButton = (CButton *)GetDlgItem(FaceAttributes[nItem].uControlID);
				if (pButton != NULL)
				{
					pButton->SetCheck(nSet);
				}
			}

			bFirst = FALSE;
	
			if (pOnlyFace)	// use one face - now break
			{	
				break;
			}
		}
		else
		{
			// update fields with face's data
			if (t.UAxis[3] != fshiftX)
			{
				fshiftX = NOT_INIT;
			}

			if (t.VAxis[3] != fshiftY)
			{
				fshiftY = NOT_INIT;
			}

			if (t.scale[0] != fscaleX)
			{
				fscaleX = NOT_INIT;
			}

			if (t.scale[1] != fscaleY)
			{
				fscaleY = NOT_INIT;
			}

			if (t.rotate != frotate)
			{
				frotate = NOT_INIT;
			}

			if (t.material != material)
			{
				material = NOT_INIT;
			}

			if (t.nLightmapScale != nLightmapScale)
			{
				nLightmapScale = NOT_INIT;
			}

#ifdef SLE //// SLE NEW - texture shear
			/* // not needed, this messes up controls
			if ( t.UAxis[0] != fuaxis0 )
			{
			//	fuaxis0 = NOT_INIT;
			}
			if ( t.UAxis[1] != fuaxis1 )
			{
			//	fuaxis1 = NOT_INIT;
			}
			if ( t.UAxis[2] != fuaxis2 )
			{
			//	fuaxis2 = NOT_INIT;
			}
			//if ( t.UAxis[ 3 ] != fuaxis3 )
			{
			//	fuaxis3 = NOT_INIT;
			}
			if ( t.VAxis[0] != fvaxis0 )
			{
			//	fvaxis0 = NOT_INIT;
			}
			if ( t.VAxis[1] != fvaxis1 )
			{
			//	fvaxis1 = NOT_INIT;
			}
			if ( t.VAxis[2] != fvaxis2 )
			{
			//	fvaxis2 = NOT_INIT;
			}
			//if ( t.VAxis[ 3 ] != fvaxis3 )
			{
			//	fvaxis3 = NOT_INIT;
			}
			*/
#endif
			if (!strTexture.IsEmpty() && strTexture != t.texture)
			{
				strTexture = "";
			}

			//
			// Update the checkbox state for the face attributes. If any of this face's
			// attributes are different from the current checkbox state, set the checkbox
			// to the undefined state.
			//
			m_FaceContents = t.q2contents;
			m_FaceSurface = t.q2surface;

			for (int nItem = 0; nItem < sizeof(FaceAttributes) / sizeof(FaceAttributes[0]); nItem++)
			{
				int nSet = ((*FaceAttributes[nItem].puAttribute & FaceAttributes[nItem].uFlag) != 0);
				CButton *pButton = (CButton *)GetDlgItem(FaceAttributes[nItem].uControlID);
				if (pButton != NULL)
				{
					if (pButton->GetCheck() != nSet)
					{
						pButton->SetButtonStyle(BS_AUTO3STATE);
						pButton->SetCheck(2);
					}
				}
			}
		}
	}

	//
	// Set the state of the face alignment checkbox.
	//
	CButton *pFaceAlign = (CButton *)GetDlgItem(IDC_ALIGN_FACE);

	if (nFaceAlignCount == 0)
	{
		pFaceAlign->SetCheck(0);
	}
	else if (nFaceAlignCount == faceCount)
	{
		pFaceAlign->SetCheck(1);
	}
	else
	{
		pFaceAlign->SetCheck(2);
	}

	//
	// Set the state of the world alignment checkbox.
	//
	CButton *pWorldAlign = (CButton *)GetDlgItem(IDC_ALIGN_WORLD);

	if (nWorldAlignCount == 0)
	{
		pWorldAlign->SetCheck(0);
	}
	else if (nWorldAlignCount == faceCount)
	{
		pWorldAlign->SetCheck(1);
	}
	else
	{
		pWorldAlign->SetCheck(2);
	}

	//
	// Set up fields.
	//
#ifdef SLE //// SLE NEW - texture shift step control
	// needs to be FloatToWnd so OnDeltaPosFloatSpin code works
	FloatToWnd(fshiftX, &m_shiftX);
	FloatToWnd(fshiftY, &m_shiftY);
#else
	FloatToSpin(fshiftX, (CSpinButtonCtrl*)GetDlgItem(IDC_SPINSHIFTX), FALSE);
	FloatToSpin(fshiftY, (CSpinButtonCtrl*)GetDlgItem(IDC_SPINSHIFTY), FALSE);
#endif
	IntegerToSpin(nLightmapScale, (CSpinButtonCtrl *)GetDlgItem(IDC_SPIN_LIGHTMAP_SCALE));

	FloatToWnd(fscaleX, &m_scaleX);
	FloatToWnd(fscaleY, &m_scaleY);

#ifdef SLE //// SLE NEW - texture shear
	FloatToWnd(fshearX, &m_shearX);
	FloatToWnd(fshearY, &m_shearY);
	FloatToWnd(fshearZ, &m_shearZ);

//	FloatToSpin(fshearX, ( CSpinButtonCtrl* )GetDlgItem(IDC_SPINSHEARX), TRUE);
//	FloatToSpin(fshearY, ( CSpinButtonCtrl* )GetDlgItem(IDC_SPINSHEARY), TRUE);
//	FloatToSpin(fshearZ, ( CSpinButtonCtrl* )GetDlgItem(IDC_SPINSHEARZ), TRUE);
	
	//// SLE NEW - grow/shrink face selection
	FloatToSpin(m_growAngle_float, ( CSpinButtonCtrl* )GetDlgItem(IDC_SPINDOT), TRUE);
#endif

	FloatToSpin(frotate, ( CSpinButtonCtrl* )GetDlgItem(IDC_SPINROTATE), TRUE);

	if (!strTexture.IsEmpty())
	{
		SelectTexture( strTexture );
	}
	else
	{
		// make empty
		m_TextureList.SetCurSel( -1 );
	}

	//
	// if no faces selected -- get selection from texture bar
	//
	if( faceCount == 0 )
	{
		CString strTexName = GetDefaultTextureName();
		SelectTexture( strTexName );
	}

	//
	// Call ctexturebar implementation because OUR implementation sets the 
	// q2 checkboxes, which flashes the screen a bit (cuz we change them
	// again three lines down.)
	//
	UpdateTexture();

	// Update the smoothing group data.
	if ( GetMaterialPageTool() == MATERIALPAGETOOL_SMOOTHING_GROUP )
	{
		m_FaceSmoothDlg.UpdateControls();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : uCmd - 
// Output : Returns TRUE on success, FALSE on failure.
//-----------------------------------------------------------------------------
BOOL CFaceEditMaterialPage::OnAlign( UINT uCmd )
{
	// Set the material tool current.
	SetMaterialPageTool( MATERIALPAGETOOL_MATERIAL );

	// mark position in undo stack
	GetHistory()->MarkUndoPosition(NULL, "Align texture");

	CFaceEditSheet *pSheet = ( CFaceEditSheet* )GetParent();
	int faceCount = pSheet->GetFaceListCount();

	for( int i = 0; i < faceCount; i++ )
	{
		CMapFace *pFace = pSheet->GetFaceListDataFace( i );

		CMapSolid *pSolid = pSheet->GetFaceListDataSolid( i );
		GetHistory()->Keep( pSolid );

		switch( uCmd )
		{
			case IDC_ALIGN_WORLD:
			{
				pFace->InitializeTextureAxes( TEXTURE_ALIGN_WORLD, INIT_TEXTURE_AXES | INIT_TEXTURE_FORCE );
				break;
			}

			case IDC_ALIGN_FACE:
			{
				pFace->InitializeTextureAxes( TEXTURE_ALIGN_FACE, INIT_TEXTURE_AXES | INIT_TEXTURE_FORCE );
				break;
			}
		}
	}

	CMapDoc::GetActiveMapDoc()->SetModifiedFlag();

	UpdateDialogData();

	return ( TRUE );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::OnHideMask(void)
{
	m_bHideMask = m_cHideMask.GetCheck();

	CMapFace::SetShowSelection( m_bHideMask == FALSE );

	CMapDoc::GetActiveMapDoc()->UpdateAllViews( MAPVIEW_UPDATE_ONLY_3D | MAPVIEW_UPDATE_OBJECTS | MAPVIEW_UPDATE_COLOR );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : Extents - 
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::GetAllFaceExtents( Extents_t Extents )
{
	BOOL		bFirst = TRUE;
	Extents_t	FaceExtents;

	CFaceEditSheet *pSheet = ( CFaceEditSheet* )GetParent();
	int faceCount = pSheet->GetFaceListCount();	

	for( int nFace = 0; nFace < faceCount; nFace++ )
	{
		CMapFace *pFace = pSheet->GetFaceListDataFace( nFace );
		pFace->GetFaceExtents(FaceExtents);

		if ((FaceExtents[EXTENTS_XMIN][0] < Extents[EXTENTS_XMIN][0]) || (bFirst))
		{
			Extents[EXTENTS_XMIN] = FaceExtents[EXTENTS_XMIN];
		}

		if ((FaceExtents[EXTENTS_XMAX][0] > Extents[EXTENTS_XMAX][0]) || (bFirst))
		{
			Extents[EXTENTS_XMAX] = FaceExtents[EXTENTS_XMAX];
		}

		if ((FaceExtents[EXTENTS_YMIN][1] < Extents[EXTENTS_YMIN][1]) || (bFirst))
		{
			Extents[EXTENTS_YMIN] = FaceExtents[EXTENTS_YMIN];
		}

		if ((FaceExtents[EXTENTS_YMAX][1] > Extents[EXTENTS_YMAX][1]) || (bFirst))
		{
			Extents[EXTENTS_YMAX] = FaceExtents[EXTENTS_YMAX];
		}

		if ((FaceExtents[EXTENTS_ZMIN][2] < Extents[EXTENTS_ZMIN][2]) || (bFirst))
		{
			Extents[EXTENTS_ZMIN] = FaceExtents[EXTENTS_ZMIN];
		}

		if ((FaceExtents[EXTENTS_ZMAX][2] > Extents[EXTENTS_ZMAX][2]) || (bFirst))
		{
			Extents[EXTENTS_ZMAX] = FaceExtents[EXTENTS_ZMAX];
		}

		bFirst = FALSE;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : uCmd - 
// Output : Returns TRUE on success, FALSE on failure.
//-----------------------------------------------------------------------------
BOOL CFaceEditMaterialPage::OnJustify( UINT uCmd )
{
	// Set the material tool current.
	SetMaterialPageTool( MATERIALPAGETOOL_MATERIAL );

	BOOL		bTreatManyAsOneFace;
	Extents_t	Extents;

	// mark undo position
	GetHistory()->MarkUndoPosition( NULL, "Justify texture" );

	CFaceEditSheet *pSheet = ( CFaceEditSheet* )GetParent();
	int faceCount = pSheet->GetFaceListCount();	

	// If multiple faces are selected, use the m_bTreatManyAsOneFace variable to determine
	// how to perform the justification.
	if( faceCount > 1 )
	{
		bTreatManyAsOneFace = m_bTreatAsOneFace;
		if( bTreatManyAsOneFace )
		{
			GetAllFaceExtents( Extents );
		}
	}
	// If only one face is selected, treat it singly.
	else
	{
		bTreatManyAsOneFace = FALSE;
	}

	for( int i = 0; i < faceCount; i++ )
	{
		CMapFace *pFace = pSheet->GetFaceListDataFace( i );

		CMapSolid *pSolid = pSheet->GetFaceListDataSolid( i );
		GetHistory()->Keep( pSolid );

		if( !bTreatManyAsOneFace )
		{
			pFace->GetFaceExtents( Extents );
		}

		switch (uCmd)
		{
			case IDC_JUSTIFY_TOP:
			{
				pFace->JustifyTextureUsingExtents(TEXTURE_JUSTIFY_TOP, Extents);
				break;
			}

			case IDC_JUSTIFY_BOTTOM:
			{
				pFace->JustifyTextureUsingExtents(TEXTURE_JUSTIFY_BOTTOM, Extents);
				break;
			}

			case IDC_JUSTIFY_LEFT:
			{
				pFace->JustifyTextureUsingExtents(TEXTURE_JUSTIFY_LEFT, Extents);
				break;
			}

			case IDC_JUSTIFY_RIGHT:
			{
				pFace->JustifyTextureUsingExtents(TEXTURE_JUSTIFY_RIGHT, Extents);
				break;
			}

			case IDC_JUSTIFY_CENTER:
			{
				pFace->JustifyTextureUsingExtents(TEXTURE_JUSTIFY_CENTER, Extents);
				break;
			}

			case IDC_JUSTIFY_FITTOFACE:
			{
				pFace->JustifyTextureUsingExtents(TEXTURE_JUSTIFY_FIT, Extents);
				break;
			}
#ifdef SLE //// SLE NEW - extra fit modes
			case IDC_JUSTIFY_FIT_WIDTH:
			{
				pFace->JustifyTextureUsingExtents(TEXTURE_JUSTIFY_FIT_WIDTH, Extents);
				break;
			}
			case IDC_JUSTIFY_FIT_HEIGHT:
			{
				pFace->JustifyTextureUsingExtents(TEXTURE_JUSTIFY_FIT_HEIGHT, Extents);
				break;
			}
			case IDC_JUSTIFY_FIT_025:
			{
				pFace->JustifyTextureUsingExtents(TEXTURE_JUSTIFY_FIT_025, Extents);
				break;
			}
			case IDC_JUSTIFY_FIT_05:
			{
				pFace->JustifyTextureUsingExtents(TEXTURE_JUSTIFY_FIT_05, Extents);
				break;
			}
			case IDC_JUSTIFY_FIT_2X2:
			{
				pFace->JustifyTextureUsingExtents(TEXTURE_JUSTIFY_FIT_2X2, Extents);
				break;
			}
			case IDC_JUSTIFY_FIT_3X3:
			{
				pFace->JustifyTextureUsingExtents(TEXTURE_JUSTIFY_FIT_3X3, Extents);
				break;
			}
			case IDC_JUSTIFY_FIT_4X4:
			{
				pFace->JustifyTextureUsingExtents(TEXTURE_JUSTIFY_FIT_4X4, Extents);
				break;
			}
			case IDC_JUSTIFY_FIT_8X8:
			{
				pFace->JustifyTextureUsingExtents(TEXTURE_JUSTIFY_FIT_8X8, Extents);
				break;
			}
			case IDC_JUSTIFY_FIT_16X16:
			{
				pFace->JustifyTextureUsingExtents(TEXTURE_JUSTIFY_FIT_16X16, Extents);
				break;
			}
			case IDC_JUSTIFY_FIT_DEFAULT:
			{
				pFace->JustifyTextureUsingExtents(TEXTURE_JUSTIFY_FIT_DEFAULT, Extents);
				break;
			}
#endif
		}
	}

	CMapDoc::GetActiveMapDoc()->SetModifiedFlag();

	UpdateDialogData();

	return(TRUE);
}
#ifdef SLE //// SLE NEW - randomise texture shift buttons
// Purpose: shift texture on X randomly, useful for avoiding repetition on stairs, etc
void CFaceEditMaterialPage::OnShiftXRandom(void)
{
	BOOL		bTreatManyAsOneFace;
	Extents_t	Extents;

	// mark undo position
	GetHistory()->MarkUndoPosition(NULL, "Shift texture randomly (X)");

	CFaceEditSheet *pSheet = ( CFaceEditSheet* )GetParent();
	int faceCount = pSheet->GetFaceListCount();

	// If multiple faces are selected, use the m_bTreatManyAsOneFace variable to determine
	// how to perform the justification.
	if ( faceCount > 1 )
	{
		bTreatManyAsOneFace = m_bTreatAsOneFace;
		if ( bTreatManyAsOneFace )
		{
			GetAllFaceExtents(Extents);
		}
	}
	// If only one face is selected, treat it singly.
	else
	{
		bTreatManyAsOneFace = FALSE;
	}

	for ( int i = 0; i < faceCount; i++ )
	{
		CMapFace *pFace = pSheet->GetFaceListDataFace(i);

		CMapSolid *pSolid = pSheet->GetFaceListDataSolid(i);
		GetHistory()->Keep(pSolid);

		if ( !bTreatManyAsOneFace )
		{
			pFace->GetFaceExtents(Extents);
		}

		pFace->JustifyTextureUsingExtents(TEXTURE_JUSTIFY_RANDOM_X, Extents);
	}

	CMapDoc::GetActiveMapDoc()->SetModifiedFlag();

	UpdateDialogData();

//	return( TRUE );
}

void CFaceEditMaterialPage::OnShiftYRandom(void)
{
	BOOL		bTreatManyAsOneFace;
	Extents_t	Extents;

	// mark undo position
	GetHistory()->MarkUndoPosition(NULL, "Shift texture randomly (Y)");

	CFaceEditSheet *pSheet = ( CFaceEditSheet* )GetParent();
	int faceCount = pSheet->GetFaceListCount();

	// If multiple faces are selected, use the m_bTreatManyAsOneFace variable to determine
	// how to perform the justification.
	if ( faceCount > 1 )
	{
		bTreatManyAsOneFace = m_bTreatAsOneFace;
		if ( bTreatManyAsOneFace )
		{
			GetAllFaceExtents(Extents);
		}
	}
	// If only one face is selected, treat it singly.
	else
	{
		bTreatManyAsOneFace = FALSE;
	}

	for ( int i = 0; i < faceCount; i++ )
	{
		CMapFace *pFace = pSheet->GetFaceListDataFace(i);

		CMapSolid *pSolid = pSheet->GetFaceListDataSolid(i);
		GetHistory()->Keep(pSolid);

		if ( !bTreatManyAsOneFace )
		{
			pFace->GetFaceExtents(Extents);
		}

		pFace->JustifyTextureUsingExtents(TEXTURE_JUSTIFY_RANDOM_Y, Extents);
	}

	CMapDoc::GetActiveMapDoc()->SetModifiedFlag();

	UpdateDialogData();

//	return( TRUE );
}
#endif
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : id - 
// Output : Returns TRUE on success, FALSE on failure.
//-----------------------------------------------------------------------------
BOOL CFaceEditMaterialPage::OnSwitchMode( UINT id )
{
	CWnd *pButton = GetDlgItem( IDC_MODE );

	CFaceEditSheet *pSheet = ( CFaceEditSheet* )GetParent();
	pSheet->SetClickMode( id );

	switch( id )
	{
	case CFaceEditSheet::ModeLiftSelect:	// set 
		pButton->SetWindowText( "Mode: Lift+Select" );
		break;
	case CFaceEditSheet::ModeLift:
		pButton->SetWindowText( "Mode: Lift" );
		break;
	case CFaceEditSheet::ModeSelect:
		pButton->SetWindowText( "Mode: Select" );
		break;
	case CFaceEditSheet::ModeApply:
		pButton->SetWindowText( "Mode: Apply (texture)" );
		break;
	case CFaceEditSheet::ModeApplyAll:
		pButton->SetWindowText( "Mode: Apply (all)" );
		break;
	case CFaceEditSheet::ModeAlignToView:
		pButton->SetWindowText( "Align To View" );
		break;
	}

	return TRUE;
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::OnMode()
{
	// Set the material tool current.
	SetMaterialPageTool( MATERIALPAGETOOL_MATERIAL );

	// switch mode - 
	//  LIFT - lift texture from clicked face
	//	APPLY - apply selected texture to clicked face
	//	SELECT - mark each face
	//	LIFT/SELECT - mark clicked faces & lift textures

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu( MF_STRING, CFaceEditSheet::ModeLiftSelect, "Lift+Select" );
	menu.AppendMenu( MF_STRING, CFaceEditSheet::ModeLift, "Lift" );
	menu.AppendMenu( MF_STRING, CFaceEditSheet::ModeSelect, "Select" );
	menu.AppendMenu( MF_STRING, CFaceEditSheet::ModeApply, "Apply (texture only)" );
	menu.AppendMenu( MF_STRING, CFaceEditSheet::ModeApplyAll, "Apply (texture + values)" );
	menu.AppendMenu( MF_STRING, CFaceEditSheet::ModeAlignToView, "Align To View" );

	// track menu
	CWnd *pButton = GetDlgItem( IDC_MODE );
	CRect r;
	pButton->GetWindowRect( r );
	menu.TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON, r.left, r.bottom, this, NULL );
}
#ifdef SLE //// SLE NEW - extra fit modes
BOOL CFaceEditMaterialPage::OnFitMode(UINT id)
{
	CWnd *pButton = GetDlgItem(IDC_JUSTIFY_FITTOFACE);

	UINT uCmd = IDC_JUSTIFY_FITTOFACE;

	switch (id)
	{
		case CFaceEditSheet::FitModeWidth:
			uCmd = IDC_JUSTIFY_FIT_WIDTH;
			break;
		case CFaceEditSheet::FitModeHeight:
			uCmd = IDC_JUSTIFY_FIT_HEIGHT;
			break;
		case CFaceEditSheet::FitMode025x025:
			uCmd = IDC_JUSTIFY_FIT_025;
			break;
		case CFaceEditSheet::FitMode05x05:
			uCmd = IDC_JUSTIFY_FIT_05;
			break;
		case CFaceEditSheet::FitMode2x2:
			uCmd = IDC_JUSTIFY_FIT_2X2;
			break;
		case CFaceEditSheet::FitMode3x3:
			uCmd = IDC_JUSTIFY_FIT_3X3;
			break;
		case CFaceEditSheet::FitMode4x4:
			uCmd = IDC_JUSTIFY_FIT_4X4;
			break;
		case CFaceEditSheet::FitMode8x8:
			uCmd = IDC_JUSTIFY_FIT_8X8;
			break;
		case CFaceEditSheet::FitMode16x16:
			uCmd = IDC_JUSTIFY_FIT_16X16;
			break;
		case CFaceEditSheet::FitModeDefault:
			uCmd = IDC_JUSTIFY_FIT_DEFAULT;
			break;
		default:
			break;
	}

	return OnJustify(uCmd);

	CFaceEditSheet *pSheet = ( CFaceEditSheet* )GetParent();

	return TRUE;
}
#endif
#ifdef SLE //// SLE CHANGE - Fit now works as an expand for extra Fit modes
void CFaceEditMaterialPage::OnFitExpand()
{
	// Set the material tool current.
	SetMaterialPageTool(MATERIALPAGETOOL_MATERIAL);

	// switch mode - 
	//  Whole face - the usual Fit
	//	Fit Width - only change X
	//	Fit Height - only change Y
	//	Fit 2x2 - tile XY 2 times
	//  Fit 4x4 - tile XY 4 times

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, CFaceEditSheet::FitModeWholeFace, "Whole Face");
	menu.AppendMenu(MF_STRING, CFaceEditSheet::FitModeWidth, "Fit Width");
	menu.AppendMenu(MF_STRING, CFaceEditSheet::FitModeHeight, "Fit Height");
	menu.AppendMenu(MF_STRING, CFaceEditSheet::FitMode025x025, "Tile 0.25");
	menu.AppendMenu(MF_STRING, CFaceEditSheet::FitMode05x05, "Tile 0.5");
	menu.AppendMenu(MF_STRING, CFaceEditSheet::FitMode2x2, "Tile 2x2");
	menu.AppendMenu(MF_STRING, CFaceEditSheet::FitMode3x3, "Tile 3x3");
	menu.AppendMenu(MF_STRING, CFaceEditSheet::FitMode4x4, "Tile 4x4");
	menu.AppendMenu(MF_STRING, CFaceEditSheet::FitMode8x8, "Tile 8x8");
	menu.AppendMenu(MF_STRING, CFaceEditSheet::FitMode16x16, "Tile 16x16");
	menu.AppendMenu(MF_STRING, CFaceEditSheet::FitModeDefault, "Reset to Default");

	// track menu
	CWnd *pButton = GetDlgItem(IDC_JUSTIFY_FITTOFACE);
	CRect r;
	pButton->GetWindowRect(r);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, r.left, r.bottom, this, NULL);
}
#endif
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nSBCode - 
//			nPos - 
//			*pScrollBar - 
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::OnVScroll( UINT nSBCode, UINT nPos, CScrollBar *pScrollBar )
{
#ifdef SLE //// SLE NEW - texture shear
	if ( !m_shearLock_bool )
	{
		CFaceEditSheet *pSheet = ( CFaceEditSheet* )GetParent();
		if ( !pSheet || ( pSheet && pSheet->GetFaceListCount() == 0 ) ) return;

		CMapFace *pFace;
		pFace = pSheet->GetFaceListDataFace(0);

		if ( !pFace ) return;

		TEXTURE &t = pFace->texture;

		float fshearX, fshearY, fshearZ;

		fshearX = t.UAxis[ 2 ];
		fshearY = t.VAxis[ 1 ];
		fshearZ = t.VAxis[ 0 ];

		FloatToSpin(fshearX, ( CSpinButtonCtrl* )GetDlgItem(IDC_SPINSHEARX), TRUE);
		FloatToSpin(fshearY, ( CSpinButtonCtrl* )GetDlgItem(IDC_SPINSHEARY), TRUE);
		FloatToSpin(fshearZ, ( CSpinButtonCtrl* )GetDlgItem(IDC_SPINSHEARZ), TRUE);
	}

	Apply(NULL, FACE_APPLY_MAPPING | FACE_APPLY_SHEAR | FACE_APPLY_LIGHTMAP_SCALE);
#else
	Apply(NULL, FACE_APPLY_MAPPING);
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pNMHDR - 
//			pResult - 
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::OnDeltaPosFloatSpin( NMHDR *pNMHDR, LRESULT *pResult ) 
{
	NM_UPDOWN *pNMUpDown = ( NM_UPDOWN* )pNMHDR;

	CEdit *pEdit = NULL;
	switch( pNMUpDown->hdr.idFrom )
	{
		case IDC_SPINSCALEY:
		{
			pEdit = &m_scaleY;
			break;
		}

		case IDC_SPINSCALEX:
		{
			pEdit = &m_scaleX;
			break;
		}
#ifdef SLE //// SLE NEW - grow/shrink face selection
		case IDC_SPINDOT:
		{
			pEdit = &m_cGrowAngle;
			break;
		}

		//// SLE NEW - texture shear
		case IDC_SPINSHEARX:
		{
			pEdit = &m_shearX;
			break;
		}

		case IDC_SPINSHEARY:
		{
			pEdit = &m_shearY;
			break;
		}

		case IDC_SPINSHEARZ:
		{
			pEdit = &m_shearZ;
			break;
		}
		//// SLE NEW - texture shift step control
		case IDC_SPINSHIFTX:
		{
			pEdit = &m_shiftX;
			break;
		}

		case IDC_SPINSHIFTY:
		{
			pEdit = &m_shiftY;
			break;
		}
#endif
	}

	if( pEdit != NULL )
	{
		CString str;
		pEdit->GetWindowText(str);
		float fTmp = atof(str);
#ifdef SLE
		if( pEdit == &m_shearX || pEdit == &m_shearX || pEdit == &m_shearZ)
			fTmp += 0.01f * float(pNMUpDown->iDelta);
		
		//// SLE NEW - texture shift step control
		else if ( pEdit == &m_shiftX )
		{
			CString step;
			m_shiftStepX.GetWindowText(step);
			fTmp += 1 * atof(step) * float(pNMUpDown->iDelta);
		}
		else if ( pEdit == &m_shiftY )
		{
			CString step;
			m_shiftStepY.GetWindowText(step);
			fTmp += 1 * atof(step) * float(pNMUpDown->iDelta);
		}
		else
#endif
			fTmp += 0.1f * float( pNMUpDown->iDelta );
#ifdef SLE //// SLE NEW - texture shift step control
		 if ( pEdit == &m_shiftX || pEdit == &m_shiftY)
			str.Format( "%g", fTmp );
		 else
			str.Format( "%.2f", fTmp );
#else
		str.Format( "%.2f", fTmp );
#endif
		pEdit->SetWindowText( str );

		*pResult = 0;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nType - 
//			cx - 
//			cy - 
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::OnSize( UINT nType, int cx, int cy )
{
	return;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::OnSelChangeTexture( void )
{
	// Set the material tool current.
	SetMaterialPageTool( MATERIALPAGETOOL_MATERIAL );

	if( !m_bInitialized )
	{
		return;
	}

	UpdateTexture();

	if( m_pCurTex != NULL )
	{
		m_TextureList.AddMRU( m_pCurTex );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::OnCheckUnCheck( void )
{
	Apply(NULL, FACE_APPLY_MAPPING);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::OnTreatAsOne( void )
{
	// Set the material tool current.
	SetMaterialPageTool( MATERIALPAGETOOL_MATERIAL );

	CButton *pCheck = ( CButton* )GetDlgItem( IDC_TREAT_AS_ONE );
	Assert( pCheck != NULL );
	m_bTreatAsOneFace = pCheck->GetCheck();
}

//-----------------------------------------------------------------------------
// Purpose: Invokes the texture replace dialog.
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::OnReplace( void )
{
	// Set the material tool current.
	SetMaterialPageTool( MATERIALPAGETOOL_MATERIAL );

	//
	// get active map doc
	//
	CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();
	if( !pDoc )
		return;

	// ready the replace dialog
	CReplaceTexDlg dlg( pDoc->GetSelection()->GetCount() );

	// get the texture to replace -- the default texture?!
	dlg.m_strFind = GetDefaultTextureName();

	//
	// open replace dialog -- modal
	//
	if( dlg.DoModal() != IDOK )
		return;
	
	// mark undo position
	GetHistory()->MarkUndoPosition( pDoc->GetSelection()->GetList(), "Replace Textures" );

	if( dlg.m_bMarkOnly )
	{
		pDoc->SelectObject( NULL, scClear );	// clear selection first
	}

	dlg.DoReplaceTextures();
}

//-----------------------------------------------------------------------------
// Purpose: Updates the m_pTexture data member based on the current selection.
//			Also updates the window text and the texture picture.
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::UpdateTexture( void )
{
	int iSel = m_TextureList.GetCurSel();

	if( iSel == LB_ERR )
	{
		m_TexturePic.SetTexture( NULL );
		m_pCurTex = NULL;
		return;
	}

	m_pCurTex = ( IEditorTexture* )m_TextureList.GetItemDataPtr( iSel );
	m_TexturePic.SetTexture( m_pCurTex );

	if( m_pCurTex )
	{
		char szBuf[128];
		sprintf( szBuf, "%dx%d", m_pCurTex->GetWidth(), m_pCurTex->GetHeight() );
		GetDlgItem( IDC_TEXTURESIZE )->SetWindowText( szBuf );

		char szTexName[128];
		m_pCurTex->GetShortName( szTexName );
		SetDefaultTextureName( szTexName );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Selects a texture by name.
// Input  : pszTextureName - Texture name to select.
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::SelectTexture( LPCSTR pszTextureName )
{
	int nIndex = m_TextureList.SelectString( -1, pszTextureName );

	//
	// If the texture is not in the list, add it to the list.
	//
	if( nIndex == LB_ERR )
	{
		IEditorTexture *pTex = g_Textures.FindActiveTexture( pszTextureName );
		if( pTex != NULL )
		{
			nIndex = m_TextureList.AddString( pszTextureName );
			m_TextureList.SetItemDataPtr( nIndex, pTex );
			m_TextureList.SetCurSel( nIndex );
		}
	}

	UpdateTexture();

	if( nIndex != LB_ERR )
	{
		IEditorTexture *pTex = ( IEditorTexture* )m_TextureList.GetItemDataPtr( nIndex );
		m_TextureList.AddMRU( pTex );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::NotifyGraphicsChanged( void )
{
	if( !IsWindow( m_hWnd ) )
	{
		return;
	}

	// load groups into group list
	CString str;
	int iCurSel = m_TextureGroupList.GetCurSel();
	if (iCurSel != LB_ERR)
	{
		m_TextureGroupList.GetLBText(iCurSel, str);
	}

	m_TextureGroupList.SetRedraw(FALSE);
	m_TextureGroupList.ResetContent();
	m_TextureGroupList.AddString("All Textures");

	int nCount = g_Textures.GroupsGetCount();
	if (nCount > 1)
	{
		//
		// Skip first group ("All Textures").
		//
		for (int i = 1; i < nCount; i++)
		{
			CTextureGroup *pGroup = g_Textures.GroupsGet(i);
			if (pGroup->GetTextureFormat() == g_pGameConfig->GetTextureFormat())
			{
				const char *p = strstr(pGroup->GetName(), "textures\\");
				if (p)
				{
					p += strlen("textures\\");
				}
				else
				{
					p = pGroup->GetName();
				}

				m_TextureGroupList.AddString(p);
			}
		}
	}
	m_TextureGroupList.SetRedraw(TRUE);

	if (iCurSel == LB_ERR || m_TextureGroupList.SelectString(-1, str) == LB_ERR)
	{
		m_TextureGroupList.SetCurSel(0);
	}
	
	m_TextureGroupList.Invalidate();

	char szName[MAX_PATH];
	m_TextureGroupList.GetLBText(m_TextureGroupList.GetCurSel(), szName);
	g_Textures.SetActiveGroup(szName);

	//
	// This is called when the loaded graphics list is changed,
	// or on first init by this->Create().
	//
	m_TextureList.LoadGraphicList();
	UpdateTexture();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::OnBrowse( void )
{
	// Set the material tool current.
	SetMaterialPageTool( MATERIALPAGETOOL_MATERIAL );

	CTextureBrowser *pBrowser = GetMainWnd()->pTextureBrowser;

	int iSel = m_TextureList.GetCurSel();

	if (iSel != LB_ERR)
	{
		IEditorTexture *pTex = (IEditorTexture *)m_TextureList.GetItemDataPtr(iSel);
		if (pTex != NULL)
		{
			char sz[128];
	
			pTex->GetShortName(sz);
			pBrowser->SetInitialTexture(sz);
		}
	}

	if (pBrowser->DoModal() == IDOK)
	{
		IEditorTexture *pTex = g_Textures.FindActiveTexture(pBrowser->m_cTextureWindow.szCurTexture);
#ifdef SLE_TEXTUREBROWSER_FAVOURITES
		if ( pBrowser->m_cFavouritesWindow.m_bMouseOver )
		{
			pTex = g_Textures.FindActiveTexture(pBrowser->m_cFavouritesWindow.szCurTexture);
			pBrowser->m_cTextureWindow.m_bShouldUpdateTextureSelection = false; // kind of wonky but will get the job done...
		}
		else
		{
			pBrowser->m_cTextureWindow.m_bShouldUpdateTextureSelection = true;
		}
#endif

		if (pTex != NULL)
		{
			int iCount = m_TextureList.GetCount();
			for (int i = 0; i < iCount; i++)
			{
				if (pTex == (IEditorTexture *)m_TextureList.GetItemDataPtr(i))
				{
					m_TextureList.SetCurSel(i);
					UpdateTexture();
					m_TextureList.AddMRU(pTex);
					break;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::OnChangeTextureGroup( void )
{
	// Set the material tool current.
	SetMaterialPageTool( MATERIALPAGETOOL_MATERIAL );

	int iGroup = m_TextureGroupList.GetCurSel();

	//
	// Set the active texture group by name.
	//
	char szName[MAX_PATH];
	m_TextureGroupList.GetLBText(iGroup, szName);
	g_Textures.SetActiveGroup(szName);

	//
	// Refresh the texture list contents.
	//
	m_TextureList.LoadGraphicList();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::OnButtonApply( void )
{
	// Set the material tool current.
	SetMaterialPageTool( MATERIALPAGETOOL_MATERIAL );

#ifdef SLE
	Apply(NULL, FACE_APPLY_MAPPING | FACE_APPLY_SHEAR | FACE_APPLY_LIGHTMAP_SCALE);
#else
	Apply(NULL, FACE_APPLY_ALL);
#endif
}
#ifdef SLE //// SLE NEW - texture shear
void CFaceEditMaterialPage::OnToggleShearLock(void)
{
	CButton *pCheck = ( CButton* )GetDlgItem(IDC_FACEEDIT_SHEARLOCK);
	Assert(pCheck != NULL);
	m_shearLock_bool = pCheck->GetCheck();
}
#endif
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
BOOL CFaceEditMaterialPage::OnSetActive( void )
{
	CMainFrame *pMainFrm = GetMainWnd();
	if( !pMainFrm )
		return FALSE;

	ToolManager()->SetTool( TOOL_FACEEDIT_MATERIAL );

	// Set the initial face edit tool state.
	SetMaterialPageTool( MATERIALPAGETOOL_MATERIAL );

	return CPropertyPage::OnSetActive();
}

//-----------------------------------------------------------------------------
// Purpose: Brings up the smoothing group dialog.
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::OnButtonSmoothingGroups( void )
{
	if( !m_FaceSmoothDlg.Create( IDD_SMOOTHING_GROUPS, this ) )
		return;	

	m_FaceSmoothDlg.ShowWindow( SW_SHOW );

	// Set the initial face edit tool state.
	SetMaterialPageTool( MATERIALPAGETOOL_SMOOTHING_GROUP );

	return;
}
#ifdef SLE
//-----------------------------------------------------------------------------
//// SLE NEW - mark button on texture tool, without needing to open texture browser every time
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::OnButtonMarkAll(void)
{
	{
		CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();
		if (pDoc != NULL)
		{
			pDoc->ReplaceTextures(m_TexturePic.GetTexture()->GetName(), "", TRUE, 0x100, FALSE, FALSE);
			EndDialog(IDOK);
		}
	}
} 

//-----------------------------------------------------------------------------
//// SLE NEW - button to texture selected faces as nodraw 
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::OnButtonApplyNodraw(void)
{
	{
		GetHistory()->MarkUndoPosition(NULL, "Apply Face Attributes");

		int i;
		// Keep the solids that we are about to change.
		CUtlVector<CMapSolid *> kept;
		CFaceEditSheet *pSheet = (CFaceEditSheet*)GetParent();
		for (i = 0; i < pSheet->GetFaceListCount(); i++)
		{
			CMapSolid *pSolid = pSheet->GetFaceListDataSolid(i);
			if (kept.Find(pSolid) == -1)
			{
				GetHistory()->Keep(pSolid);
				kept.AddToTail(pSolid);
			}
		}

		//
		// Run thru stored faces & apply.
		//
		int faceCount = pSheet->GetFaceListCount();
		for (i = 0; i < faceCount; i++)
		{
			CMapFace *pFace = pSheet->GetFaceListDataFace(i);

			pFace->SetTexture("tools/toolsnodraw");

			//	CMapClass	*pParent = dynamic_cast< CMapClass * >(pFace->GetParent());
			//	if (pParent)
			//	{
			//		pMapDoc->RemoveFromAutoVisGroups(pParent);
			//		pMapDoc->AddToAutoVisGroup(pParent);
			//	}
		}

		//	pMapDoc->SetModifiedFlag();
	}
}
#endif
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::SetMaterialPageTool( unsigned short iMaterialTool )
{
	if ( m_iMaterialTool == MATERIALPAGETOOL_SMOOTHING_GROUP )
	{
		// Close the window.
		m_FaceSmoothDlg.DestroyWindow();
#ifdef SLE //// SLE CHANGE - somehow, the dlg window started acting up without explicit call to close when, well, closed
		m_FaceSmoothDlg.CloseWindow();
#endif
	}

	// Set the new material tool.
	m_iMaterialTool = iMaterialTool;
}

//-----------------------------------------------------------------------------
// Purpose: Called when a new material (.vmt file) is detected.
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::NotifyNewMaterial( IEditorTexture *pTex )
{
	m_TextureList.LoadGraphicList();
	UpdateTexture();
}

//-----------------------------------------------------------------------------
// Purpose: Called to set the enabled state of the dialog controls
//-----------------------------------------------------------------------------
void CFaceEditMaterialPage::SetReadOnly( bool bIsReadOnly )
{
	BOOL	State = ( bIsReadOnly ? FALSE : TRUE );

	m_shiftX.EnableWindow( State );
	m_shiftY.EnableWindow( State );
	m_scaleX.EnableWindow( State );
	m_scaleY.EnableWindow( State );
	m_rotate.EnableWindow( State );
	m_cLightmapScale.EnableWindow( State );
	m_cHideMask.EnableWindow( State );
	m_cExpand.EnableWindow( State );
	m_TextureList.EnableWindow( State );
	m_TextureGroupList.EnableWindow( State );

#ifdef SLE //// SLE NEW - texture shear
	m_shearX.EnableWindow(State);
	m_shearY.EnableWindow(State);
	m_shearZ.EnableWindow(State);
	
	//// SLE NEW - grow/shrink face selection
	m_cGrowAngle.EnableWindow(State);
	m_cGrowButton.EnableWindow(State);
	m_cShrinkButton.EnableWindow(State);

	//// SLE NEW - texture shift step control
	m_shiftStepX.EnableWindow(State);
	m_shiftStepY.EnableWindow(State);
#endif

	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_JUSTIFY_LEFT ), State );
	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_JUSTIFY_RIGHT ), State );
	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_JUSTIFY_FITTOFACE ), State );
	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_JUSTIFY_TOP ), State );
	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_JUSTIFY_BOTTOM ), State );
	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_JUSTIFY_CENTER ), State );
	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_TREAT_AS_ONE ), State );
	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_ALIGN_WORLD ), State );
	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_ALIGN_FACE ), State );
	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_BROWSE ), State );
	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_REPLACE ), State );
	::EnableWindow( ::GetDlgItem( m_hWnd, ID_FACEEDIT_APPLY ), State );
	::EnableWindow( ::GetDlgItem( m_hWnd, IDC_MODE ), State );
	::EnableWindow( ::GetDlgItem( m_hWnd, ID_BUTTON_SMOOTHING_GROUPS ), State );
}
#ifdef SLE
void CFaceEditMaterialPage::OnGrowSelection()
{
	// todo:
	// - ignore tool textures while selecting non-tool textures?
	// - add angle control
	// - add a checkbox to instantly select all adjacent faces together
	// (effectively just loop through them till there's no more left)

	CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();
	if ( !pDoc ) return;

	CMapWorld *pWorld = pDoc->GetMapWorld();
	if ( pWorld == NULL ) return;

	CFaceEditSheet *pSheet = ( CFaceEditSheet* )GetParent();
	if ( !pSheet ) return;

	int selectionCount = pSheet->GetFaceListCount();

	if ( selectionCount == 0 ) return;

	// if the user clicked on a new face and cleared selection, reset the list
	if ( selectionCount == 1 )
		m_growIgnoreList.RemoveAll();

	float dot = cos(DEG2RAD(m_growAngle_float));

	// get all the world's solids.
	// Extract it outside the loop, since it's not going to change while looping.

	EnumChildrenPos_t pos; // place here for faster but one-face-at-a-time selection (for later)
	CMapClass *pChild = pWorld->GetFirstDescendent(pos);
	CMapFaceList solidFaces;

	while ( pChild != NULL )
	{
		if ( pChild->IsVisible() )
		{
			CMapSolid *solid = dynamic_cast < CMapSolid * > ( pChild );
			if ( ( solid != NULL ) )
			{
				// for each solid, get all of solid's faces
				for ( int i = 0; i < solid->GetFaceCount(); i++ )
				{
					CMapFace *testFace = solid->GetFace(i);

					if ( !testFace ) break;

					if ( m_growIgnoreList.HasElement(testFace) )
					{
						//	pFace->SetTexture("tools/toolsnodraw");
						continue;
					}

					// ignore nodraw, sky and tools
					if ( !V_strncmp(testFace->GetTexture()->GetFileName(), "tools/", 6) )
					{
						continue;
					}

					if( !solidFaces.HasElement(testFace))
						solidFaces.AddToTail(testFace);
				}
			}
		}
		pChild = pWorld->GetNextDescendent(pos);
	}

	SetReadOnly(true); // prevent spam-clicking, they can become very slow because the ignore list doesn't get updated

	for ( int i = 0; i < selectionCount; i++ )
	{
		CMapFace *pFace = pSheet->GetFaceListDataFace(i);

		if ( !pFace ) continue;

		// reduce the redundant checks by not checking previous selections,
		// speeds up the whole thing
		if ( m_growIgnoreList.HasElement(pFace) )
		{
		//	pFace->SetTexture("tools/toolsnodraw");
			continue;
		}

		int myPointCount = pFace->GetPointCount();
		Vector myNormal;
		pFace->GetFaceNormal(myNormal);

#if 0
		// get all the world's solids
		EnumChildrenPos_t pos;
		CMapClass *pChild = pWorld->GetFirstDescendent(pos);
		while ( pChild != NULL )
		{
			if ( pChild->IsVisible() )
			{
				CMapSolid *solid = dynamic_cast < CMapSolid * > ( pChild );
				if ( ( solid != NULL ) )
				{
					// for each solid, get all of solid's faces
					for ( int i = 0; i < solid->GetFaceCount(); i++ )
					{
						CMapFace *testFace = solid->GetFace(i);

						if ( !testFace ) break;
						 
						if ( m_growIgnoreList.HasElement(testFace) )
						{
						//	pFace->SetTexture("tools/toolsnodraw");
							continue;
						}

						// ignore nodraw, sky and tools
						if ( !V_strncmp(testFace->GetTexture()->GetFileName(), "tools/", 6) )
						{
							continue;
						}

						Vector faceNormal;
						testFace->GetFaceNormal(faceNormal);

						// check if they are possibly within angle threshold of the selected face
						if ( faceNormal.Dot(myNormal) >= dot )
						{
							// check if they share any vertices with current selection
							// iterate through all of the points of the face being checked...
							for ( int j = 0; j < testFace->GetPointCount(); j++ )
							{
								Vector facePoint;
								testFace->GetPoint(facePoint, j);
								Vector myPoint;
								// and then iterate through all of the already selected face's points
								for ( int k = 0; k < myPointCount; k++ )
								{
									pFace->GetPoint(myPoint, k);

									// if the verts are close enough, this face is eligible
									if ( facePoint.DistTo(myPoint) <= (0.1f) )
									{
										bool found = false;
										CFaceEditSheet *parent = ( CFaceEditSheet* )GetParentSheet();
										if ( !parent ) break;

										// simulate clicking on that face to select
										parent->ClickFace(solid, i, CFaceEditSheet::cfSelect);

										// store the previous selection; it won't be checked again, reducing the redundant checks
										m_growIgnoreList.AddToTail(pFace);
									}
								}
							}
						}
					}
				}
			}
			pChild = pWorld->GetNextDescendent(pos);
		}
#else
		// for each solid, get all of solid's faces
		for ( int i = 0; i < solidFaces.Count(); i++ )
		{
			CMapFace *testFace = solidFaces.Element(i);

			if ( !testFace ) break;

			if ( m_growIgnoreList.HasElement(testFace) )
			{
				continue;
			}

			Vector faceNormal;
			testFace->GetFaceNormal(faceNormal);

			// check if they are possibly within angle threshold of the selected face
			if ( faceNormal.Dot(myNormal) >= dot )
			{
				// check if they share any vertices with current selection
				// iterate through all of the points of the face being checked...
				for ( int j = 0; j < testFace->GetPointCount(); j++ )
				{
					Vector facePoint;
					testFace->GetPoint(facePoint, j);
					Vector myPoint;
					// and then iterate through all of the already selected face's points
					for ( int k = 0; k < myPointCount; k++ )
					{
						pFace->GetPoint(myPoint, k);

						// if the verts are close enough, this face is eligible
						if ( facePoint.DistTo(myPoint) <= ( 0.1f ) )
						{
							bool found = false;
							CFaceEditSheet *parent = ( CFaceEditSheet* )GetParentSheet();
							if ( !parent ) break;

							// simulate clicking on that face to select
							CMapSolid* solid = ( CMapSolid* )( testFace->GetParent() );
							if ( !solid ) break;

							parent->ClickFace(solid, solid->GetFaceIndex(testFace), CFaceEditSheet::cfSelect, -1, false);

							// store the previous selection; it won't be checked again, reducing the redundant checks
							m_growIgnoreList.AddToTail(pFace);
						}
					}
				}
			}
		}
#endif
	}

	SetReadOnly(false);
//	CString str;
//	str.Format("ignoreList has %i faces", m_growIgnoreList.Count());
//	AfxMessageBox(str, MB_OK);
}

void CFaceEditMaterialPage::OnShrinkSelection()
{
	// get the verts of all the selected faces, and if a face has verts with free-standing verts
	// (not shared with other selected faces), deselect them;
	// don't deselect the last face

	/*
	1. Get a list of all currently selected faces
	2. Per-face, collect all the verts of the face.
	3. Run a check per-vert against all other selected faces and their verts to see
	if all verts of the tested face have a neighbour. If all do, skip the face. Otherwise,
	mark it for deselection.
	*/
	CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();
	if ( !pDoc ) return;

	CMapWorld *pWorld = pDoc->GetMapWorld();
	if ( pWorld == NULL ) return;

	CFaceEditSheet *pSheet = ( CFaceEditSheet* )GetParent();
	if ( !pSheet ) return;

	int faceCount = pSheet->GetFaceListCount();

	if ( faceCount <= 1 ) return;
}

void CFaceEditMaterialPage::OnPickByAngle(void)
{
	CButton *pCheck = ( CButton* )GetDlgItem(IDC_FACEEDIT_PICKBYANGLE);
	Assert(pCheck != NULL);
	m_pickByAngle_bool = pCheck->GetCheck();
}

void CFaceEditMaterialPage::OnClickPickByAngle(CMapFace *face, int cmd, int clickMode)
{
	if ( !m_pickByAngle_bool ) return;

	if ( !face ) return;

	CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();
	if ( !pDoc ) return;

	CMapWorld *pWorld = pDoc->GetMapWorld();
	if ( pWorld == NULL ) return;

	CFaceEditSheet *pSheet = ( CFaceEditSheet* )GetParent();
	if ( !pSheet ) return;

//	OnGrowSelection();
	// unfortunate code duplication, maybe solve later - 
	// simply calling OnGrowSelection works but it also means growing selections
	// around parts the user didn't click if they already had a selection elsewhere

	// todo: necessary?
	m_growIgnoreList.RemoveAll();

	float dot = cos(DEG2RAD(m_growAngle_float));

	// get all the world's solids.
	// Extract it outside the loop, since it's not going to change while looping.

	EnumChildrenPos_t pos; // place here for faster but one-face-at-a-time selection (for later)
	CMapClass *pChild = pWorld->GetFirstDescendent(pos);
	CMapFaceList solidFaces;

	while ( pChild != NULL )
	{
		if ( pChild->IsVisible() )
		{
			CMapSolid *solid = dynamic_cast < CMapSolid * > ( pChild );
			if ( ( solid != NULL ) )
			{
				// for each solid, get all of solid's faces
				for ( int i = 0; i < solid->GetFaceCount(); i++ )
				{
					CMapFace *testFace = solid->GetFace(i);

					if ( !testFace ) break;

					if ( m_growIgnoreList.HasElement(testFace) )
					{
						continue;
					}

					// don't ignore when manually selecting per-angle
				//	if ( !V_strncmp(testFace->GetTexture()->GetFileName(), "tools/", 6) )
				//	{
				//		continue;
				//	}

					if ( !solidFaces.HasElement(testFace) )
						solidFaces.AddToTail(testFace);
				}
			}
		}
		pChild = pWorld->GetNextDescendent(pos);
	}

	SetReadOnly(true); // prevent spam-clicking, they can become very slow because the ignore list doesn't get updated
	
	int myPointCount = face->GetPointCount();
	Vector myNormal;
	face->GetFaceNormal(myNormal);

	// for each solid, get all of solid's faces
	for ( int i = 0; i < solidFaces.Count(); i++ )
	{
		CMapFace *testFace = solidFaces.Element(i);

		if ( !testFace ) break;

		if ( m_growIgnoreList.HasElement(testFace) )
		{
			continue;
		}

		Vector faceNormal;
		testFace->GetFaceNormal(faceNormal);

		// check if they are possibly within angle threshold of the selected face
		if ( faceNormal.Dot(myNormal) >= dot )
		{
			// check if they share any vertices with current selection
			// iterate through all of the points of the face being checked...
			for ( int j = 0; j < testFace->GetPointCount(); j++ )
			{
				Vector facePoint;
				testFace->GetPoint(facePoint, j);
				Vector myPoint;
				// and then iterate through all of the already selected face's points
				for ( int k = 0; k < myPointCount; k++ )
				{
					face->GetPoint(myPoint, k);

					// if the verts are close enough, this face is eligible
					if ( facePoint.DistTo(myPoint) <= ( 0.1f ) )
					{
						bool found = false;
						CFaceEditSheet *parent = ( CFaceEditSheet* )GetParentSheet();
						if ( !parent ) break;

						// simulate clicking on that face to select
						CMapSolid* solid = ( CMapSolid* )( testFace->GetParent() );
						if ( !solid ) break;

						parent->ClickFace(solid, solid->GetFaceIndex(testFace), cmd, clickMode, false);

					}
				}
			}
		}
	}

	SetReadOnly(false);
}
#endif
