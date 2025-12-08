//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
// TransformDlg.cpp : implementation file
//

#include "stdafx.h"
#include "hammer.h"
#include "TransformDlg.h"
#ifdef SLE
#include "mapdoc.h" //// for accessing the world
#include "MapWorld.h" //// for accessing the sky camera
#include "Options.h" //// for accessing the texture scale lock setting
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

/////////////////////////////////////////////////////////////////////////////
// CTransformDlg dialog

static LPCTSTR pszSection = "Transform";

void CTransformDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTransformDlg)
	DDX_Radio(pDX, IDC_MODE, m_iMode);
#ifdef SLE //// SLE NEW - more transform dialogue controls
	DDX_Check(pDX, IDC_TRANSFORM_LOCAL, m_bUseLocalTransform);
	DDX_Check(pDX, IDC_TRANSFORM_LOCKTEXSCALE, m_bLockTextureScale);
	DDX_Check(pDX, IDC_TRANSFORM_MODELSCALE, m_bScaleModelEntities);
	DDX_Control(pDX, IDC_TRANSFORM_LOCKTEXSCALE, m_cLockTexScale);
#endif
	DDX_Text(pDX, IDC_X, m_X);
	DDX_Text(pDX, IDC_Y, m_Y);
	DDX_Text(pDX, IDC_Z, m_Z);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTransformDlg, CDialog)
	//{{AFX_MSG_MAP(CTransformDlg)
#ifdef SLE //// SLE NEW - more transform dialogue controls
	ON_BN_CLICKED(IDC_TRANSFORMRESET, OnButtonReset)
	ON_BN_CLICKED(IDC_TRANSFORM_SKYSCALE, OnButtonCalculateSkyScale)
	ON_BN_CLICKED(IDC_TRANSFORM_LOCKTEXSCALE, OnButtonLockTexScale)

#ifdef	SLE_DARK_THEME	//// SLE dark theme test
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
#endif
#endif
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CTransformDlg::CTransformDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTransformDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTransformDlg)
	m_iMode = -1;
	m_X = 0.0f;
	m_Y = 0.0f;
	m_Z = 0.0f;
#ifdef SLE //// SLE NEW - more transform dialogue controls
	m_bUseLocalTransform = 0;
	m_bScaleModelEntities = 0;
#endif
	//}}AFX_DATA_INIT

	// load up old defaults
	CString str;
	str = AfxGetApp()->GetProfileString(pszSection, "X_Value", "0.0");
	m_X = atof(str);
	str = AfxGetApp()->GetProfileString(pszSection, "Y_Value", "0.0");
	m_Y = atof(str);
	str = AfxGetApp()->GetProfileString(pszSection, "Z_Value", "0.0");
	m_Z = atof(str);
	m_iMode = AfxGetApp()->GetProfileInt(pszSection, "Mode_Value", -1);
#ifdef SLE //// SLE NEW - more transform dialogue controls
	m_bUseLocalTransform = AfxGetApp()->GetProfileInt(pszSection, "Use_Local", 0);
	m_bScaleModelEntities = AfxGetApp()->GetProfileInt(pszSection, "Set_Modelscale", 0);
#endif
}
#ifdef SLE //// SLE NEW - overhauled transform dialogue
BOOL CTransformDlg::OnInitDialog(void)
{
	CDialog::OnInitDialog();
	m_cLockTexScale.SetCheck(Options.IsScaleLockingTextures() == TRUE);
	return(TRUE);
}

//// SLE NEW - remember transform dialogue values and mode
void CTransformDlg::SaveValues()
{
	CString str;
	AfxGetApp()->WriteProfileInt(pszSection, "Mode_Value", m_iMode);
	str.Format("%f", m_X);
	AfxGetApp()->WriteProfileString(pszSection, "X_Value", str);
	str.Format("%f", m_Y);
	AfxGetApp()->WriteProfileString(pszSection, "Y_Value", str);
	str.Format("%f", m_Z);
	AfxGetApp()->WriteProfileString(pszSection, "Z_Value", str);

	//// SLE NEW - more transform dialogue controls
	AfxGetApp()->WriteProfileInt(pszSection, "Use_Local", m_bUseLocalTransform);
	AfxGetApp()->WriteProfileInt(pszSection, "Set_Modelscale", m_bScaleModelEntities);
}

void CTransformDlg::OnButtonReset(void)
{
	SetDlgItemText(IDC_X, "0");
	SetDlgItemText(IDC_Y, "0");
	SetDlgItemText(IDC_Z, "0");
	m_X = 0.0f;
	m_Y = 0.0f;
	m_Z = 0.0f;
	SaveValues();	
}

//// SLE NEW - calculate scaling values based on the sky camera
void CTransformDlg::OnButtonCalculateSkyScale(void)
{
	if ( m_iMode != 1 ) return; // don't accidentally mess up rotate/translate boxes

	CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();
	if ( !pDoc ) return;

	CMapWorld *pWorld = pDoc->GetMapWorld();
	if ( !pWorld ) return;

	float scale = 0.0625;
	if( pWorld->GetSkyCameraScale() > 0 ) // mapper sanity check
		scale = 1 / pWorld->GetSkyCameraScale();
	
	CString str;
	str.Format("%.6f", scale);

	SetDlgItemText(IDC_X, str);
	SetDlgItemText(IDC_Y, str);
	SetDlgItemText(IDC_Z, str);

	m_X = m_Y = m_Z = scale;

	SaveValues();
}

void CTransformDlg::OnButtonLockTexScale(void)
{
	Options.SetScaleLockingTextures(m_cLockTexScale.GetCheck());
}
#endif
/////////////////////////////////////////////////////////////////////////////
// CTransformDlg message handlers
