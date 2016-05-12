// Video_Synopsis_Win.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Video_Synopsis_Win.h"
#include "synopsis.h"

#include <cv.h>
#include <highgui.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <opencv2/video/background_segm.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace cv;
using namespace std;


int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;
	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: code your application's behavior here.
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	char file_path[] = "E:\\video_synopsis_data\\192.168.1.96_31¶°Ç°Çò»ú_20150804081124_20150804084125.mp4";
	const char file_out_path[] = "E:\\video_synopsis_out\\20150804081124_20150804084125.avi";
	int fps, frame_number;

	tube_database s_database = buildTrackDB_KNN(file_path);
	refineDB(s_database);
	return 0;
}
