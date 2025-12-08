//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "stdafx.h"
#include "hammer.h"
#include "hammer_mathlib.h"
#include "TorusDlg.h"
#ifdef SLE
#include "mapdoc.h" //// for viewports updating when previewing
#include "Options.h" //// for toggling the stretch arches/toruses setting
#endif
// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

static LPCTSTR pszSection = "Torus";

void MakeArcCenterRadius(float xCenter, float yCenter, float xrad, float yrad, int npoints, float start_ang, float fArc, float points[][2]);
void MakeArc(float x1, float y1, float x2, float y2, int npoints, float start_ang, float fArc, float points[][2]);

CTorusDlg::CTorusDlg(Vector& boxmins, Vector& boxmaxs, CWnd* pParent /*=NULL*/)
	: CDialog(CTorusDlg::IDD, pParent)
{
	bmins = boxmins;
	bmaxs = boxmaxs;

	//{{AFX_DATA_INIT(CTorusDlg)
	m_iSides = 0;
	m_iWallWidth = 0;
	m_iAddHeight = 0;
	m_fArc = 0.0f;
	m_fAngle = 0.0f;
	m_fRotationArc = 0.0f;
	m_fRotationAngle = 0.0f;
	m_iRotationSides = 0;
	m_fCrossSectionRadius = 0;
	//}}AFX_DATA_INIT

	// load up old defaults
	CString str;
	m_iWallWidth = AfxGetApp()->GetProfileInt(pszSection, "Wall Width", 32);
	str = AfxGetApp()->GetProfileString(pszSection, "Arc_", "360");
	m_fArc = atof(str);
	m_iSides = AfxGetApp()->GetProfileInt(pszSection, "Sides", 16);
	str = AfxGetApp()->GetProfileString(pszSection, "Start Angle_", "0");
	m_fAngle = atof(str);

	str = AfxGetApp()->GetProfileString(pszSection, "Rotation Arc_", "360");
	m_fRotationArc = atof(str);
	m_iRotationSides = AfxGetApp()->GetProfileInt(pszSection, "Rotation Sides", 16);
	str = AfxGetApp()->GetProfileString(pszSection, "Rotation Start Angle_", "0");
	m_fRotationAngle = atof(str);
	
	m_iAddHeight = AfxGetApp()->GetProfileInt(pszSection, "Add Height", 0);

	Vector vecSize;
	VectorSubtract( bmaxs, bmins, vecSize );
	if ( m_iAddHeight > vecSize.z )
	{
		m_iAddHeight = vecSize.z;
	}

	// This is the maximum cross-section radius
	m_fCrossSectionRadius = MaxTorusCrossSectionRadius();
}

void CTorusDlg::SaveValues()
{
	CString str;
	AfxGetApp()->WriteProfileInt(pszSection, "Wall Width", m_iWallWidth);
	str.Format("%f", m_fArc);
	AfxGetApp()->WriteProfileString(pszSection, "Arc_", str);
	AfxGetApp()->WriteProfileInt(pszSection, "Sides", m_iSides);
	str.Format("%f", m_fAngle);
	AfxGetApp()->WriteProfileString(pszSection, "Start Angle_", str);

	str.Format("%f", m_fRotationArc);
	AfxGetApp()->WriteProfileString(pszSection, "Rotation Arc_", str);
	str.Format("%f", m_fRotationAngle);
	AfxGetApp()->WriteProfileString(pszSection, "Rotation Start Angle_", str);
	AfxGetApp()->WriteProfileInt(pszSection, "Rotation Sides", m_iRotationSides);
	AfxGetApp()->WriteProfileInt(pszSection, "Add Height", m_iAddHeight);
#ifdef SLE //// SLE NEW - torus preview needs cross section radius saved	
	str.Format("%f", m_fCrossSectionRadius);
	AfxGetApp()->WriteProfileString(pszSection, "Cross Section Radius", str);
#endif
}

void CTorusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTorusDlg)
	DDX_Control(pDX, IDC_TORUS_ANGLESPIN,		m_cStartAngleSpin);
	DDX_Control(pDX, IDC_TORUS_WALLWIDTHSPIN,	m_cWallWidthSpin);
	DDX_Control(pDX, IDC_WALLWIDTH,				m_cWallWidth);
	DDX_Control(pDX, IDC_TORUS_SIDESSPIN,		m_cSidesSpin);
	DDX_Control(pDX, IDC_SIDES,					m_cSides);
	DDX_Control(pDX, IDC_TORUS_ARCSPIN,			m_cArcSpin);
	DDX_Control(pDX, IDC_ARC,					m_cArc);
	DDX_Control(pDX, IDC_PREVIEW,				m_cPreview);
	DDX_Control(pDX, IDC_PREVIEW_TOP_VIEW,		m_cTopViewPreview);
	DDX_Text(pDX, IDC_SIDES,					m_iSides);
	DDV_MinMaxInt(pDX,							m_iSides, 3, 2048);
	DDX_Text(pDX, IDC_WALLWIDTH,				m_iWallWidth);
	DDX_Text(pDX, IDC_ADDHEIGHT,				m_iAddHeight);
	DDX_Text(pDX, IDC_ARC,						m_fArc);
	DDV_MinMaxFloat(pDX,						m_fArc, 8.f, 360.f);
	DDX_Text(pDX, IDC_ANGLE,					m_fAngle);
	DDV_MinMaxFloat(pDX,						m_fAngle, -360.0f, 360.f);
	DDX_Text(pDX, IDC_ROTATION_ARC,				m_fRotationArc);
	DDV_MinMaxFloat(pDX,						m_fRotationArc, 0.f, 3600.f);
	DDX_Text(pDX, IDC_ROTATION_ANGLE,			m_fRotationAngle);
	DDV_MinMaxFloat(pDX,						m_fRotationAngle, -360.0f, 360.f);
	DDX_Text(pDX, IDC_ROTATION_SIDES,			m_iRotationSides);
	DDV_MinMaxInt(pDX,							m_iRotationSides, 3, 2048);
	DDX_Text(pDX, IDC_CROSS_SECTION_RADIUS,		m_fCrossSectionRadius);
	DDV_MinMaxFloat(pDX,						m_fCrossSectionRadius, 0.f, 5000.f);
#ifdef SLE //// SLE NEW - angle controls for Torus dialogue
	DDX_Control(pDX, IDC_TORUS_CROSSSPIN,		m_cCrossSectionSpin);
	DDX_Control(pDX, IDC_TORUS_ROTATIONSIDES_SPIN,	m_cRotationSidesSpin);
	DDX_Control(pDX, IDC_TORUS_ROTATIONARC_SPIN,	m_cRotationArcSpin);
	DDX_Control(pDX, IDC_TORUS_ROTATIONANGLE_SPIN,	m_cRotationAngleSpin);
	DDX_Control(pDX, IDC_TORUS_ADDHEIGHT_SPIN,		m_cAddHeightSpin);
	//// toggle stretch option from the dialogue
	DDX_Check(pDX, IDC_STRETCH_ARCH, Options.general.bStretchArches);
#endif
	//}}AFX_DATA_MAP

	if ( pDX->m_bSaveAndValidate )
	{
		Vector vecSize;
		VectorSubtract( bmaxs, bmins, vecSize );
		if ( m_iAddHeight > vecSize.z )
		{
			m_iAddHeight = vecSize.z;
		}
	}
}

BEGIN_MESSAGE_MAP(CTorusDlg, CDialog)
	//{{AFX_MSG_MAP(CTorusDlg)
	ON_EN_CHANGE(IDC_ARC, OnChangeArc)
	ON_EN_CHANGE(IDC_ROTATION_ARC, OnChangeTorusArc)
	ON_BN_CLICKED(IDC_CIRCLE, OnCircle)
	ON_BN_CLICKED(IDC_TORUS_COMPUTE_RADIUS, OnComputeRadius)
	ON_EN_UPDATE(IDC_SIDES, OnUpdateSides)
	ON_EN_UPDATE(IDC_WALLWIDTH, OnUpdateWallwidth)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_TORUS_PREVIEW, OnTorusPreview)	
#ifdef SLE //// SLE NEW - similar to Arc dlg, update preview any time settings are changed
	ON_EN_UPDATE(IDC_CROSS_SECTION_RADIUS, OnChangeCrossSection)
	ON_EN_UPDATE(IDC_ADDHEIGHT, OnChangeAddHeight)
	ON_EN_UPDATE(IDC_ANGLE, OnChangeStartAngle)
	ON_EN_UPDATE(IDC_ROTATION_ANGLE, OnChangeRotationAngle)
	ON_EN_UPDATE(IDC_ROTATION_SIDES, OnChangeRotationSides)
#endif
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTorusDlg message handlers

void CTorusDlg::OnChangeArc() 
{
#ifdef SLE //// SLE NEW - similar to Arc dlg, update preview any time settings are changed
	OnTorusPreview();
#endif
}

void CTorusDlg::OnChangeTorusArc() 
{
#ifdef SLE //// SLE NEW - similar to Arc dlg, update preview any time settings are changed
	OnTorusPreview();
#endif
}

#ifdef SLE //// SLE NEW - similar to Arc dlg, update preview any time settings are changed
void CTorusDlg::OnChangeCrossSection()
{
	OnTorusPreview();
}

void CTorusDlg::OnChangeAddHeight()
{
	OnTorusPreview();
}

void CTorusDlg::OnChangeStartAngle()
{
	OnTorusPreview();
}

void CTorusDlg::OnChangeRotationAngle()
{
	OnTorusPreview();
}

void CTorusDlg::OnChangeRotationSides()
{
	OnTorusPreview();
}
#endif

void CTorusDlg::OnCircle() 
{
	m_cArcSpin.SetPos(360);
	OnTorusPreview();
}

void CTorusDlg::OnComputeRadius() 
{
	UpdateData( TRUE );

	// This is the maximum cross-section radius
	m_fCrossSectionRadius = MaxTorusCrossSectionRadius();
	UpdateData( FALSE );
	OnTorusPreview();
}

void CTorusDlg::OnUpdateSides() 
{
#ifdef SLE //// SLE NEW - similar to Arc dlg, update preview any time settings are changed
	OnTorusPreview();
#endif
}

void CTorusDlg::OnUpdateWallwidth() 
{
#ifdef SLE //// SLE NEW - similar to Arc dlg, update preview any time settings are changed
	OnTorusPreview();
#endif
}

BOOL CTorusDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_cArcSpin.SetRange(8, 360);
#ifdef SLE //// SLE CHANGE - allow toruses with 2 sides, nothing wrong with that
	m_cSidesSpin.SetRange(2, 100);
#else
	m_cSidesSpin.SetRange(3, 100);
#endif
	m_cWallWidthSpin.SetRange(2, m_iMaxWallWidth);
#ifdef SLE //// SLE CHANGE - allow negative direction
	m_cStartAngleSpin.SetRange(-360, 360);
#else
	m_cStartAngleSpin.SetRange(0, 360);
#endif
#ifdef SLE //// SLE NEW - angle controls for Torus dialogue
	m_cCrossSectionSpin.SetRange(0, 16384);
	m_cRotationSidesSpin.SetRange(0, 128);
	m_cRotationArcSpin.SetRange(0, 360);
	m_cRotationAngleSpin.SetRange(-360, 360);
	m_cAddHeightSpin.SetRange(-16384, 16384);
#endif
	m_cPreview.ShowWindow(SW_HIDE);
	m_cTopViewPreview.ShowWindow(SW_HIDE);

	bInitialized = TRUE;
	return TRUE;
}

void CTorusDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CBrush black(RGB(0,0,0));
	CBrush grey(RGB(128,128,128));

	// Do not call CDialog::OnPaint() for painting messages
	CRect rcPreview;
	m_cPreview.GetWindowRect(&rcPreview);
	ScreenToClient(&rcPreview);
	dc.FillRect(rcPreview, &black);

	DrawTorusCrossSection( &dc );

	rcPreview.InflateRect(1,1);
	dc.FrameRect(rcPreview, &grey);
	ValidateRect(rcPreview);

	m_cTopViewPreview.GetWindowRect(&rcPreview);
	ScreenToClient(&rcPreview);
	dc.FillRect(rcPreview, &black);

	DrawTorusTopView( &dc );

	rcPreview.InflateRect(1,1);
	dc.FrameRect(rcPreview, &grey);
	ValidateRect(rcPreview);

	bInitialized = TRUE;
}

void CTorusDlg::OnTorusPreview() 
{
	// 
	// Build preview.
	//
	bInitialized = TRUE;
	InvalidateRect(NULL);
	UpdateWindow();
#ifdef SLE //// update viewports for the Block Tool preview	
	SaveValues();
	CMapDoc *pDoc = CMapDoc::GetActiveMapDoc();
	if ( pDoc )
	{
		pDoc->UpdateAllViews(MAPVIEW_UPDATE_TOOL);
		pDoc->RenderAllViews();
	}
#endif
}

CTorusDlg::~CTorusDlg()
{
}

//-----------------------------------------------------------------------------
// Gets the inner radius of the torus cross-section
//-----------------------------------------------------------------------------
float CTorusDlg::MaxTorusCrossSectionRadius() const
{
	Vector vecSize;
	VectorSubtract( bmaxs, bmins, vecSize );

	float flTorusRadius = (vecSize.z - m_iAddHeight);

	// Check for multiple revolutions...
	if ( ( m_fRotationArc > 360 ) || (( m_fRotationArc == 360 ) && ( m_iAddHeight != 0 )) )
	{
		// Height per revolution
		float flHeightPerRev = 360.0f * m_iAddHeight / m_fRotationArc;
		if ( flHeightPerRev < flTorusRadius )
		{
			flTorusRadius = flHeightPerRev;
		}
	}

	// Also constrain it based on x & y too...
	if ( (vecSize.x * 0.5f) < flTorusRadius )
	{
		flTorusRadius = vecSize.x * 0.5f;
	}

	if ( (vecSize.y * 0.5f) < flTorusRadius )
	{
		flTorusRadius = vecSize.y * 0.5f;
	}

	flTorusRadius *= 0.5f;
	if ( flTorusRadius < m_iWallWidth )
	{
		flTorusRadius = m_iWallWidth;
	}

	return flTorusRadius;
}

//-----------------------------------------------------------------------------
// Gets the inner radius of the torus cross-section
//-----------------------------------------------------------------------------
float CTorusDlg::GetTorusCrossSectionRadius() const
{
	Vector vecSize;
	VectorSubtract( bmaxs, bmins, vecSize );

	float flTorusRadius = m_fCrossSectionRadius;

	// Also constrain it based on x & y too...
	if ( (vecSize.x * 0.25f) < flTorusRadius )
	{
		flTorusRadius = vecSize.x * 0.25f;
	}

	if ( (vecSize.y * 0.25f) < flTorusRadius )
	{
		flTorusRadius = vecSize.y * 0.25f;
	}

	return flTorusRadius;
}

//-----------------------------------------------------------------------------
// Draws the torus cross-section
//-----------------------------------------------------------------------------
void CTorusDlg::DrawTorusCrossSection(CDC* pDC )
{
	int iSides, iWallWidth;
	float fArc, fStartAngle;

	int i;
	float fOuterPoints[ARC_MAX_POINTS][2];
	float fInnerPoints[ARC_MAX_POINTS][2];

	float fScaleX;
	float fScaleY;

	UpdateData(TRUE);

	CPen m_hPen, *pOldPen;

	m_hPen.CreatePen(PS_SOLID, 1, RGB(255,255,255));

	pOldPen = pDC->SelectObject(&m_hPen);

	CRect rcItem;
	m_cPreview.GetWindowRect(&rcItem);
	ScreenToClient(&rcItem);

	CPoint pt;
	pt.x = rcItem.left + rcItem.Width()  / 2;
	pt.y = rcItem.top  + rcItem.Height() / 2;

	float flMaxRadius = GetTorusCrossSectionRadius();
	iWallWidth = m_iWallWidth;
	if ( iWallWidth > flMaxRadius )
		iWallWidth = flMaxRadius;
	float flTorusRadius = flMaxRadius - iWallWidth;

	float flDeltaZ = bmaxs[2] - bmins[2];
	if (flDeltaZ)
	{
		if ( flDeltaZ < flMaxRadius * 2.0f )
		{
			flDeltaZ = flMaxRadius * 2.0f;
		}
		fScaleX = rcItem.Width()/flDeltaZ;
		fScaleY = rcItem.Height()/flDeltaZ;
	}
	else
	{
		fScaleX = fScaleY = 1.0f;
//		fScaleX = rcItem.Width() / (2.0f * flMaxRadius);
//		fScaleY = rcItem.Height() / (2.0f * flMaxRadius);
	}

	fArc = m_fArc;
	fStartAngle = m_fAngle;
	iSides = m_iSides;

	MakeArcCenterRadius(0, 0,
		flTorusRadius + iWallWidth, flTorusRadius + iWallWidth, 
		iSides, fStartAngle, fArc, fOuterPoints);

	MakeArcCenterRadius(0, 0, 
		flTorusRadius, flTorusRadius, 
		iSides, fStartAngle, fArc, fInnerPoints);

	// check wall width - if it's half or more of the total,
	//  set the inner poinst to the center point of the box
	//  and turn off the CreateSouthFace flag
		
	Vector points[4];	
	for (i = 0; i < iSides; i++)
	{
		int iNextPoint = i+1;
		if (iNextPoint >= iSides + 1)
		{
			iNextPoint = 0;
		}

		points[0][0] = fOuterPoints[i][0];
		points[0][1] = fOuterPoints[i][1];

		points[1][0] = fOuterPoints[iNextPoint][0];
		points[1][1] = fOuterPoints[iNextPoint][1];

		points[2][0] = fInnerPoints[iNextPoint][0];
		points[2][1] = fInnerPoints[iNextPoint][1];

		points[3][0] = fInnerPoints[i][0];
		points[3][1] = fInnerPoints[i][1];

		for (int j = 0; j < 4; j++)
		{
			points[j][0] = fScaleX * points[j][0];
			points[j][1] = fScaleY * points[j][1];
		}

		pDC->MoveTo(pt.x + (int)points[1][0], pt.y - (int)points[1][1]);
		pDC->LineTo(pt.x + (int)points[0][0], pt.y - (int)points[0][1]);
		pDC->LineTo(pt.x + (int)points[3][0], pt.y - (int)points[3][1]);
		pDC->LineTo(pt.x + (int)points[2][0], pt.y - (int)points[2][1]);
	}

	// Close the cross-section off...
	if ( fArc != 360.0f )
	{
		pDC->LineTo(pt.x + (int)points[1][0], pt.y - (int)points[1][1]);
	}

	pDC->SelectObject(pOldPen);
}

void CTorusDlg::DrawTorusTopView( CDC* pDC )
{
	int i;
	float fOuterPoints[ARC_MAX_POINTS][2];
	float fInnerPoints[ARC_MAX_POINTS][2];

	float fScaleX;
	float fScaleY;

	UpdateData(TRUE);

	CPen m_hPen, *pOldPen;

	m_hPen.CreatePen(PS_SOLID, 1, RGB(255,255,255));

	pOldPen = pDC->SelectObject(&m_hPen);

	CRect rcItem;
	m_cTopViewPreview.GetWindowRect(&rcItem);
	ScreenToClient(&rcItem);

	CPoint pt;
	pt.x = rcItem.left + rcItem.Width()  / 2;
	pt.y = rcItem.top  + rcItem.Height() / 2;

	if (bmaxs[0] - bmins[0])
	{
		fScaleX = rcItem.Width() /(bmaxs[0] - bmins[0]);
	}
	else
	{
		fScaleX = 1.0f;
	}
	
	if (bmaxs[1] - bmins[1])
	{
		fScaleY = rcItem.Height() /(bmaxs[1] - bmins[1]);
	}
	else
	{
		fScaleY = 1.0f;
	}

	int iSides, iWallWidth;
	float fArc, fStartAngle;

	fArc = m_fRotationArc;
	fStartAngle = m_fRotationAngle;
	iSides = m_iRotationSides;
	iWallWidth = GetTorusCrossSectionRadius() * 2.0f;

	float xCenter = (bmaxs[0] + bmins[0]) * 0.5f;
	float yCenter = (bmaxs[1] + bmins[1]) * 0.5f;
	float xRad = (bmaxs[0] - xCenter - iWallWidth);
	float yRad = (bmaxs[1] - yCenter - iWallWidth);
	if (xRad < 0.0f )
	{
		xRad = 0.0f;
	}
	if (yRad < 0.0f )
	{
		yRad = 0.0f;
	}

	MakeArcCenterRadius(xCenter, yCenter, xRad + iWallWidth, yRad + iWallWidth, 
		iSides,	fStartAngle, fArc, fOuterPoints);

	MakeArcCenterRadius(xCenter, yCenter, xRad, yRad,
		iSides, fStartAngle, fArc, fInnerPoints);
	
	Vector vecCenter;
	VectorLerp( bmins, bmaxs, 0.5f, vecCenter );
	
	Vector points[4];	
	for (i = 0; i < iSides; i++)
	{
		int iNextPoint = i+1;
		if (iNextPoint >= iSides + 1)
		{
			iNextPoint = 0;
		}

		points[0][0] = fOuterPoints[i][0];
		points[0][1] = fOuterPoints[i][1];

		points[1][0] = fOuterPoints[iNextPoint][0];
		points[1][1] = fOuterPoints[iNextPoint][1];

		points[2][0] = fInnerPoints[iNextPoint][0];
		points[2][1] = fInnerPoints[iNextPoint][1];

		points[3][0] = fInnerPoints[i][0];
		points[3][1] = fInnerPoints[i][1];

		for (int j = 0; j < 4; j++)
		{
			points[j][0] = fScaleX * (points[j][0] - vecCenter[0]);
			points[j][1] = fScaleY * (points[j][1] - vecCenter[1]);
		}

		pDC->MoveTo(pt.x + (int)points[1][0], pt.y - (int)points[1][1]);
		pDC->LineTo(pt.x + (int)points[0][0], pt.y - (int)points[0][1]);
		pDC->LineTo(pt.x + (int)points[3][0], pt.y - (int)points[3][1]);
		pDC->LineTo(pt.x + (int)points[2][0], pt.y - (int)points[2][1]);
	}

	// Close the cross-section off...
	if ( fArc != 360.0f )
	{
		pDC->LineTo(pt.x + (int)points[1][0], pt.y - (int)points[1][1]);
	}

	pDC->SelectObject(pOldPen);
}
