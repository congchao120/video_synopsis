// LS_DlgVideo.cpp : implementation file
//

#include "stdafx.h"
#include "Video_Synopsis_Win.h"
#include "LS_DlgVideo.h"
#include "afxdialogex.h"


// CLS_DlgVideo dialog

IMPLEMENT_DYNAMIC(CLS_DlgVideo, CDialog)

CLS_DlgVideo::CLS_DlgVideo(CWnd* pParent /*=NULL*/)
	: CDialog(CLS_DlgVideo::IDD, pParent)
{

}

CLS_DlgVideo::~CLS_DlgVideo()
{
}

void CLS_DlgVideo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLS_DlgVideo, CDialog)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CLS_DlgVideo message handlers


void CLS_DlgVideo::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialog::OnPaint() for painting messages

	RECT rect = {0};	
	this->GetClientRect(&rect);
	int m_iWidth = (rect.right - rect.left);
	int m_iHeight = (rect.bottom - rect.top);
	CBitmap bitmap;
	bitmap.LoadBitmap(IDB_NOVIDEO);
	CDC *pDC = GetDC();
	CDC memDC;//¼æÈÝDC
	memDC.CreateCompatibleDC(pDC);
	memDC.SelectObject(&bitmap);
	BITMAP bmp;
	bitmap.GetBitmap(&bmp);
	pDC->StretchBlt(0, 0, m_iWidth, m_iHeight, &memDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
	memDC.DeleteDC();
	::DeleteObject(&bitmap);
}
