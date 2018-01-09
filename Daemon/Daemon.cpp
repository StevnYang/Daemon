// Daemon.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "DaemonClass.h"


int _tmain(int argc, _TCHAR* argv[])
{
#ifdef T_DEBUG

	CDaemonClass NTSInstance("FSSL Daemon");
	NTSInstance.OnInit();
	NTSInstance.Run();

#else

	//ShellExecute(NULL,"open","E:\\S_FSSL_SVN\\ImageCenter\\trunk\\医创平台\\DcmUploaderTool\\UploaderTool - Login\\Bin\\Release\\UploadTool.exe",NULL,NULL,SW_HIDE);
	//return 0;
	CDaemonClass NTSInstance("FSSLDaemon");
	if (!NTSInstance.ParseStandardArgs(argc, argv))
	{
		NTSInstance.StartService();
	}


#endif
	return 0;
}


