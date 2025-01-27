﻿
// NetworkDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "Network.h"
#include "NetworkDlg.h"
#include "afxdialogex.h"

#include "Server.h"
#include <string>
#include <ws2tcpip.h>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CNetworkDlg 대화 상자


CNetworkDlg::CNetworkDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_NETWORK_DIALOG, pParent), m_pServer(nullptr)
	, m_port(0)
	, m_sendMsg(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CNetworkDlg::~CNetworkDlg()
{
	closeServer();
}

void CNetworkDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS, m_ip);
	DDX_Text(pDX, IDC_EDIT_PORT, m_port);
	DDV_MinMaxInt(pDX, m_port, 1024, 65535);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Control(pDX, IDC_LIST_MSG, m_listMsg);
	DDX_Text(pDX, IDC_EDIT_MSG, m_sendMsg);
}

BEGIN_MESSAGE_MAP(CNetworkDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_START, &CNetworkDlg::OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_BTN_STOP, &CNetworkDlg::OnBnClickedBtnStop)
	ON_BN_CLICKED(IDC_BTN_SEND, &CNetworkDlg::OnBnClickedBtnSend)
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CNetworkDlg::OnBnClickedBtnClear)
	ON_MESSAGE(UM_CONNECT_CLIENT, &CNetworkDlg::OnConnectClient)
	ON_MESSAGE(UM_DISCONNECT_CLIENT, &CNetworkDlg::OnDisconnectClient)
	ON_MESSAGE(UM_RECV_NICK, &CNetworkDlg::OnRecevieNick)
	ON_MESSAGE(UM_RECV_TEXT, &CNetworkDlg::OnRecevieText)

END_MESSAGE_MAP()


// CNetworkDlg 메시지 처리기

BOOL CNetworkDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.
	
	// ip init
	std::wstring ip = Server::getMyip();	
	m_ip.SetWindowText(ip.c_str());	
	m_port = 7000;
	UpdateData(false);
	
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(false);

	initListClient();

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CNetworkDlg::OnPaint()
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
HCURSOR CNetworkDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CNetworkDlg::OnBnClickedBtnStart()
{
	if (m_pServer == nullptr)
	{
		UpdateData(true);

		CString ip;
		m_ip.GetWindowTextW(ip);		

		m_pServer = new Server(this);
		m_pServer->startServer(ip.operator LPCWSTR(), m_port);

		GetDlgItem(IDC_BTN_START)->EnableWindow(false);
		GetDlgItem(IDC_BTN_STOP)->EnableWindow(true);
	}
}


void CNetworkDlg::OnBnClickedBtnStop()
{
	closeServer();
	GetDlgItem(IDC_BTN_START)->EnableWindow(true);
	GetDlgItem(IDC_BTN_STOP)->EnableWindow(false);
	m_list.DeleteAllItems();
}

void CNetworkDlg::closeServer()
{
	if (m_pServer)
	{
		m_pServer->endServer();
		delete m_pServer;
		m_pServer = nullptr;		
	}	
}

void CNetworkDlg::initListClient()
{
	CRect rect;
	m_list.GetClientRect(&rect);
	m_list.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);	
	m_list.InsertColumn(0, _T("닉네임"), LVCFMT_LEFT, int((double)rect.Width()*0.2));
	m_list.InsertColumn(1, _T("IP"), LVCFMT_LEFT, int((double)rect.Width()*0.2));
	m_list.InsertColumn(2, _T("PORT"), LVCFMT_LEFT, int((double)rect.Width()*0.2));	
	m_list.InsertColumn(3, _T("접속 시점"), LVCFMT_LEFT, int((double)rect.Width()*0.3));
}

void CNetworkDlg::addListClient(const CString& ip, const CString& port)
{
	int cnt = m_list.GetItemCount();	

	m_list.InsertItem(cnt, __T(""));		

	m_list.SetItem(cnt, 1, LVIF_TEXT, ip, NULL, NULL, NULL, NULL);
	m_list.SetItem(cnt, 2, LVIF_TEXT, port, NULL, NULL, NULL, NULL);

	CTime time = CTime::GetCurrentTime();
	CString strtime = time.Format(_T("%Y-%m-%d %H:%M:%S"));
	m_list.SetItem(cnt, 3, LVIF_TEXT, strtime, NULL, NULL, NULL, NULL);
}

void CNetworkDlg::addListNick(const CString& nick)
{
	int cnt = m_list.GetItemCount();
	m_list.SetItem(cnt-1, 0, LVIF_TEXT, nick, NULL, NULL, NULL, NULL);

}

void CNetworkDlg::addListText(const CString & msg)
{
	m_listMsg.AddString(msg);
}

LRESULT CNetworkDlg::OnConnectClient(WPARAM wp, LPARAM lp)
{
	wchar_t buf[46];
	DWORD buf_size = sizeof(buf);
	memset(&buf, 0, buf_size);
	InetNtop(AF_INET, &wp, buf, buf_size);
	
	CString ip = buf;
	CString port;
	port.Format(_T("%d"), lp);

	addListClient(ip, port);	
	return 0;
}

LRESULT CNetworkDlg::OnDisconnectClient(WPARAM wp, LPARAM lp)
{
	wchar_t buf[46];
	DWORD buf_size = sizeof(buf);
	memset(&buf, 0, buf_size);
	InetNtop(AF_INET, &wp, buf, buf_size);

	CString ip = buf;
	CString port;
	port.Format(_T("%d"), lp);

	// delete list control (ip and port)
	for (int row = 0; row < m_list.GetItemCount(); row++)
	{
		CString cip = m_list.GetItemText(row, 1);
		CString cport = m_list.GetItemText(row, 2);

		if (ip == cip && port == cport)
			m_list.DeleteItem(row);
	}
	return 0;
}

LRESULT CNetworkDlg::OnRecevieNick(WPARAM wp, LPARAM lp)
{
	wstring wstr = Server::MultibyteToUnicode(CP_UTF8, (char*)wp, (int)lp);
	CString cstr(wstr.c_str());
	addListNick(cstr);
	return LRESULT();
}

LRESULT CNetworkDlg::OnRecevieText(WPARAM wp, LPARAM lp)
{
	wstring wstr = Server::MultibyteToUnicode(CP_UTF8, (char*)wp, (int)lp);
	CString cstr(wstr.c_str());
	addListText(cstr);
	return LRESULT();
}


void CNetworkDlg::OnBnClickedBtnSend()
{
	if (m_pServer)
	{
		UpdateData(true);
		if (m_sendMsg.GetLength() > 0)
		{
			m_pServer->sendText(m_sendMsg.operator LPCWSTR(), _TEXT);
			m_sendMsg = _T("");
			UpdateData(false);
		}
	}
}


void CNetworkDlg::OnBnClickedBtnClear()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
