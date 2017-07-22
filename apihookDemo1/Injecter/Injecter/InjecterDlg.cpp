
// InjecterDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Injecter.h"
#include "InjecterDlg.h"
#include "afxdialogex.h"
#include "tlhelp32.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CInjecterDlg 对话框



CInjecterDlg::CInjecterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_INJECTER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CInjecterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CInjecterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPEN, &CInjecterDlg::OnBnClickedOpen)
	ON_BN_CLICKED(IDC_INJECT, &CInjecterDlg::OnBnClickedInject)
END_MESSAGE_MAP()


// CInjecterDlg 消息处理程序

BOOL CInjecterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CInjecterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CInjecterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CInjecterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CInjecterDlg::OnBnClickedOpen()
{
	CFileDialog fileDialog(TRUE, 0, 0, 6UL, _T("DLL File| *.dll|"));
	if (fileDialog.DoModal() == IDOK) {
		CString Dllpath;
		Dllpath = fileDialog.GetPathName();
		SetDlgItemText(IDC_DLLPATH, Dllpath);
	}
}

DWORD ProcessFind(LPCTSTR Exename) {
	HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (!hProcess) {
		return FALSE;
	}
	
	PROCESSENTRY32 info;

	info.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcess, &info)) {
		return FALSE;
	}

	while (true)
	{
		if (_tcscmp(info.szExeFile, Exename) == 0) {
			return info.th32ProcessID;
		}
		
		if (!Process32Next(hProcess, &info)) {
			return FALSE;
		}
	}
	return FALSE;
}

BOOL Inject(LPCTSTR Dllpath, DWORD ProcessID) {
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, ProcessID);

	if (!hProcess) {
		return FALSE;
	}

	SIZE_T PathSize = (_tcslen(Dllpath) + 1) * sizeof(TCHAR);

	LPVOID StartAddress = VirtualAllocEx(hProcess, NULL, PathSize, MEM_COMMIT, PAGE_READWRITE);

	if (!StartAddress) {
		return FALSE;
	}

	if (!WriteProcessMemory(hProcess, StartAddress, Dllpath, PathSize, NULL)) {
		return FALSE;
	}

	PTHREAD_START_ROUTINE pfnStartAddress = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "LoadLibraryW");

	if (!pfnStartAddress) {
		return FALSE;
	}

	//xp中使用CreateRemoteThread
	HANDLE hThread = CreateRemoteThreadEx(hProcess, NULL, NULL, pfnStartAddress, StartAddress, NULL, NULL, NULL);

	if (!hThread) {
		return FALSE;
	}

	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);

	CloseHandle(hProcess);

	return TRUE;
}

void CInjecterDlg::OnBnClickedInject()
{
	CString Dllpath;
	CString Exename;
	GetDlgItemText(IDC_EXENAME, Exename);
	GetDlgItemText(IDC_DLLPATH, Dllpath);

	if (Exename.GetLength() == 0) {
		MessageBox(_T("Please input exe name !"));
		return;
	}

	DWORD ProcessID = ProcessFind(Exename);

	if (!ProcessID) {
		MessageBox(_T("Can't find the process !"));
		return;
	}

	BOOL IsInjected = Inject(Dllpath, ProcessID);

	if (IsInjected) {
		MessageBox(_T("Inject Success !"));
	}
	else {
		MessageBox(_T("Inject Failed !")); 
	}
}
