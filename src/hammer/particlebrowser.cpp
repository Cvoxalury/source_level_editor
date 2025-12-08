//
// ParticleBrowser.cpp : implementation file
//

#include "stdafx.h"
#include "ParticleBrowser.h"
#include "matsys_controls/particlepicker.h"
#include "matsys_controls/matsyscontrols.h"
#include "vgui/ISurface.h"
#include "vgui_controls/TextEntry.h"
#include "vgui_controls/Splitter.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/Panel.h"
#include "KeyValues.h"
#include "vgui/KeyCode.h"
#include "texturesystem.h"
#include "HammerVGui.h"

static LPCTSTR pszIniSection = "Particle Browser";

// CParticleBrowser dialog

class CParticleBrowserPanel : public vgui::EditablePanel
{
public:
	CParticleBrowserPanel( CParticleBrowser *pBrowser, const char *panelName, vgui::HScheme hScheme ) : 
	  vgui::EditablePanel( NULL, panelName, hScheme )
	{
		m_pBrowser = pBrowser;
	}

	virtual	void OnSizeChanged(int newWide, int newTall)
	{
		// call Panel and not EditablePanel OnSizeChanged.
		Panel::OnSizeChanged(newWide, newTall);
	}

	virtual void OnCommand( const char *pCommand )
	{
		if ( Q_strcmp( pCommand, "OK" ) == 0 )
		{	
			m_pBrowser->EndDialog( IDOK );
		}
		else if ( Q_strcmp( pCommand, "Cancel" ) == 0 )
		{
			m_pBrowser->EndDialog( IDCANCEL );
		}
	}

	virtual void OnKeyCodeTyped(vgui::KeyCode code)
	{
		vgui::EditablePanel::OnKeyCodeTyped( code );

		if ( code == KEY_ENTER )
		{
			m_pBrowser->EndDialog( IDOK );
		}
		else if ( code == KEY_ESCAPE )
		{
			m_pBrowser->EndDialog( IDCANCEL );
		}
	}

	virtual void OnMessage(const KeyValues *params, vgui::VPANEL ifromPanel)
	{
		vgui::EditablePanel::OnMessage( params, ifromPanel );
		
		if ( Q_strcmp( params->GetName(), "ParticleSystemSelectionChanged" ) == 0 ) 
		{
			m_pBrowser->UpdateStatusLine();
		}
	}

	CParticleBrowser *m_pBrowser;
};

IMPLEMENT_DYNAMIC(CParticleBrowser, CDialog)
CParticleBrowser::CParticleBrowser(CWnd* pParent /*=NULL*/)
	: CDialog(CParticleBrowser::IDD, pParent)
{
#ifdef ENABLE_PARTICLE_BROWSER_PICKER
	m_pPicker = new CParticlePicker(NULL);
#endif
	m_pStatusLine = new vgui::TextEntry(NULL, "StatusLine");

	m_pButtonOK = new vgui::Button(NULL, "OpenButton", "OK");
	m_pButtonCancel = new vgui::Button(NULL, "CancelButton", "Cancel");
}

CParticleBrowser::~CParticleBrowser()
{
	// CDialog isn't going to clean up its vgui children
#ifdef ENABLE_PARTICLE_BROWSER_PICKER
	delete m_pPicker;
#endif
	delete m_pStatusLine;
	delete m_pButtonOK;
	delete m_pButtonCancel;
}

void CParticleBrowser::SetParticleSysName(const char *pParticleSysName)
{
	char pTempName[255];
	strcpy(pTempName, pParticleSysName);

	char * pSelectedParticleSys = strchr(pTempName, '/');
	if (pSelectedParticleSys)
	{
		pSelectedParticleSys += 1;
		Q_FixSlashes(pSelectedParticleSys, '\\');
	}

#ifdef ENABLE_PARTICLE_BROWSER_PICKER
	m_pPicker->PostActionSignal(new KeyValues("SelectedParticleSysChanged", "particle", pParticleSysName));

	m_pPicker->SelectParticleSys(pParticleSysName);
	m_pPicker->SetInitialSelection(pSelectedParticleSys);
#endif

	m_pStatusLine->SetText(pParticleSysName);
}

void CParticleBrowser::GetParticleSysName(char *pParticleName, int length)
{
	//	Assert(length > 0);

	//	if (m_pPicker->GetSelectedAssetCount() > 0)
	//	{
	//		Q_snprintf(pParticleName, length, "%s", m_pPicker->GetSelectedAsset());
	//	}
	//	else
	//	{
	//		pParticleName[0] = 0;
	//	}

#ifdef ENABLE_PARTICLE_BROWSER_PICKER
	m_pPicker->GetSelectedParticleSysName(pParticleName, length);
#endif
	Q_FixSlashes(pParticleName, '/');
}

void CParticleBrowser::UpdateStatusLine()
{
	char szParticle[1024];

	//	if (m_pPicker->GetSelectedAssetCount() > 0)
	//	{
	//		Q_snprintf(szParticle, sizeof(szParticle), "%s", m_pPicker->GetSelectedAsset());
	//	}
	//	else
	//	{
	//		szParticle[0] = 0;
	//	}

#ifdef ENABLE_PARTICLE_BROWSER_PICKER
	m_pPicker->GetSelectedParticleSysName(szParticle, sizeof(szParticle));
#endif
	m_pStatusLine->SetText(szParticle);
}

void CParticleBrowser::SaveLoadSettings(bool bSave)
{
	CString	str;
	CRect	rect;
	CWinApp	*pApp = AfxGetApp();

	if (bSave)
	{
		GetWindowRect(rect);
		str.Format("%d %d %d %d", rect.left, rect.top, rect.right, rect.bottom);
		pApp->WriteProfileString(pszIniSection, "Position", str);
#ifdef ENABLE_PARTICLE_BROWSER_PICKER
		pApp->WriteProfileString(pszIniSection, "Filter", m_pPicker->GetFilter());
#endif
	}
	else
	{
		str = pApp->GetProfileString(pszIniSection, "Position");

		if (!str.IsEmpty())
		{
			sscanf(str, "%d %d %d %d", &rect.left, &rect.top, &rect.right, &rect.bottom);
			MoveWindow(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, FALSE);
			Resize();
		}

		str = pApp->GetProfileString(pszIniSection, "Filter");

		if (!str.IsEmpty())
		{
#ifdef ENABLE_PARTICLE_BROWSER_PICKER
			m_pPicker->SetFilter(str);
#endif
		}
	}
}

void CParticleBrowser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void CParticleBrowser::Resize()
{
	// reposition controls
	CRect rect;
	GetClientRect(&rect);

	m_VGuiWindow.MoveWindow(rect);

#ifdef ENABLE_PARTICLE_BROWSER_PICKER
	m_pPicker->SetBounds(0, 0, rect.Width(), rect.Height() - 32);
#endif
	m_pButtonCancel->SetPos(8, rect.Height() - 30);
	m_pButtonOK->SetPos(84, rect.Height() - 30);
	m_pStatusLine->SetBounds(160, rect.Height() - 30, max(100, rect.Width() - 166), 24);
}

void CParticleBrowser::OnSize(UINT nType, int cx, int cy) 
{
	if (nType == SIZE_MINIMIZED || !IsWindow(m_VGuiWindow.m_hWnd) )
	{
		CDialog::OnSize(nType, cx, cy);
		return;
	}

	Resize();

	CDialog::OnSize(nType, cx, cy);
}

BOOL CParticleBrowser::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

BEGIN_MESSAGE_MAP(CParticleBrowser, CDialog)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL CParticleBrowser::PreTranslateMessage( MSG* pMsg )
{
	// don't filter dialog message
	return CWnd::PreTranslateMessage( pMsg );
}

BOOL CParticleBrowser::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_VGuiWindow.Create(NULL, _T("ParticleViewer"), WS_VISIBLE | WS_CHILD, CRect(0, 0, 100, 100), this, IDD_PARTICLE_BROWSER);

	vgui::EditablePanel *pMainPanel = new CParticleBrowserPanel(this, "ParticleBrowerPanel", HammerVGui()->GetHammerScheme());

	m_VGuiWindow.SetParentWindow(&m_VGuiWindow);
	m_VGuiWindow.SetMainPanel(pMainPanel);
	pMainPanel->MakePopup(false, false);
	m_VGuiWindow.SetRepaintInterval(5);

#ifdef ENABLE_PARTICLE_BROWSER_PICKER
	m_pPicker->SetParent(pMainPanel);
	m_pPicker->AddActionSignalTarget(pMainPanel);
#endif

	m_pButtonOK->SetParent(pMainPanel);
	m_pButtonOK->AddActionSignalTarget(pMainPanel);
	m_pButtonOK->SetCommand("OK");

	m_pButtonCancel->SetParent(pMainPanel);
	m_pButtonCancel->AddActionSignalTarget(pMainPanel);
	m_pButtonCancel->SetCommand("Cancel");

	m_pStatusLine->SetParent(pMainPanel);
	m_pStatusLine->SetEditable(false);

	SaveLoadSettings(false); // load

#ifdef ENABLE_PARTICLE_BROWSER_PICKER
	/*
	auto s = new vgui::CBoxSizer(vgui::ESLD_VERTICAL);
	auto s2 = new vgui::CBoxSizer(vgui::ESLD_HORIZONTAL);
	s->AddPanel(m_pPicker, vgui::SizerAddArgs_t().Expand(1.0f).Padding(0));

	s2->AddPanel(m_pButtonOK, vgui::SizerAddArgs_t().Padding(0));
	s2->AddSpacer(vgui::SizerAddArgs_t().Padding(2));
	s2->AddPanel(m_pButtonCancel, vgui::SizerAddArgs_t().Padding(0));
	s2->AddSpacer(vgui::SizerAddArgs_t().Padding(2));
	s2->AddPanel(m_pStatusLine, vgui::SizerAddArgs_t().Expand(1.0f).Padding(0));

	s->AddSpacer(vgui::SizerAddArgs_t().Padding(2));
	s->AddSizer(s2, vgui::SizerAddArgs_t().Padding(2));

	pMainPanel->SetSizer(s);
	pMainPanel->InvalidateLayout(true);
	*/
	m_pPicker->Activate();
#endif

	return TRUE;
}

void CParticleBrowser::OnDestroy()
{
	SaveLoadSettings(true); // save

	// model browser destroys our default cubemap, reload it
	g_Textures.RebindDefaultCubeMap();

	CDialog::OnDestroy();
}

void CParticleBrowser::Show()
{
#ifdef ENABLE_PARTICLE_BROWSER_PICKER
	if (m_pPicker)
	{
		m_pPicker->SetVisible(true);
	}
#endif
	if (m_pStatusLine)
		m_pStatusLine->SetVisible(true);
	if (m_pButtonOK)
		m_pButtonOK->SetVisible(true);
	if (m_pButtonCancel)
		m_pButtonCancel->SetVisible(true);

}
void CParticleBrowser::Hide()
{
#ifdef ENABLE_PARTICLE_BROWSER_PICKER
	if (m_pPicker)
		m_pPicker->SetVisible(false);
#endif
	if (m_pStatusLine)
		m_pStatusLine->SetVisible(false);

	if (m_pButtonOK)
		m_pButtonOK->SetVisible(false);

	if (m_pButtonCancel)
		m_pButtonCancel->SetVisible(false);
}
