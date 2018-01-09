// Daemon.cpp : �������̨Ӧ�ó������ڵ㡣
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

	//ShellExecute(NULL,"open","E:\\S_FSSL_SVN\\ImageCenter\\trunk\\ҽ��ƽ̨\\DcmUploaderTool\\UploaderTool - Login\\Bin\\Release\\UploadTool.exe",NULL,NULL,SW_HIDE);
	//return 0;
	CDaemonClass NTSInstance("FSSLDaemon");
	if (!NTSInstance.ParseStandardArgs(argc, argv))
	{
		NTSInstance.StartService();
	}


#endif
	return 0;
}


