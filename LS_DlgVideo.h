#pragma once


// CLS_DlgVideo dialog

class CLS_DlgVideo : public CDialog
{
	DECLARE_DYNAMIC(CLS_DlgVideo)

public:
	CLS_DlgVideo(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLS_DlgVideo();

// Dialog Data
	enum { IDD = IDD_LS_DLGVIDEO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};
