#include "stdafx.h"
#include <afxwin.h>
#include "DaemonClass.h"
#include <windows.h>  
#include <tchar.h>  
#include <stdio.h>  
#include "PSAPI.H"  
#include <tlhelp32.h>
#include <Userenv.h>
#pragma comment( lib, "PSAPI.LIB" )  
#pragma comment( lib, "Userenv.lib")


CDaemonClass::CDaemonClass(const char* szServiceName)
	: CNTService(szServiceName)
{
	m_bRun = TRUE;
	m_bPause = FALSE;
	m_DaemonThread = NULL;

	m_nDaemonSpan = 1000;
}


CDaemonClass::~CDaemonClass(void)
{
}



BOOL CDaemonClass::OnInit()
{
	Prompt();

	char config[1024] = {0};
	char  filePath[MAX_PATH] = {0};
	GetModuleFileName(GetModuleHandle(NULL),filePath,MAX_PATH);

	string sTempPath(filePath);
	int nPos = sTempPath.rfind('\\');
	string sConfigFilePath = sTempPath.substr(0,nPos);
	sConfigFilePath += "\\Config.ini"; 

	GetPrivateProfileString("DAEMON","PROCESS","",config,1024,sConfigFilePath.c_str());
	string sConfigValue(config);
	SplitString(sConfigValue,'#',m_vecDaemonProcess);	

	m_nDaemonSpan = GetPrivateProfileInt("DAEMON","SPAN",5000,sConfigFilePath.c_str());
	return TRUE;
}


void CDaemonClass::Run()
{
	
	m_DaemonThread = CreateThread(NULL,0,CDaemonClass::DaemonTaskProc,this,NULL,NULL);
	WaitForSingleObject(m_DaemonThread,INFINITE);
}


void CDaemonClass::OnStop()
{
	m_bRun = FALSE;
}


void CDaemonClass::OnPause()
{
	m_bPause = TRUE;
}


void CDaemonClass::OnContinue()
{
	m_bPause = FALSE;
}



void CDaemonClass::SplitString(const string &sIn,char splitChar,vector<string> &resultArr)
{
	if (sIn.empty())
	{
		return ;
	}
	resultArr.clear();
	string sTemp = sIn;
	string sLeft = "";

	int iPos = 0;
	while (iPos != -1)
	{
		iPos = sTemp.find(splitChar);
		if (iPos == string::npos)
		{
			resultArr.push_back(sTemp);
			break;
		}

		sLeft = sTemp.substr(0,iPos);
		resultArr.push_back(sLeft);
		sTemp = sTemp.substr(iPos+1,string::npos);
	}
}


DWORD WINAPI CDaemonClass::DaemonTaskProc(LPVOID lpParameter)
{
	CDaemonClass *pDaemon = (CDaemonClass*)lpParameter;

	HINSTANCE hkernel32; 
	hkernel32=GetModuleHandle("kernel32.dll");

	if (NULL == hkernel32)
	{
		return -1;
	}
	DWORD dProcessID = -1;
	while (pDaemon->m_bRun)
	{
		Sleep(pDaemon->m_nDaemonSpan);
		if (pDaemon->m_bPause)
		{
			continue;
		}

		vector<string>::iterator procVec =  pDaemon->m_vecDaemonProcess.begin();

		for (;procVec != pDaemon->m_vecDaemonProcess.end();procVec++)
		{
			string processPath = *procVec;
			dProcessID = GetProcessPID(processPath);
			if (-1 == dProcessID)
			{
				EWinExec wWinExec = (EWinExec)::GetProcAddress(hkernel32,"WinExec"); 
				//UINT uiRes = wWinExec(processPath.c_str(),SW_SHOW);

				//ShellExecute(NULL,"open",processPath.c_str(),NULL,NULL,SW_SHOWNORMAL);
				//ExeAsUser(processPath);
				ExeAsUserEx(processPath);
				int iErr = GetLastError();
				//uiRes = 0;
			}
		}


	}
	return 0;
}


DWORD CDaemonClass::GetProcessPID(const string &processPath)  
{  
	if (processPath.empty())
	{
		return -1;
	}
	string processName = "";
	int nPos = processPath.rfind('\\');
	processName = processPath.substr(nPos+1,string::npos);

	const char *processname = processName.c_str();

	char temp[1024];
	DWORD    lpidprocesses[1024],cbneeded,cprocesses;  
	HANDLE   hprocess;  
	HMODULE  hmodule;  
	UINT     i;  
	TCHAR    normalname[MAX_PATH]=("UnknownProcess");  

	PROCESSENTRY32 pe32;
	pe32.dwSize=sizeof(pe32);
	HANDLE hProcessSnap=::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);//获得进程列表的快照，第一个參数能够有其它选项，具体请參考MSDN
	if(hProcessSnap==INVALID_HANDLE_VALUE)
	{
		::MessageBox(NULL,"CreateToolhelp32Snapshot error","error",MB_OK);
		return -1;
	}
	HANDLE hProcess;
	BOOL bMore=::Process32First(hProcessSnap,&pe32);//获得第一个进程的信息
	while(bMore)
	{
		::wsprintf(temp,"%s",pe32.szExeFile);
		if(!::strcmp(temp,processname))
		{
			hProcess=::OpenProcess(PROCESS_ALL_ACCESS,false,(DWORD)pe32.th32ProcessID);

			if(hProcess != NULL)  
			{  
				char strFilePath[MAX_PATH] = {0} ; 
				char strProcessName[MAX_PATH] = {0} ; 
				GetModuleFileNameExA(hProcess, NULL, strFilePath, MAX_PATH);  
				//获取进程名 结果和上面没区别  
				//GetModuleFileNameEx(hProcess,0, strProcessName,MAX_PATH);
				if(0 == ::strcmp(temp,processname))
				{
					return pe32.th32ProcessID ;
				}
			}  
			else
			{
				break;
			}
		}
		bMore=::Process32Next(hProcessSnap,&pe32);//获得其它进程信息
	}
	return -1;  
}  

BOOL CDaemonClass::Prompt()
{
	HANDLE hToken;
	if(!OpenProcessToken(::GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken))
	{
		return FALSE;
	}
	TOKEN_PRIVILEGES tkp;
	tkp.PrivilegeCount =1;
	if(!LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&tkp.Privileges [0].Luid ))
	{
		return FALSE;
	}
	if(!AdjustTokenPrivileges(hToken,false,&tkp,sizeof(tkp),NULL,0))
	{
		return FALSE;
	}
	return true;
}


void CDaemonClass::ExeAsUser(string sExePath)
{
	HANDLE hTokenThis = NULL;  
	HANDLE hTokenDup = NULL;  
	HANDLE hThisProcess = GetCurrentProcess();  
	OpenProcessToken(hThisProcess, TOKEN_ALL_ACCESS, &hTokenThis);  
	DuplicateTokenEx(hTokenThis, MAXIMUM_ALLOWED,NULL, SecurityIdentification, TokenPrimary, &hTokenDup); 
	DWORD dwSessionId = WTSGetActiveConsoleSessionId();  
	SetTokenInformation(hTokenDup, TokenSessionId, &dwSessionId, sizeof(DWORD));  

	STARTUPINFO si;  
	PROCESS_INFORMATION pi;  
	ZeroMemory(&si, sizeof(STARTUPINFO));  
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));  
	si.cb = sizeof(STARTUPINFO);  
	//si.lpDesktop = "WinSta0//Default";  

	LPVOID pEnv = NULL;  
	DWORD dwCreationFlag = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;  

	CreateEnvironmentBlock(&pEnv, hTokenDup, FALSE);  

	BOOL bResult = CreateProcessAsUser(  
		hTokenDup,  
		NULL,  
		//(char *)sExePath.c_str(),  
		 (char *)"notepad",  
		NULL,  
		NULL,  
		FALSE,  
		NULL,  
		NULL,  
		NULL,  
		&si,  
		&pi);
	 int iErr = GetLastError();
}


void CDaemonClass::ExeAsUserEx(string sExePath)
{
	HANDLE hToken = NULL;  
	//创建进程快照  
	PROCESSENTRY32 pe32 = { 0 };  
	pe32.dwSize = sizeof(pe32);   
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);  
	if (hSnapShot!=0 && hSnapShot!=INVALID_HANDLE_VALUE)  
	{  
		BOOL bRet = Process32First(hSnapShot,&pe32);  
		while(bRet)  
		{  
			if (_tcsicmp(pe32.szExeFile,"Explorer.EXE") == 0)  
			{  
				HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS,false,pe32.th32ProcessID);  

				if (hProcess!=NULL)  
				{  
					BOOL flag = OpenProcessToken(hProcess,TOKEN_ALL_ACCESS,&hToken);  
					CloseHandle(hProcess);  
				}  
				break;  
			}  
			bRet = Process32Next(hSnapShot,&pe32);  
		}  
		CloseHandle(hSnapShot);  
	}  

	STARTUPINFO si ={sizeof(si)};  
	PROCESS_INFORMATION pi;
	//TCHAR FileName[256] 外部EXE的完整路径
	//BOOL bSuccess = CreateProcessAsUser(hToken,sExePath.c_str(),NULL,NULL,NULL,FALSE,NULL,NULL,NULL,&si,&pi);
	BOOL bSuccess = CreateProcessAsUser(hToken,sExePath.c_str(),NULL,NULL,NULL,FALSE,CREATE_NEW_CONSOLE |NORMAL_PRIORITY_CLASS|CREATE_UNICODE_ENVIRONMENT,NULL,NULL,&si,&pi);
	
}
