//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "stdafx.h"
#include "resource.h"
#include "Splash.h"
#ifdef SLE //// SLE NEW - easter egg splash screens
#include "Options.h"
#include "vstdlib\random.h"
#include "tier0/icommandline.h"
#endif
#define SPLASH_MIN_SHOW_TIME_MS	500

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>
#ifdef SLE //// SLE CHANGE - bring back the midi
#define HAMMER_TIME 0
#endif
#ifdef HAMMER_TIME
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "StatusBarIDs.h"

unsigned char g_CantTouchThis[] = 
{
	0x4D, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x0B, 0x00, 0xF0, 0x4D, 0x54,
	0x72, 0x6B, 0x00, 0x00, 0x00, 0x13, 0x00, 0xFF, 0x58, 0x04, 0x04, 0x02, 0x18, 0x08, 0x00, 0xFF,
	0x51, 0x03, 0x06, 0xC8, 0x1C, 0x00, 0xFF, 0x2F, 0x00, 0x4D, 0x54, 0x72, 0x6B, 0x00, 0x00, 0x00,
	0x6E, 0x00, 0xFF, 0x03, 0x05, 0x45, 0x20, 0x50, 0x6E, 0x6F, 0x00, 0xC1, 0x02, 0x84, 0x58, 0x91,
	0x48, 0x39, 0x00, 0x4C, 0x43, 0x00, 0x51, 0x47, 0x3C, 0x48, 0x00, 0x00, 0x4C, 0x00, 0x00, 0x51,
	0x00, 0x3C, 0x48, 0x33, 0x00, 0x4C, 0x44, 0x00, 0x51, 0x48, 0x3C, 0x48, 0x00, 0x00, 0x4C, 0x00,
	0x00, 0x51, 0x00, 0x82, 0x2C, 0x47, 0x2E, 0x00, 0x4A, 0x3B, 0x00, 0x4F, 0x3B, 0x3C, 0x47, 0x00,
	0x00, 0x4A, 0x00, 0x00, 0x4F, 0x00, 0x3C, 0x47, 0x37, 0x00, 0x4A, 0x4B, 0x00, 0x4F, 0x46, 0x3C,
	0x47, 0x00, 0x00, 0x4A, 0x00, 0x00, 0x4F, 0x00, 0x82, 0x2C, 0x48, 0x3C, 0x00, 0x4C, 0x41, 0x00,
	0x51, 0x39, 0x3C, 0x48, 0x00, 0x00, 0x4C, 0x00, 0x00, 0x51, 0x00, 0x00, 0xFF, 0x2F, 0x00, 0x4D,
	0x54, 0x72, 0x6B, 0x00, 0x00, 0x00, 0x29, 0x00, 0xFF, 0x03, 0x0A, 0x4C, 0x65, 0x61, 0x64, 0x20,
	0x73, 0x79, 0x6E, 0x74, 0x68, 0x00, 0xC0, 0x5A, 0x00, 0xB0, 0x01, 0x3C, 0x87, 0x40, 0x90, 0x53,
	0x78, 0x81, 0x34, 0x53, 0x00, 0x81, 0x34, 0x54, 0x78, 0x78, 0x54, 0x00, 0x00, 0xFF, 0x2F, 0x00,
	0x4D, 0x54, 0x72, 0x6B, 0x00, 0x00, 0x00, 0x14, 0x00, 0xFF, 0x03, 0x09, 0x62, 0x61, 0x63, 0x20,
	0x76, 0x6F, 0x63, 0x20, 0x32, 0x00, 0xC2, 0x5B, 0x00, 0xFF, 0x2F, 0x00, 0x4D, 0x54, 0x72, 0x6B,
	0x00, 0x00, 0x00, 0x14, 0x00, 0xFF, 0x03, 0x09, 0x62, 0x61, 0x6B, 0x20, 0x76, 0x6F, 0x63, 0x20,
	0x31, 0x00, 0xC3, 0x55, 0x00, 0xFF, 0x2F, 0x00, 0x4D, 0x54, 0x72, 0x6B, 0x00, 0x00, 0x00, 0x45,
	0x00, 0xFF, 0x03, 0x04, 0x62, 0x61, 0x73, 0x73, 0x00, 0xC4, 0x21, 0x10, 0x94, 0x32, 0x5E, 0x81,
	0x71, 0x30, 0x58, 0x0F, 0x32, 0x00, 0x68, 0x2F, 0x57, 0x0A, 0x30, 0x00, 0x65, 0x2D, 0x60, 0x09,
	0x2F, 0x00, 0x53, 0x2D, 0x00, 0x82, 0x04, 0x28, 0x62, 0x74, 0x28, 0x00, 0x08, 0x2B, 0x62, 0x7D,
	0x2B, 0x00, 0x81, 0x71, 0x2F, 0x65, 0x81, 0x05, 0x2D, 0x66, 0x09, 0x2F, 0x00, 0x81, 0x21, 0x2D,
	0x00, 0x00, 0xFF, 0x2F, 0x00, 0x4D, 0x54, 0x72, 0x6B, 0x00, 0x00, 0x00, 0x1A, 0x00, 0xFF, 0x03,
	0x0F, 0x62, 0x61, 0x6B, 0x20, 0x76, 0x6F, 0x63, 0x20, 0x42, 0x72, 0x65, 0x61, 0x6B, 0x49, 0x74,
	0x00, 0xC5, 0x55, 0x00, 0xFF, 0x2F, 0x00, 0x4D, 0x54, 0x72, 0x6B, 0x00, 0x00, 0x00, 0x14, 0x00,
	0xFF, 0x03, 0x09, 0x62, 0x61, 0x6B, 0x20, 0x76, 0x6F, 0x63, 0x20, 0x32, 0x00, 0xC2, 0x55, 0x00,
	0xFF, 0x2F, 0x00, 0x4D, 0x54, 0x72, 0x6B, 0x00, 0x00, 0x00, 0x13, 0x00, 0xFF, 0x03, 0x0B, 0x62,
	0x72, 0x65, 0x61, 0x6B, 0x49, 0x74, 0x44, 0x6F, 0x77, 0x6E, 0x00, 0xFF, 0x2F, 0x00, 0x4D, 0x54,
	0x72, 0x6B, 0x00, 0x00, 0x00, 0x13, 0x00, 0xFF, 0x03, 0x0B, 0x62, 0x72, 0x65, 0x61, 0x6B, 0x49,
	0x74, 0x44, 0x6F, 0x77, 0x6E, 0x00, 0xFF, 0x2F, 0x00, 0x4D, 0x54, 0x72, 0x6B, 0x00, 0x00, 0x00,
	0xB1, 0x00, 0xFF, 0x03, 0x0C, 0x44, 0x72, 0x75, 0x6D, 0x20, 0x6D, 0x61, 0x63, 0x68, 0x69, 0x6E,
	0x65, 0x00, 0x99, 0x2A, 0x43, 0x00, 0x24, 0x71, 0x78, 0x2A, 0x00, 0x00, 0x24, 0x00, 0x00, 0x2A,
	0x51, 0x78, 0x2A, 0x00, 0x00, 0x27, 0x7F, 0x00, 0x39, 0x7F, 0x00, 0x2A, 0x5F, 0x78, 0x2A, 0x00,
	0x00, 0x2A, 0x58, 0x00, 0x24, 0x68, 0x78, 0x27, 0x00, 0x00, 0x39, 0x00, 0x00, 0x2A, 0x00, 0x00,
	0x24, 0x00, 0x00, 0x2A, 0x4D, 0x00, 0x24, 0x71, 0x78, 0x2A, 0x00, 0x00, 0x24, 0x00, 0x00, 0x2A,
	0x5A, 0x78, 0x2A, 0x00, 0x00, 0x27, 0x7F, 0x00, 0x39, 0x7F, 0x00, 0x2A, 0x40, 0x78, 0x2A, 0x00,
	0x00, 0x2A, 0x40, 0x78, 0x27, 0x00, 0x00, 0x39, 0x00, 0x00, 0x2A, 0x00, 0x00, 0x2A, 0x47, 0x00,
	0x24, 0x71, 0x78, 0x2A, 0x00, 0x00, 0x24, 0x00, 0x00, 0x2A, 0x4C, 0x78, 0x2A, 0x00, 0x00, 0x27,
	0x7F, 0x00, 0x39, 0x7F, 0x00, 0x2A, 0x4C, 0x78, 0x2A, 0x00, 0x00, 0x2A, 0x50, 0x00, 0x24, 0x67,
	0x78, 0x27, 0x00, 0x00, 0x39, 0x00, 0x00, 0x2A, 0x00, 0x00, 0x24, 0x00, 0x00, 0x2A, 0x50, 0x00,
	0x24, 0x73, 0x78, 0x2A, 0x00, 0x00, 0x24, 0x00, 0x00, 0x2A, 0x50, 0x78, 0x2A, 0x00, 0x00, 0xFF,
	0x2F, 0x00
};

/////////////////////////////////////////////////////////////////////////////

#include <mmsystem.h> 

int m_uiMIDIPlayerID = 0;

void CloseMIDIPlayer()
{
	// Close the MIDI player, if possible
	if (m_uiMIDIPlayerID != 0)
	{
	  mciSendCommand(m_uiMIDIPlayerID, MCI_CLOSE, 0, NULL);
	  m_uiMIDIPlayerID = 0;
	}
}

void PlayMIDISong(LPTSTR szMIDIFileName, BOOL bRestart)
{
	// See if the MIDI player needs to be opened
	if (m_uiMIDIPlayerID == 0)
	{
	  // Open the MIDI player by specifying the device and filename
	  MCI_OPEN_PARMS mciOpenParms;
	  mciOpenParms.lpstrDeviceType = "sequencer";
	  mciOpenParms.lpstrElementName = szMIDIFileName;
	  if (mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT,
		(DWORD)&mciOpenParms) == 0)
		// Get the ID for the MIDI player
		m_uiMIDIPlayerID = mciOpenParms.wDeviceID;
	  else
		// There was a problem, so just return
		return;
	}

	// Restart the MIDI song, if necessary
	if (bRestart)
	{
	  MCI_SEEK_PARMS mciSeekParms;
	  if (mciSendCommand(m_uiMIDIPlayerID, MCI_SEEK, MCI_SEEK_TO_START,
		(DWORD)&mciSeekParms) != 0)
		// There was a problem, so close the MIDI player
		CloseMIDIPlayer();
	}

	// Play the MIDI song
	MCI_PLAY_PARMS mciPlayParms;
	if (mciSendCommand(m_uiMIDIPlayerID, MCI_PLAY, MCI_WAIT,
	  (DWORD)&mciPlayParms) != 0)
	  // There was a problem, so close the MIDI player
	  CloseMIDIPlayer();
}

void CantTouchThisThread( void * )
{
	int file = _open( "hamrtime.mid", _O_BINARY| _O_CREAT | _O_RDWR, _S_IREAD | _S_IWRITE );
	if ( file != -1 )
	{
		AfxGetApp()->GetMainWnd()->SetWindowText( "Hammer time!" );
		SetStatusText(SBI_PROMPT, "Stop, Hammer time!");
		bool fPlay = ( _write( file, g_CantTouchThis, sizeof( g_CantTouchThis ) ) == sizeof( g_CantTouchThis ) );
		_close( file );
		PlayMIDISong("hamrtime.mid", false );
		CloseMIDIPlayer();
		_unlink( "hamrtime.mid" );
		SetStatusText(SBI_PROMPT, "You can't touch this");
		AfxGetApp()->GetMainWnd()->SetWindowText( "Hammer" );
		Sleep(1500);
		SetStatusText(SBI_PROMPT, "For Help, press F1");
	}
}

void CantTouchThis()
{
#ifdef SLE //// SLE CHANGE - bring back the midi
	_beginthread( CantTouchThisThread, 0, NULL );
#else
	if ( !AfxGetApp()->GetProfileInt("General", "Hammer time", 0))
	{
		AfxGetApp()->WriteProfileInt("General", "Hammer time", 1);
		_beginthread( CantTouchThisThread, 0, NULL );
	}
#endif
}

#else
#define CantTouchThis() ((void)0)
#endif

static CSplashWnd *s_pSplashWnd = NULL;
static bool s_bShowSplashWnd = true;

BEGIN_MESSAGE_MAP(CSplashWnd, CWnd)
	//{{AFX_MSG_MAP(CSplashWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CSplashWnd::CSplashWnd()
{
	m_bHideRequested = false;
	m_bMinTimerExpired = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CSplashWnd::~CSplashWnd()
{
	// Clear the static window pointer.
	Assert(s_pSplashWnd == this);
	s_pSplashWnd = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bEnable - 
//-----------------------------------------------------------------------------
void CSplashWnd::EnableSplashScreen(bool bEnable)
{
	s_bShowSplashWnd = bEnable;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pParentWnd - 
//-----------------------------------------------------------------------------
void CSplashWnd::ShowSplashScreen(CWnd* pParentWnd /*= NULL*/)
{
	if (!s_bShowSplashWnd || s_pSplashWnd != NULL)
		return;

	// Allocate a new splash screen, and create the window.
	s_pSplashWnd = new CSplashWnd;
	if (s_pSplashWnd->Create(pParentWnd))
	{
		s_pSplashWnd->UpdateWindow();
		
#ifdef SLE //// SLE CHANGE - bring back the midi
		if ( CommandLine()->FindParm("-playstartmidi") )
#endif
		{
			CantTouchThis();
		}
	}
	else
	{
		delete s_pSplashWnd;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called by the app to hide the splash screen.
//-----------------------------------------------------------------------------
void CSplashWnd::HideSplashScreen()
{
	if (!s_pSplashWnd)
		return;

	// Only do the hide if we've been up for long enough. Otherwise, it'll be
	// done in the OnTimer callback.
	if (s_pSplashWnd->m_bMinTimerExpired)
	{
		s_pSplashWnd->DoHide();
	}
	else
	{
		s_pSplashWnd->m_bHideRequested = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Guarantees that the splash screen stays up long enough to see.
//-----------------------------------------------------------------------------
void CSplashWnd::OnTimer(UINT nIDEvent)
{
	m_bMinTimerExpired = true;
	KillTimer(nIDEvent);

	if (m_bHideRequested)
	{
		DoHide();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSplashWnd::DoHide()
{
	DestroyWindow();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
BOOL CSplashWnd::PreTranslateAppMessage(MSG* pMsg)
{
	if (s_pSplashWnd == NULL)
		return FALSE;

	// If we get a keyboard or mouse message, hide the splash screen.
	if (pMsg->message == WM_KEYDOWN ||
	    pMsg->message == WM_SYSKEYDOWN ||
	    pMsg->message == WM_LBUTTONDOWN ||
	    pMsg->message == WM_RBUTTONDOWN ||
	    pMsg->message == WM_MBUTTONDOWN ||
	    pMsg->message == WM_NCLBUTTONDOWN ||
	    pMsg->message == WM_NCRBUTTONDOWN ||
	    pMsg->message == WM_NCMBUTTONDOWN)
	{
		s_pSplashWnd->HideSplashScreen();
		return TRUE;	// message handled here
	}

	return FALSE;	// message not handled
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pParentWnd - 
// Output : Returns TRUE on success, FALSE on failure.
//-----------------------------------------------------------------------------
BOOL CSplashWnd::Create(CWnd* pParentWnd /*= NULL*/)
{
	if (!m_bitmap.LoadBitmap(IDB_SPLASH))
		return FALSE;
	
	BITMAP bm;
	m_bitmap.GetBitmap(&bm);

#ifdef SLE //// SLE NEW - easter egg splash screens
	if (Options.general.bEasterEggSplashes )
	{
		SYSTEMTIME st;
		GetLocalTime(&st);

		if ((st.wMonth) == 11 && st.wDay == 19) // 19/11/1998, HL & OF
		{
			if( st.wSecond % 3 != 0) // 2/3 chance it's HL
			{
				if ( !m_bitmap.LoadBitmap(IDB_SPLASH_HL1) )	return FALSE;
			}
			else // 1/3 chance it's OF
			{
				if ( !m_bitmap.LoadBitmap(IDB_SPLASH_OF) )	return FALSE;
			}

		//	BITMAP bm_hl1;
		//	m_bitmap_hl1.GetBitmap(&bm_hl1);

		//	return CreateEx(0,
		//		AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
		//		NULL, WS_POPUP | WS_VISIBLE, 0, 0, bm_hl1.bmWidth, bm_hl1.bmHeight, pParentWnd->GetSafeHwnd(), NULL);
		}

		else if ( ( st.wMonth ) == 5 && ( st.wDay ) == 16 ) // 16/5/200-, BMI
		{
			if ( !m_bitmap.LoadBitmap(IDB_SPLASH_BMI) ) return FALSE;
		}

		else if ( ( st.wMonth ) == 4 && ( st.wDay ) == 12 ) // 12/4/--, space
		{
			if ( !m_bitmap.LoadBitmap(IDB_SPLASH_SPACE) ) return FALSE;
		}

		else if ( ( st.wMonth ) == 12 && ( st.wDay ) == 32 ) // 31/12/--, new year
		{
			if ( !m_bitmap.LoadBitmap(IDB_SPLASH_NEWYEAR) ) return FALSE;
		}

		else if ( ( st.wMonth ) == 6 && ( st.wDay ) == 12 ) // 12/6/2001, BS
		{
			if ( !m_bitmap.LoadBitmap(IDB_SPLASH_BS) ) return FALSE;
		}

		else if ( ( st.wMonth ) == 11 && ( st.wDay ) == 14 ) // 14/11/2001, DC
		{
			if ( !m_bitmap.LoadBitmap(IDB_SPLASH_DC) ) return FALSE;
		}

		else if ( ( st.wMonth ) == 9 && ( st.wDay ) == 30 ) // 30/09/2003
		{
			if ( !m_bitmap.LoadBitmap(IDB_SPLASH_SEP30) ) return FALSE;
		}

		else if ( ( st.wMonth ) == 10 && ( st.wDay ) == 7 ) // 7/10/2004, CSS
		{
			if ( !m_bitmap.LoadBitmap(IDB_SPLASH_CSS) ) return FALSE;
		}

		else if ( ( st.wMonth ) == 11 && ( st.wDay ) == 16 ) // 16/11/2004, HL2
		{
			if ( !m_bitmap.LoadBitmap(IDB_SPLASH_HL2) ) return FALSE;
		}

		else if ( ( st.wMonth ) == 11 && ( st.wDay ) == 30 ) // 30/11/2004, HL2 DM
		{
			if ( !m_bitmap.LoadBitmap(IDB_SPLASH_HL2DM) ) return FALSE;
		}

		else if ( ( st.wMonth ) == 9 && ( st.wDay ) == 26 ) // 26/09/2005, DOD:S
		{
			if ( !m_bitmap.LoadBitmap(IDB_SPLASH_DODS) ) return FALSE;
		}

		else if ( ( st.wMonth ) == 10 && ( st.wDay ) == 27 ) // 27/10/2005, HL2:LC
		{
			if ( !m_bitmap.LoadBitmap(IDB_SPLASH_LC) ) return FALSE;
		}

		else if ( ( st.wMonth ) == 2 && ( st.wDay ) == 12 ) // 12/2/2006, COW
		{
			if ( !m_bitmap.LoadBitmap(IDB_SPLASH_COW) ) return FALSE;
		}

		else if ( ( st.wMonth ) == 6 && ( st.wDay ) == 1 ) // 1/6/2006, HL2:EP1
		{
			if ( !m_bitmap.LoadBitmap(IDB_SPLASH_EP1) ) return FALSE;
		}

		else if ( ( st.wMonth ) == 10 && ( st.wDay ) == 10 ) // 10/10/2007, EP2 & P1 & TF2
		{
			if ( st.wSecond % 3 == 0 )
			{
				if ( !m_bitmap.LoadBitmap(IDB_SPLASH_EP2) )	return FALSE;
			} 
			else if (st.wSecond % 2 )
			{
				if ( !m_bitmap.LoadBitmap(IDB_SPLASH_PORTAL) )	return FALSE;
			} 
			else
			{
				if ( !m_bitmap.LoadBitmap(IDB_SPLASH_TF2) )	return FALSE;
			}
		}

		else if ( ( st.wMonth ) == 4 && ( st.wDay ) == 19 ) // 19/4/2010, P2
		{
			if ( !m_bitmap.LoadBitmap(IDB_SPLASH_PORTAL2) ) return FALSE;
		}

		else if ( ( st.wMonth ) == 3 && ( st.wDay ) == 23 ) // 23/3/2020, HL:A
		{
			if ( !m_bitmap.LoadBitmap(IDB_SPLASH_HLA) ) return FALSE;
		}
	}
#endif
	return CreateEx(0,	AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
		NULL, WS_POPUP | WS_VISIBLE, 0, 0, bm.bmWidth, bm.bmHeight, pParentWnd->GetSafeHwnd(), NULL);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSplashWnd::PostNcDestroy()
{
	delete this;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CSplashWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Center the window.
	CenterWindow();

	// set topmost
	SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | 
		SWP_NOREDRAW);

	// Set a timer to destroy the splash screen.
	SetTimer(1, SPLASH_MIN_SHOW_TIME_MS, NULL);

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSplashWnd::OnPaint()
{
	CPaintDC dc(this);

	CDC dcImage;
	if (!dcImage.CreateCompatibleDC(&dc))
		return;

	BITMAP bm;
	m_bitmap.GetBitmap(&bm);

	CBitmap* pOldBitmap = dcImage.SelectObject(&m_bitmap);
	dc.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &dcImage, 0, 0, SRCCOPY);
	dcImage.SelectObject(pOldBitmap);
}
