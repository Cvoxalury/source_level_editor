#include "stdafx.h"
#include "ButtonContext.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CButtonContext::CButtonContext()
{
}

CButtonContext::~CButtonContext()
{
}

BEGIN_MESSAGE_MAP(CButtonContext, CButton)
	//{{AFX_MSG_MAP(CButtonContext)
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CButtonContext::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	NMHDR hdr;
	hdr.code = NM_RCLICK;
	hdr.hwndFrom = this->GetSafeHwnd();
	hdr.idFrom = GetDlgCtrlID();
	TRACE("OnRButtonUp");
	AfxMessageBox("Pressed button context", MB_OK);
	this->GetParent()->SendMessage(WM_NOTIFY, ( WPARAM )hdr.idFrom, ( LPARAM )&hdr);
}
