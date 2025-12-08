//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
// ScaleVerticesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "hammer.h"
#include "ScaleVerticesDlg.h"
#include "MapDoc.h"
#include "GlobalFunctions.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

/////////////////////////////////////////////////////////////////////////////
// CScaleVerticesDlg dialog
#ifdef SLE //// SLE NEW - better vertex tool scaling
static LPCTSTR pszSection = "TransformVertex";
#endif
CScaleVerticesDlg::CScaleVerticesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScaleVerticesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScaleVerticesDlg)
	//}}AFX_DATA_INIT
#ifdef SLE
	m_idummy = 1;
	m_iMode = 1;
//	m_iMode = AfxGetApp()->GetProfileInt(pszSection, "Mode_Value", -1); // disable loading at the moment
#endif
}

void CScaleVerticesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScaleVerticesDlg)
#ifdef SLE //// SLE NEW - better vertex tool scaling
	DDX_Control(pDX, IDC_MORPH_TRANSX_SPIN, m_cTransformXSpin);
	DDX_Control(pDX, IDC_MORPH_TRANSY_SPIN, m_cTransformYSpin);
	DDX_Control(pDX, IDC_MORPH_TRANSZ_SPIN, m_cTransformZSpin);
	DDX_Control(pDX, IDC_MORPH_TRANSX, m_cTransformX);
	DDX_Control(pDX, IDC_MORPH_TRANSY, m_cTransformY);
	DDX_Control(pDX, IDC_MORPH_TRANSZ, m_cTransformZ);
	DDX_Radio(pDX, IDC_MORPH_MODE, m_idummy);
#else
	DDX_Control(pDX, IDC_SCALESPIN, m_cScaleSpin);
	DDX_Control(pDX, IDC_SCALE, m_cScale);
#endif
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CScaleVerticesDlg, CDialog)
	//{{AFX_MSG_MAP(CScaleVerticesDlg)
#ifdef SLE //// SLE NEW - better vertex tool scaling
	ON_EN_CHANGE(IDC_MORPH_TRANSX, OnChangeTransformX)
	ON_EN_CHANGE(IDC_MORPH_TRANSY, OnChangeTransformY)
	ON_EN_CHANGE(IDC_MORPH_TRANSZ, OnChangeTransformZ)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MORPH_TRANSX_SPIN, OnDeltaposTransformXSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MORPH_TRANSY_SPIN, OnDeltaposTransformYSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_MORPH_TRANSZ_SPIN, OnDeltaposTransformZSpin)
	ON_BN_CLICKED(IDC_MORPH_MODE, OnMode)
	ON_BN_CLICKED(IDC_MORPH_MODE2, OnMode)
	ON_BN_CLICKED(IDC_MORPH_MODE3, OnMode)
#else
	ON_EN_CHANGE(IDC_SCALE, OnChangeScale)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SCALESPIN, OnDeltaposScalespin)
#endif
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScaleVerticesDlg message handlers
#ifdef SLE //// SLE NEW - better vertex tool scaling
void CScaleVerticesDlg::OnChangeTransformX()
{
	SaveValues();

	CString str;
	m_cTransformX.GetWindowText(str);
	m_fTransformX = atof(str);

	if ( m_fTransformX <= 0 && m_iMode == 1 )
	{
		m_fTransformX = 1.0f;
	}

	// send command to document
	CMapDoc::GetActiveMapDoc()->OnCmdMsg(ID_VTRANSX_CHANGED, CN_COMMAND, NULL, NULL);
}

void CScaleVerticesDlg::OnChangeTransformY()
{
	SaveValues();

	CString str;
	m_cTransformY.GetWindowText(str);
	m_fTransformY = atof(str);

	if ( m_fTransformY <= 0 && m_iMode == 1 )
	{
		m_fTransformY = 1.0f;
	}

	// send command to document
	CMapDoc::GetActiveMapDoc()->OnCmdMsg(ID_VTRANSY_CHANGED, CN_COMMAND, NULL, NULL);
}

void CScaleVerticesDlg::OnChangeTransformZ()
{
	SaveValues();

	CString str;
	m_cTransformZ.GetWindowText(str);
	m_fTransformZ = atof(str);

	if ( m_fTransformZ <= 0 && m_iMode == 1 )
	{
		m_fTransformZ = 1.0f;
	}

	// send command to document
	CMapDoc::GetActiveMapDoc()->OnCmdMsg(ID_VTRANSZ_CHANGED, CN_COMMAND, NULL, NULL);
}

void CScaleVerticesDlg::OnDeltaposTransformXSpin(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_UPDOWN* pNMUpDown = ( NM_UPDOWN* )pNMHDR;

	CString str;
	m_cTransformX.GetWindowText(str);
	m_fTransformX = atof(str);
	m_fTransformX += 0.1f * float(pNMUpDown->iDelta);
	if ( m_fTransformX <= 0 && m_iMode == 1 )
		m_fTransformX = 1.0f;
	str.Format("%.3f", m_fTransformX);
	m_cTransformX.SetWindowText(str);

	*pResult = 0;
}

void CScaleVerticesDlg::OnDeltaposTransformYSpin(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_UPDOWN* pNMUpDown = ( NM_UPDOWN* )pNMHDR;

	CString str;
	m_cTransformY.GetWindowText(str);
	m_fTransformY = atof(str);
	m_fTransformY += 0.1f * float(pNMUpDown->iDelta);
	if ( m_fTransformY <= 0 && m_iMode == 1 )
		m_fTransformY = 1.0f;
	str.Format("%.3f", m_fTransformY);
	m_cTransformY.SetWindowText(str);

	*pResult = 0;
}

void CScaleVerticesDlg::OnDeltaposTransformZSpin(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_UPDOWN* pNMUpDown = ( NM_UPDOWN* )pNMHDR;

	CString str;
	m_cTransformZ.GetWindowText(str);
	m_fTransformZ = atof(str);
	m_fTransformZ += 0.1f * float(pNMUpDown->iDelta);
	if ( m_fTransformZ <= 0 && m_iMode == 1 )
		m_fTransformZ = 1.0f;
	str.Format("%.3f", m_fTransformZ);
	m_cTransformZ.SetWindowText(str);

	*pResult = 0;
}

BOOL CScaleVerticesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	if ( m_iMode == 1 )
	{
		m_cTransformX.SetWindowText("1.0");
		m_cTransformY.SetWindowText("1.0");
		m_cTransformZ.SetWindowText("1.0");
	}
	else
	{
		m_cTransformX.SetWindowText("0.0");
		m_cTransformY.SetWindowText("0.0");
		m_cTransformZ.SetWindowText("0.0");
	}

	m_cTransformXSpin.SetRange(UD_MINVAL, UD_MAXVAL);
	m_cTransformYSpin.SetRange(UD_MINVAL, UD_MAXVAL);
	m_cTransformZSpin.SetRange(UD_MINVAL, UD_MAXVAL);

	return TRUE;
}

void CScaleVerticesDlg::SaveValues()
{
//	AfxGetApp()->WriteProfileInt(pszSection, "Mode_Value", m_iMode);
	AfxGetApp()->WriteProfileInt(pszSection, "Mode_Value", 1); // disable saving at the moment
}

void CScaleVerticesDlg::OnMode(void)
{
	// should be redundant, but the radio button just refused to work
	if ( IsDlgButtonChecked(IDC_MORPH_MODE) ) m_iMode = 0;
	else if ( IsDlgButtonChecked(IDC_MORPH_MODE2) ) m_iMode = 1;
	else if ( IsDlgButtonChecked(IDC_MORPH_MODE3) ) m_iMode = 2;
	
	if ( m_iMode == 1 )
	{
		m_cTransformX.SetWindowText("1.0");
		m_cTransformY.SetWindowText("1.0");
		m_cTransformZ.SetWindowText("1.0");
	}
	else
	{
		m_fTransformX = 0;
		m_fTransformY = 0;
		m_fTransformZ = 0;
		m_cTransformX.SetWindowText("0.0");
		m_cTransformY.SetWindowText("0.0");
		m_cTransformZ.SetWindowText("0.0");
	}

	SaveValues();
}
#else
void CScaleVerticesDlg::OnChangeScale() 
{
	CString str;
	m_cScale.GetWindowText(str);
	m_fScale = atof(str);

	if (m_fScale <= 0)
	{
		m_fScale = (float)0.005;
	}

	// send command to document
	CMapDoc::GetActiveMapDoc()->OnCmdMsg(ID_VSCALE_CHANGED, CN_COMMAND, NULL, NULL);
}

void CScaleVerticesDlg::OnDeltaposScalespin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	CString str;
	m_cScale.GetWindowText(str);
	m_fScale = atof(str);
	m_fScale += 0.1f * float(pNMUpDown->iDelta);
	if(m_fScale <= 0)
		m_fScale = 0;
	str.Format("%.3f", m_fScale);
	m_cScale.SetWindowText(str);

	*pResult = 0;
}

BOOL CScaleVerticesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_cScale.SetWindowText("1.0");
	m_cScaleSpin.SetRange(UD_MINVAL, UD_MAXVAL);

	return TRUE;
}
#endif
#ifdef SLE //// SLE NEW - Standard Ok/Cancel buttons for the dialogue; Cancel zeroes the changes
void CScaleVerticesDlg::OnCancel() 
{
	m_cTransformX.SetWindowText("0.0");
	m_cTransformY.SetWindowText("0.0");
	m_cTransformZ.SetWindowText("0.0");
	m_fTransformX = 0.0f;
	m_fTransformY = 0.0f;
	m_fTransformZ = 0.0f;

	CDialog::OnCancel();
}
#endif
void CScaleVerticesDlg::OnClose() 
{
	CDialog::OnClose();
}
