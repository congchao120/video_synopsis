// VideoSyn.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "synopsis.h"
#include <cv.h>
#include <highgui.h>
int _tmain(int argc, _TCHAR* argv[])
{

	const char file_path[] = "E:\\video_synopsis_data\\192.168.1.96_31_20150804074126_20150804081124.mp4";
	const char file_out_path[] = "E:\\video_synopsis_out\\192.168.1.96_31_20150804074126_20150804081124.avi";

	int fps, frame_number;
	CvSize size;
	IplImage* bgImage = NULL;
	const int frame_num_used = 500;
	list< list<tube *> > database;
	int code = (int)CV_FOURCC('d', 'i', 'v', '3');
	bgModeling(file_path, file_out_path, frame_num_used, &bgImage, fps, frame_number, size, code);
	
	code = (int)CV_FOURCC('x', 'v', 'i', 'd');

	cvShowImage("bg", bgImage);
 	cvWaitKey();
	buildTrackDB(file_path, bgImage, database);
	mergeDB(database, file_out_path, fps, size, bgImage);
	return 0;
}

