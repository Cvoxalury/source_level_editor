//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

class CRunMap : public CDialog
{
// Construction
public:
	CRunMap(CWnd* pParent = NULL);   // standard constructor
	void SaveToIni();

// Dialog Data
	//{{AFX_DATA(CRunMap)
	enum { IDD = IDD_RUNMAP };
	int		m_iVis;
	BOOL	m_bDoNotRunGame;
	CString	m_strQuakeParms;
	int		m_iLight;
	int		m_iQBSP;
	BOOL	m_bHDRLight;
#ifdef SLE  //// SLE NEW - extra normal compile options
	BOOL	m_bBSPAllow;
	BOOL	m_bBSPNoCubemap;
	BOOL	m_bVRadLDR;
	BOOL	m_bVRadTS;
	BOOL	m_bVRadSPL;
	BOOL	m_bVRadSPP;
	BOOL	m_bVRadNOSR;
	BOOL	m_bUseProcessWnd;
	CString m_strVisThreads;
	CString m_strVRadThreads;
#endif
	//}}AFX_DATA

	BOOL m_bSwitchMode;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRunMap)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRunMap)
	afx_msg void OnExpert();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};