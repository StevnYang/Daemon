#pragma once
#include "ntservice.h"
#include <string>
#include <vector>
using namespace std;

typedef UINT (WINAPI *EWinExec)(LPCSTR, UINT);  

class CDaemonClass :
	public CNTService
{
public:
	CDaemonClass(const char* szServiceName);
	~CDaemonClass(void);


	virtual BOOL OnInit();
	virtual void Run();
	virtual void OnStop();
	virtual void OnPause();
	virtual void OnContinue();

private:
	//string m_sDaemonProcess[20];
	vector<string> m_vecDaemonProcess;

	HANDLE m_DaemonThread;

	BOOL m_bRun;
	BOOL m_bPause;

	int m_nDaemonSpan;

		


private:

	static void ExeAsUser(string sExePath);
	static void ExeAsUserEx(string sExePath);
	static void SplitString(const string &sIn,char splitChar,vector<string> &resultArr);
	static DWORD WINAPI DaemonTaskProc(LPVOID lpParameter);
	static BOOL Prompt();
	static DWORD GetProcessPID(const string &processPath);
};


