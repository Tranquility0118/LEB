﻿
// Client_SideDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "Client_Side.h"
#include "Client_SideDlg.h"
#include "afxdialogex.h"
#include <string>
#include <fstream>
#include "CImageDlg.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CClientSideDlg::CClientSideDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIENT_SIDE_DIALOG, pParent)
	, m_port(0)
	, m_sendMsg(_T(""))
	, m_pClient(nullptr)
	, m_nick(_T(""))
	, m_imgType(_BMP)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CClientSideDlg::~CClientSideDlg()
{
	closeClient();
}

void CClientSideDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS, m_ip);
	DDX_Text(pDX, IDC_EDIT_PORT, m_port);
	DDV_MinMaxInt(pDX, m_port, 1024, 65535);
	DDX_Control(pDX, IDC_LIST_RECV, m_listRecv);
	DDX_Text(pDX, IDC_EDIT_SEND, m_sendMsg);
	DDX_Text(pDX, IDC_EDIT_NICK, m_nick);
	DDV_MaxChars(pDX, m_nick, 128);
}

BEGIN_MESSAGE_MAP(CClientSideDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CClientSideDlg::OnBnClickedBtnConnect)
	ON_BN_CLICKED(IDC_BTN_DISCONNECT, &CClientSideDlg::OnBnClickedBtnDisconnect)
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CClientSideDlg::OnBnClickedBtnClear)
	ON_BN_CLICKED(IDC_BTN_SEND, &CClientSideDlg::OnBnClickedBtnSend)
	ON_BN_CLICKED(IDC_BTN_FILESEND, &CClientSideDlg::OnBnClickedBtnFilesend)
	ON_MESSAGE(UM_DISCONNECT_SERVER, &CClientSideDlg::OnDisconnectServer)
	ON_MESSAGE(UM_RECV_TEXT, &CClientSideDlg::OnRecevieText)
	ON_MESSAGE(UM_RECV_IMAGE, &CClientSideDlg::OnRecevieImage)
	ON_MESSAGE(UM_RECV_FILE, &CClientSideDlg::OnRecevieFile)
	ON_MESSAGE(UM_RECV_IMAGE_NAME, &CClientSideDlg::OnRecevieImageName)
	ON_MESSAGE(UM_RECV_FILE_NAME, &CClientSideDlg::OnRecevieFileName)
	ON_LBN_DBLCLK(IDC_LIST_RECV, &CClientSideDlg::OnLbnDblclkListRecv)
END_MESSAGE_MAP()


// CClientSideDlg 메시지 처리기

BOOL CClientSideDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// ip init
	std::wstring ip = Client::getMyip();
	m_ip.SetWindowText(ip.c_str());
	m_port = 7000;
	m_nick = _T("익명");

	UpdateData(false);

	GetDlgItem(IDC_BTN_DISCONNECT)->EnableWindow(false);	

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CClientSideDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CClientSideDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CClientSideDlg::OnBnClickedBtnConnect()
{
	if (m_pClient == nullptr)
	{
		UpdateData(true);

		CString ip;
		m_ip.GetWindowTextW(ip);

		if (m_pClient == nullptr)
		{
			m_pClient = new Client(this);
			if (m_pClient->connectServer(ip.operator LPCWSTR(), m_port))
			{
				GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(false);
				GetDlgItem(IDC_BTN_DISCONNECT)->EnableWindow(true);
				GetDlgItem(IDC_EDIT_NICK)->EnableWindow(false);

				m_pClient->sendNick(m_nick.operator LPCWSTR());
			}
			else
			{
				CString str;
				str.Format(_T("Connect Error : %d"), WSAGetLastError());
				MessageBox(str, _T("Error"), MB_OK);
				closeClient();
			}
		}
	}
}


void CClientSideDlg::OnBnClickedBtnDisconnect()
{
	closeClient();
	GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(true);
	GetDlgItem(IDC_BTN_DISCONNECT)->EnableWindow(false);	
	GetDlgItem(IDC_EDIT_NICK)->EnableWindow(true);
}

void CClientSideDlg::closeClient()
{
	if (m_pClient)
	{
		m_pClient->disconnectServer();
		delete m_pClient;
		m_pClient = nullptr;
	}
}


LRESULT CClientSideDlg::OnDisconnectServer(WPARAM wp, LPARAM lp)
{
	OnBnClickedBtnDisconnect();
	return 0;
}

LRESULT CClientSideDlg::OnRecevieText(WPARAM wp, LPARAM lp)
{
	wstring wstr = Client::MultibyteToUnicode(CP_UTF8, (char*)wp, (int)lp);
	CString cstr(wstr.c_str());
	addListMsg(cstr);
	return LRESULT();
}

LRESULT CClientSideDlg::OnRecevieImage(WPARAM wp, LPARAM lp)
{
	const char* pImg = reinterpret_cast<const char*>(wp);	
	size_t img_size = static_cast<size_t>(lp);

	HGLOBAL hg = ::GlobalAlloc(GMEM_MOVEABLE, img_size);
	if (hg != nullptr)
	{
		LPVOID pBuf = ::GlobalLock(hg);
		memcpy(pBuf, pImg, img_size);
		::GlobalUnlock(hg);

		IStream* pStream = NULL;
		if (::CreateStreamOnHGlobal(hg, false, &pStream) == S_OK)
		{
			CImage img;
			HRESULT result;

			if (!img.IsNull())
				img.Destroy();

			result = img.Load(pStream);

			switch (m_imgType)
			{
			case _BMP:
				result = img.Save(m_fileName, Gdiplus::ImageFormatBMP); 
				break;
			case _JPG:
				result = img.Save(m_fileName, Gdiplus::ImageFormatJPEG);
				break;
			case _PNG:
				result = img.Save(m_fileName, Gdiplus::ImageFormatPNG);
				break;
			case _GIF:
				result = img.Save(m_fileName, Gdiplus::ImageFormatGIF);
				break;
			}			

			pStream->Release();
			::GlobalFree(hg);
			return true;
		}
		::GlobalFree(hg);
		return false;
	}
	::GlobalFree(hg);

	/*CImage img2;
	int img_w = 1468;
	int img_h = 733;
	int img_bpp = 24;
	
	img2.Create(img_w, img_h, img_bpp, 0);
	int i = 0;
	for (unsigned int y = 0; y < img_h; y++)
	{
		for (unsigned int x = 0; x < img_w; x++)
		{
			unsigned char r = pImg[i++];
			unsigned char g = pImg[i++];
			unsigned char b = pImg[i++];
			img2.SetPixel(x, y, RGB(r, g, b));
		}
	}

	img2.Save(_T("test.bmp"), Gdiplus::ImageFormatBMP);*/
	return LRESULT();
}

LRESULT CClientSideDlg::OnRecevieFile(WPARAM wp, LPARAM lp)
{
	const char* pFile = reinterpret_cast<const char*>(wp);
	size_t file_size = static_cast<size_t>(lp);

	ofstream file(m_fileName, ios::binary|ios::out);
	file.write(pFile, file_size);
	file.close();

	return LRESULT();
}

LRESULT CClientSideDlg::OnRecevieImageName(WPARAM wp, LPARAM lp)
{
	wstring wstr = Client::MultibyteToUnicode(CP_UTF8, (char*)wp, (int)lp);
	CString cstr(wstr.c_str());
	addListMsg(cstr, _IMAGE_NAME);

	// get file name and ext from recv msg
	getFileNameAndExt(cstr, m_fileName, m_fileExt);
	// img type
	if (m_fileExt == _T("BMP"))
		m_imgType = _BMP;
	else if (m_fileExt == _T("JPG"))
		m_imgType = _JPG;
	else if (m_fileExt == _T("PNG"))
		m_imgType = _PNG;
	else if (m_fileExt == _T("GIF"))
		m_imgType = _GIF;
	return LRESULT();
}

LRESULT CClientSideDlg::OnRecevieFileName(WPARAM wp, LPARAM lp)
{
	wstring wstr = Client::MultibyteToUnicode(CP_UTF8, (char*)wp, (int)lp);
	CString cstr(wstr.c_str());
	addListMsg(cstr, _FILE_NAME);

	// get file name and ext from recv msg
	getFileNameAndExt(cstr, m_fileName, m_fileExt);

	return LRESULT();
}

void CClientSideDlg::addListMsg(const CString & msg, const DATA_TYPE& type)
{
	int index = m_listRecv.AddString(msg);	
	m_listRecv.SetItemData(index, type);
	m_listRecv.SetTopIndex(m_listRecv.GetCount() - 1);
}

void CClientSideDlg::getFileNameAndExt(const CString & str, CString & file_name, CString & file_ext)
{
	// parsing file name
	CString token;
	token.Format(_T("[%s] "), m_nick);
	CString msg(str);
	msg.Replace(token, _T(""));
	file_name = file_ext = msg;

	// parsing file ext
	int index = m_fileExt.ReverseFind('.');
	file_ext = m_fileExt.Mid(++index);
	file_ext = m_fileExt.MakeUpper();
}

void CClientSideDlg::OnBnClickedBtnClear()
{
	m_listRecv.ResetContent();
}


void CClientSideDlg::OnBnClickedBtnSend()
{
	if (m_pClient)
	{
		UpdateData(true);
		if (m_sendMsg.GetLength() > 0)
		{
			m_pClient->sendText(m_sendMsg.operator LPCWSTR());
			m_sendMsg = _T("");
			UpdateData(false);

			GetDlgItem(IDC_EDIT_SEND)->SetFocus();
		}
	}
}

void CClientSideDlg::OnBnClickedBtnFilesend()
{
	if (m_pClient == nullptr)
		return;

	static TCHAR BASED_CODE szFilter[] = _T(" Image Files (*.bmp;*.jpg;*.png) |*.bmp;*.jpg;*.png| All Files(*.*)|*.*||");

	CFileDialog dlg(true, nullptr, nullptr, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilter, this);

	if (dlg.DoModal() == IDOK)
	{
		CString file_name = dlg.GetFileName();
		CString file_path = dlg.GetPathName();
		CString file_ext = dlg.GetFileExt();
		file_ext = file_ext.MakeUpper();

		DATA_TYPE type=_UNKNOWN;
		if (file_ext == _T("BMP") || file_ext == _T("JPG") || file_ext == _T("PNG"))
			type = _IMAGE;
		else
			type = _FILE;		

		m_pClient->sendFile(file_name.operator LPCWSTR(), file_path.operator LPCWSTR(), file_ext.operator LPCWSTR(), type);			
	}
}

void CClientSideDlg::OnLbnDblclkListRecv()
{
	int index = m_listRecv.GetCurSel();
	DATA_TYPE type = static_cast<DATA_TYPE>(m_listRecv.GetItemData(index));

	CString str;
	m_listRecv.GetText(index, str);

	switch (type)
	{
	case _IMAGE_NAME:
	{
		CString file_name, file_ext;
		getFileNameAndExt(str, file_name, file_ext);
		CImageDlg dlg(file_name);
		dlg.DoModal();
	}
	break;
	case _FILE_NAME:
	{
		CString file_name, file_ext;
		getFileNameAndExt(str, file_name, file_ext);
		::ShellExecute(NULL, _T("open"), file_name, NULL, NULL, SW_SHOW);
	}
	break;
	default:
		return;
	}	
}