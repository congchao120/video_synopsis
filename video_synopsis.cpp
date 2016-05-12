
#include "stdafx.h"
#include "synopsis.h"
#include "CTracker.h"
#include "resource.h"
#include <io.h>
#include <cv.h>
#include <highgui.h>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "LS_DlgVideo.h"
#include "TD_PlaySDK.h"

static unsigned char* buf;				//存放临时内存buf
static CLS_DlgVideo *m_pDlgVideo;		//显示视频的PNL
static CLS_PlaySDK   m_PlaySDK;
static SDK_VERSION    m_Version;
static CLS_DlgVideo *m_pDlg;		//显示视频的PNL
static int m_iBegin;
static int m_iEnd;   //获得第一帧和最后一帧的帧序号 
static int m_iWidth;
static int m_iHeight;
static int m_iFramerate;
static int m_iFrame_ind;//当前frame index
static int m_Thres_ObjNum;
static int m_Thres_MinArea;
static int m_Thres_SkipBeginFrame;
static tube_database m_database;
static CTracker m_tracker(0.1,0.2, m_database.tubes, 30.0,10,100);
static Ptr<BackgroundSubtractorKNN> m_KNN = createBackgroundSubtractorKNN(200, 200, false);
static Scalar Colors[]={Scalar(255,0,0),Scalar(0,255,0),Scalar(0,0,255),Scalar(255,255,0),Scalar(0,255,255),Scalar(255,0,255),Scalar(255,127,255),Scalar(127,0,255),Scalar(127,0,127)};


/**
*基本数据单元构造函数
*/
simple_tube::simple_tube(CvRect rect, int f) :position(rect), t_f(f) {
}


/**
*基本数据单元构造函数
*/
simple_tube::~simple_tube() {
	;
}
/**
*基本数据单元构造函数
*/
tube_seq::tube_seq() 
	:i_start(0),i_end(0), f_relate(false), i_index(0){
}
/**
*基本数据单元构造函数
*/
tube_seq::tube_seq(list<simple_tube> _seq, int _iframe, int _index) 
	:tubes(_seq),i_start(_iframe),i_end(_iframe), f_relate(false), i_index(_index){
}

/**
*基本数据单元析构函数
*/
tube_seq::~tube_seq() {
}


/**
*基本数据单元构造函数
*/
tube_database::tube_database() 
	:i_width(0),i_height(0){
}

/**
*基本数据单元析构函数
*/
tube_database::~tube_database() {
}

/**
*判断两矩形是否重叠
*/
bool isOverlap(const CvRect & a, const CvRect & b) {
	const CvRect * l_rect = &a,
		*r_rect = &b;
	if (a.x > b.x) {
		const CvRect * tmp = l_rect;
		l_rect = r_rect;
		r_rect = tmp;
	}

	if (l_rect->width < r_rect->x - l_rect->x)
		return false;
	else if (l_rect->y <= r_rect->y && l_rect->height >= r_rect->y - l_rect->y)
		return true;
	else if (l_rect->y > r_rect->y && r_rect->height >= l_rect->y - r_rect->y)
		return true;
	else
		return false;
}

/**
*判断两矩形是否相邻，带相邻参数
*/
bool isOverlap(CvRect a, CvRect b, int thres)
{
	a.x -= thres;
	if(a.x <= 0)
		a.x = 0;
	a.y -= thres;
	if(a.y <= 0)
		a.y = 0;
	a.width += 2*thres;
	a.height += 2*thres;

	return isOverlap(a, b);

}

/**
*前景位置矩形块合并 - 减少单元数
*/
void mergeRects(list<CvRect> & rects) {
	int x = 0, y = 0, width = 0, height = 0;//临时变量
	for (list<CvRect>::iterator i = rects.begin(); i != rects.end(); ) {
		bool merged = false;//多引入一个变量判断i是否被merge非常有用！
		list<CvRect>::iterator j = i;
		for (j++; j != rects.end(); j++) {
			if (isOverlap(*i, *j)) {
				if (i->x < j->x) {
					x = i->x;
					width = max(j->x - i->x + j->width, i->width);
				}
				else {
					x = j->x;
					width = max(i->x - j->x + i->width, j->width);
				}

				if (i->y < j->y) {
					y = i->y;
					height = max(j->y - i->y + j->height, i->height);
				}
				else {
					y = j->y;
					height = max(i->y - j->y + i->height, j->height);
				}

				//合并
				j->x = x;
				j->y = y;
				j->width = width;
				j->height = height;

				i = rects.erase(i);//删除被合并项。注意：删除前者（i）更新后者（j）！
				merged = true;
			}
		}
		if (!merged)
			i++;
	}
}


/**
*判断是否属于同一跟踪目标
*/
bool isSameObj(const CvRect & a, const CvRect & b, const float category_factor) {
	const CvRect * l_rect = &a,
		*r_rect = &b;
	if (a.x > b.x) {
		const CvRect * tmp = l_rect;
		l_rect = r_rect;
		r_rect = tmp;
	}

	int area = 0;//记录重叠区域面积
	if (l_rect->width <= r_rect->x - l_rect->x)
		return false;
	else if (l_rect->y <= r_rect->y && l_rect->height > r_rect->y - l_rect->y) {
		area = (l_rect->x + l_rect->width - r_rect->x) * (l_rect->y + l_rect->height - r_rect->y);
		if (area > category_factor * a.width * a.height || area > category_factor * b.width * b.height)
			return true;
		else
			return false;
	}
	else if (l_rect->y >= r_rect->y && r_rect->height > l_rect->y - r_rect->y) {
		area = (l_rect->x + l_rect->width - r_rect->x) * (r_rect->y + r_rect->height - l_rect->y);
		if (area > category_factor * a.width * a.height || area > category_factor * b.width * b.height)
			return true;
		else
			return false;
	}
	else
		return false;
}

/**
*前景跟踪并建立数据库结构 - KNN
*/

tube_database buildTrackDB_KNN(char * videoFilePath, \
	const int min_area, const int obj_num, const float extend_factor, \
	const float category_factor, const bool save_mode, const int skip_begin_frame) {

	m_database.tubes.clear();
	m_iFrame_ind = 0;
	m_Thres_ObjNum = obj_num;
	m_Thres_MinArea = min_area;
	m_Thres_SkipBeginFrame = skip_begin_frame;

	CvCapture * capture = cvCaptureFromFile(videoFilePath);
	if (!capture) {
		printf("Using Video SDK to read the file!\n");
		m_pDlgVideo = new CLS_DlgVideo();
		m_pDlgVideo->Create(IDD_LS_DLGVIDEO);
		m_pDlgVideo->MoveWindow(0, 0, 200, 200);
		m_pDlgVideo->ShowWindow(SW_HIDE);
		//初始化动态库
		m_PlaySDK.LoadDll();
		int iRet = -1;
		if ((iRet = m_PlaySDK.CreateSystem(NULL)) != 0)
		{
			printf("Error: 初始化动态库失败");
			return m_database;
		}

		m_PlaySDK.StartMonitorCPU();
		m_PlaySDK.GetVersion(&m_Version);
		int iBufferSize = 0;                  //创建一个播放实例，以文件方式播放
		int g_iPlayerID = m_PlaySDK.CreatePlayerFromFile(m_pDlgVideo->GetSafeHwnd(), videoFilePath, iBufferSize);

		if(g_iPlayerID < 0)
		{
			_tprintf(_T("Error: Cannot read file!\n"));
			return m_database;
		}

		RECT rcDraw = {0};
		memset(&rcDraw, 0, sizeof(rcDraw));
		m_PlaySDK.SetPlayRect(g_iPlayerID,&rcDraw);//设置播放位置和大小 

		if(m_PlaySDK.GetBeginEnd(g_iPlayerID,&m_iBegin,&m_iEnd) != 0)
		{
			_tprintf(_T("Error: Cannot acquire beginning and ending frame!\n"));
			return m_database;
		}




		if (m_PlaySDK.GetVideoParam(g_iPlayerID, &m_iWidth, &m_iHeight, &m_iFramerate) < 0 )
		{
			_tprintf(_T("Error: Can't capture for no playing!\n"));
			return m_database;
		}

		m_PlaySDK.Play(g_iPlayerID);
		m_PlaySDK.RegisterNotifyGetDecAV(g_iPlayerID, (void*)GetDecAVCbk, true);
		m_database.i_height = m_iHeight;
		m_database.i_width = m_iWidth;
		buf = new unsigned char[m_iWidth*m_iHeight*2];
		while(m_iFrame_ind < m_iEnd)
		{
			;
		}
		return m_database;
	}
	else
	{
		printf("Using OpenCV FFMpeg to read the file!\n");

		m_iFramerate = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
		m_iBegin = 0;
		m_iEnd = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);
		IplImage * frame = NULL;
		m_iWidth = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
		m_iHeight = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
		m_database.i_height = m_iHeight;
		m_database.i_width = m_iWidth;

		printf("Build track database...\n");
		while (frame = cvQueryFrame(capture)) {
			Mat frame_mat;
			int find_num = 0;
			vector<CvRect> rects; //存放找到的前景块位置
			vector<Point2d> centers; //存放找到的前景块中心
			vector<vector<Point> > contours;
			vector<Vec4i> hierarchy;

			frame_mat = cvarrToMat(frame,true);  
			Mat edges;
			detectEdge_KNN(frame, m_KNN, edges);
			imshow("Canny边缘", edges);


			findContours(edges, contours, hierarchy, CV_RETR_EXTERNAL, CHAIN_APPROX_TC89_KCOS, Point());
			find_num = contours.size();															   
			/**!!!Be careful with the param below!!!**/
			if (find_num > (m_Thres_ObjNum * 3) || frame == NULL || m_iFrame_ind <= skip_begin_frame) {
				m_iFrame_ind += 1;
				continue;
			}
			printf("frame:%d\tfind contour num:%d\n", m_iFrame_ind, find_num);

			//contour analysis
			if(contours.size()>0)
			{
				for( int i = 0; i < contours.size(); i++ )
				{
					Rect r = boundingRect(contours[i]);
					rects.push_back(r);
					centers.push_back((r.br()+r.tl())*0.5);
				}
			}

			for(int i=0; i<centers.size(); i++)
			{
				circle(frame_mat,centers[i],3,Scalar(0,255,0),1,CV_AA);
			}


 			if(centers.size()>0)
			{
				m_tracker.Update(centers, rects, m_iFrame_ind, min_area);
				
				cout << "Tracker number: " <<  m_tracker.tracks.size()  << endl;

				for(int i=0;i<m_tracker.tracks.size();i++)
				{
					if(m_tracker.tracks[i]->trace.size()>1)
					{
						char txt[255];
						sprintf(txt, "%d", m_tracker.tracks[i]->n_tube_map_index);
						putText(frame_mat,txt,m_tracker.tracks[i]->trace[m_tracker.tracks[i]->trace.size()-1],CV_FONT_HERSHEY_COMPLEX, 1, Colors[m_tracker.tracks[i]->track_id%9]);

						for(int j=0;j<m_tracker.tracks[i]->trace.size()-1;j++)
						{
							line(frame_mat,m_tracker.tracks[i]->trace[j],m_tracker.tracks[i]->trace[j+1],Colors[m_tracker.tracks[i]->track_id%9],2,CV_AA);
							
						}
					}
				}
			}

			imshow("Kalman", frame_mat);
			cvWaitKey(10);

			m_iFrame_ind += 1;
			rects.clear();
		}
		printf("Track DB has been saved! Total frame num:%d\tValid frame:%d\n", m_iEnd, m_iFrame_ind);
		m_database.i_total = m_database.tubes.size();
		printf("Tube DB has been built! Total tube num:%d\n", m_database.tubes.size());
		return m_database;
		//释放内存
	}

	
}


/**
*  提炼tube数据库，判断独立tube，相关tubes，邻近tubes，顺序tubes，时间重叠tubes
*  首先判断相关与否
*  随后筛选不相关tube，5帧内删除
*  判断邻近，类似相关，25*25*1判断
*  判断顺序
*  判断时间重叠
*/
void refineDB(tube_database& s_database, const int thres_main_duration, \
		const int relate_thres_distance, const int relate_thres_frame, const int thres_min_duration, \
		const int neighbour_thres_distance, const int ts_thres_frame, const float to_thres_overlap) {
	if (s_database.tubes.empty()) {
		printf("Fail: Database is empty, cannot refine!\n");
		return;
	}

	if (s_database.i_width <= 0 || s_database.i_height <= 0) {
		printf("Fail: Information in database is incorrect, cannot refine!\n");
		return;
	}

	FILE* fp = fopen("D:\\tube_database.csv", "wt");  
	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		for (list<simple_tube>::iterator jter = s_database.tubes[i_index].tubes.begin(); jter != s_database.tubes[i_index].tubes.end(); jter++)
		{
			simple_tube ptube = (simple_tube)(*jter);
			fprintf(fp, "%d,%d,%d,%d,%d,%d\n", s_database.tubes[i_index].i_index, ptube.position.x, ptube.position.y, ptube.position.width, ptube.position.height, ptube.t_f);   

		}
	}
	fclose(fp);

	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		printf("refine tubes: %d\t\n", i_index);
		if(s_database.tubes[i_index].tubes.size() <= thres_main_duration)
		{
			continue;
		}
		else
		{
			//判断RT与NT
			for(int j_index = i_index; j_index < s_database.i_total; j_index++)
			{
				if(s_database.tubes[j_index].i_start < s_database.tubes[i_index].i_start)
					continue;
				if(s_database.tubes[j_index].i_start -  s_database.tubes[j_index].i_end > relate_thres_frame)
					break;

				bool f_relate = false;
				bool f_neighbour = false;

				for (list<simple_tube>::iterator iiter = s_database.tubes[i_index].tubes.begin(); iiter != s_database.tubes[i_index].tubes.end(); iiter++)
				{
					if(f_relate || f_neighbour)
						break;
					simple_tube ptube_i = (simple_tube)(*iiter);
					CvRect rect_i = ptube_i.position;
					int time_frame_i = ptube_i.t_f;
					for (list<simple_tube>::iterator jjter = s_database.tubes[j_index].tubes.begin(); jjter != s_database.tubes[j_index].tubes.end(); jjter++)
					{
						simple_tube ptube_j = (simple_tube)(*jjter);
						CvRect rect_j = ptube_j.position;
						int time_frame_j = ptube_j.t_f;
						if(abs(time_frame_i - time_frame_j) < relate_thres_frame && isOverlap(rect_i, rect_j, relate_thres_distance))
						{
							f_relate = true;
							s_database.tubes[j_index].f_relate = true;
							s_database.tubes[i_index].relate_tube_seq.push_back(j_index);
							break;
						}
						else if((time_frame_i == time_frame_j)&&isOverlap(rect_i, rect_j, neighbour_thres_distance)&&(s_database.tubes[j_index].i_end - s_database.tubes[j_index].i_start) <= thres_min_duration)
						{
							f_neighbour = true;
							s_database.tubes[i_index].neighbour_tube_seq.push_back(j_index);
							break;
						}
					}

				}

			}
			//判断TOT
			for(int j_index = i_index; j_index < s_database.i_total; j_index++)
			{
				if(s_database.tubes[j_index].i_start < s_database.tubes[i_index].i_start)
					continue;
				if(s_database.tubes[j_index].i_start >= s_database.tubes[i_index].i_end)
					break;
				int i_duration_i = s_database.tubes[i_index].i_end - s_database.tubes[i_index].i_start;
				int i_duration_j = s_database.tubes[j_index].i_end - s_database.tubes[j_index].i_start;
				int i_duration = i_duration_i <= i_duration_j ? i_duration_i : i_duration_j;
				if(s_database.tubes[i_index].i_end -  s_database.tubes[j_index].i_start >= i_duration*to_thres_overlap && 
					(s_database.tubes[j_index].i_end - s_database.tubes[j_index].i_start) <= thres_min_duration)
				{
					s_database.tubes[i_index].overlap_tube_seq.push_back(j_index);
				}
			}
			//判断TST
			Mat M_i = Mat::zeros(s_database.i_height, s_database.i_width, CV_8UC1);
			Mat M_j = Mat::zeros(s_database.i_height, s_database.i_width, CV_8UC1);
			Mat M_i_xor_j = Mat::zeros(s_database.i_height, s_database.i_width, CV_8UC1);
			for (list<simple_tube>::iterator iiter = s_database.tubes[i_index].tubes.begin(); iiter != s_database.tubes[i_index].tubes.end(); iiter++)
			{
				simple_tube ptube_i = (simple_tube)(*iiter);
				CvRect rect_i = ptube_i.position;
				Mat M_i_i(M_i, rect_i);
				M_i_i.setTo(1);

			}
			int area_i = countNonZero(M_i);

			for(int j_index = i_index; j_index < s_database.i_total; j_index++)
			{
				if(s_database.tubes[j_index].i_start < s_database.tubes[i_index].i_start)
					continue;
				if(s_database.tubes[j_index].i_start -  s_database.tubes[i_index].i_end > ts_thres_frame)
					break;


				for (list<simple_tube>::iterator jjter = s_database.tubes[j_index].tubes.begin(); jjter != s_database.tubes[j_index].tubes.end(); jjter++)
				{
					simple_tube ptube_j = (simple_tube)(*jjter);
					CvRect rect_j = ptube_j.position;
					Mat M_j_j(M_j, rect_j);
					M_j_j.setTo(1);
				}

				bitwise_xor(M_i,M_j,M_i_xor_j);
				int area_xor = countNonZero(M_i_xor_j);
				int area_j = countNonZero(M_j);
				int area_max = area_i >= area_j ? area_i : area_j;
				if(area_xor <= area_max &&(s_database.tubes[j_index].i_end - s_database.tubes[j_index].i_start) <= thres_min_duration)
				{
					s_database.tubes[i_index].later_tube_seq.push_back(j_index);
				}

			}
		}
		
	}
	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		printf("Recheck tubes: %d\t\n",i_index);
		if(s_database.tubes[i_index].f_relate == false && (s_database.tubes[i_index].i_end - s_database.tubes[i_index].i_start) <= thres_min_duration)
		{
			s_database.tubes.erase(i_index);
		}
	}


	FILE* fp2 = fopen("D:\\tube_database_refine.csv", "wt");  
	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		hash_map<int, tube_seq>::iterator it;
		it = s_database.tubes.find(i_index);
		if (it == s_database.tubes.end())
		{
			continue;
		}
		fprintf(fp2, "Index: %d, Duration: %d, Dependency: %d, RT: ", s_database.tubes[i_index].i_index, s_database.tubes[i_index].i_end-s_database.tubes[i_index].i_start, s_database.tubes[i_index].f_relate);
		for (list<int>::iterator iter = s_database.tubes[i_index].relate_tube_seq.begin(); iter != s_database.tubes[i_index].relate_tube_seq.end(); iter++)
		{
			fprintf(fp2, "%d ", (*iter));
		}
		fprintf(fp2, ",NT: ");
		for (list<int>::iterator iter = s_database.tubes[i_index].neighbour_tube_seq.begin(); iter != s_database.tubes[i_index].neighbour_tube_seq.end(); iter++)
		{
			fprintf(fp2, "%d ", (*iter));
		}
		fprintf(fp2, ",TOT: ");
		for (list<int>::iterator iter = s_database.tubes[i_index].overlap_tube_seq.begin(); iter != s_database.tubes[i_index].overlap_tube_seq.end(); iter++)
		{
			fprintf(fp2, "%d ", (*iter));
		}
		fprintf(fp2, ",TST: ");
		for (list<int>::iterator iter = s_database.tubes[i_index].later_tube_seq.begin(); iter != s_database.tubes[i_index].later_tube_seq.end(); iter++)
		{
			fprintf(fp2, "%d ", (*iter));
		}
		fprintf(fp2, ",\n");

		for (list<simple_tube>::iterator iter = s_database.tubes[i_index].tubes.begin(); iter != s_database.tubes[i_index].tubes.end(); iter++)
		{
			simple_tube ptube = (simple_tube)(*iter);
			fprintf(fp2, "%d,%d,%d,%d,%d,%d\n", s_database.tubes[i_index].i_index, ptube.position.x, ptube.position.y, ptube.position.width, ptube.position.height, ptube.t_f);   

		}
	}
	fclose(fp2);
}


/**
*融合跟踪数据库，输出视频摘要，同时释放Database内存
*/
/*
void mergeDB(list< list<tube *> > & database, const char * videoFilePath, const int fps, const CvSize size, const IplImage * bgImg) {
	if (database.empty()) {
		printf("Fail: Database is empty, cannot merge!\n");
		return;
	}

	if (videoFilePath == NULL) {
		printf("Fail: NULL output file path/name!\n");
		return;
	}

	CvVideoWriter * writer = cvCreateVideoWriter(videoFilePath, CV_FOURCC('h', '2', '6', '4'), fps, size, 1);
	if (writer == NULL) {
		printf("Fail: Invalid video file path/name, cannot build a video writer!\n");
		return;
	}

	IplImage * frame = cvCreateImage(size, IPL_DEPTH_8U, 3); //输出帧
	tube * tmp = NULL; //临时tube指针
	vector<CvRect> rects; //保存每帧被填充的位置
	int frame_id = 0;

	printf("Merging track database...\n");
	while (!database.empty()) {

		cvCopy(bgImg, frame); //每次开始用背景图片刷新输出帧

		for (list< list<tube *> >::iterator i = database.begin(); i != database.end();) { //遍历级联链表每个子链表首项
			if (i->empty()) {
				i = database.erase(i);
				continue;
			}

			bool canAdd = true;
			tmp = *(i->begin());

			for (size_t t = 0; t != rects.size(); t++) { //检查是否与已经填充的有重叠
				if (isOverlap(rects[t], tmp->position)) {
					canAdd = false;
					break;
				}
			}

			if (canAdd) { //如果可以加入则将tube写入输出帧
				cvSetImageROI(frame, tmp->position);
				cvCopy(tmp->target, frame);
				cvResetImageROI(frame);
				rects.push_back(tmp->position); //将填充图块的位置保存
				delete tmp; //释放tmp指向的tube的图像内存
				i->pop_front();
				i++;
			}
			else
				i++;
		}
		cvWriteFrame(writer, frame); //写帧
		rects.clear(); //清空rects
		printf("frame:%d written.\n", frame_id);
		frame_id += 1;
	}
	printf("Database has been merged and written to file.\n");

	//释放内存
	cvReleaseImage(&frame);
	cvReleaseVideoWriter(&writer);
}
*/

/**
*释放Database内存函数 - 辅助
*/
/*
void freeDB(list< list<tube *> > & database) {
	for (list< list<tube *> >::iterator iter1 = database.begin(); iter1 != database.end(); iter1++)
		for (list<tube *>::iterator iter2 = iter1->begin(); iter2 != iter1->end();) {
			delete *iter2;
			iter2 = (*iter1).erase(iter2);
		}
}
*/

/**
*功能：	使用KNN方法与Canny算子实现背景与前景边缘提取
*参数：	frame	-	输入图像
*		knn		-	knn算子指针
*		edges	-	输出边缘
*/
void detectEdge_KNN(IplImage* frame, Ptr<BackgroundSubtractorKNN>& knn, Mat& edges, \
	int canny_thres_1, int canny_thres_2, int canny_apperture)
{
	if(frame == NULL || knn == NULL)
	{
		printf("Error input in detectEdge function.\n");
		return;
	}
	Mat frame_mat, bgimg_knn, fgmask_knn;
	frame_mat = cvarrToMat(frame,true);  

	knn->apply(frame_mat, fgmask_knn, 0.005);//-1默认
	CvMat fgMat_knn = fgmask_knn;
	cvErode(&fgMat_knn, &fgMat_knn, 0, 1);
	cvDilate(&fgMat_knn, &fgMat_knn, 0, 1);
	cvDilate(&fgMat_knn, &fgMat_knn, 0, 1);
	cvDilate(&fgMat_knn, &fgMat_knn, 0, 1);
	cvDilate(&fgMat_knn, &fgMat_knn, 0, 1);
	cvErode(&fgMat_knn, &fgMat_knn, 0, 1);
	cvErode(&fgMat_knn, &fgMat_knn, 0, 1);
	cvErode(&fgMat_knn, &fgMat_knn, 0, 1);
	fgmask_knn = cvarrToMat(&fgMat_knn, true);
	//knn->getBackgroundImage(bgimg_knn);

	edges=fgmask_knn.clone();
	Canny(fgmask_knn, edges, canny_thres_1, canny_thres_2, canny_apperture);

	//imshow("KNN前景", fgmask_knn);
}


/**
*YUV420转IplImage，用于Tiandy SDK的数据读取
*/
IplImage* YUV420_To_IplImage_Opencv(unsigned char* pYUV420, int width, int height)
{
	if (!pYUV420)
	{
		return NULL;
	}

	IplImage *yuvimage,*rgbimg,*yimg,*uimg,*vimg,*uuimg,*vvimg;
	int nWidth = width;
	int nHeight = height;
	rgbimg = cvCreateImage(cvSize(nWidth, nHeight),IPL_DEPTH_8U,3);
	yuvimage = cvCreateImage(cvSize(nWidth, nHeight),IPL_DEPTH_8U,3);

	yimg = cvCreateImageHeader(cvSize(nWidth, nHeight),IPL_DEPTH_8U,1);
	uimg = cvCreateImageHeader(cvSize(nWidth/2, nHeight/2),IPL_DEPTH_8U,1);
	vimg = cvCreateImageHeader(cvSize(nWidth/2, nHeight/2),IPL_DEPTH_8U,1);
	uuimg = cvCreateImage(cvSize(nWidth, nHeight),IPL_DEPTH_8U,1);
	vvimg = cvCreateImage(cvSize(nWidth, nHeight),IPL_DEPTH_8U,1);

	cvSetData(yimg,pYUV420, nWidth);
	cvSetData(uimg,pYUV420+nWidth*nHeight, nWidth/2);
	cvSetData(vimg,pYUV420+long(nWidth*nHeight*1.25), nWidth/2);
	cvResize(uimg,uuimg,CV_INTER_LINEAR);
	cvResize(vimg,vvimg,CV_INTER_LINEAR);

	cvMerge(yimg,uuimg,vvimg,NULL,yuvimage);
	cvCvtColor(yuvimage,rgbimg,CV_YCrCb2RGB);

	cvReleaseImage(&uuimg);
	cvReleaseImage(&vvimg);
	cvReleaseImageHeader(&yimg);
	cvReleaseImageHeader(&uimg);
	cvReleaseImageHeader(&vimg);
	cvReleaseImage(&yuvimage);
	if (!rgbimg)
	{
		return NULL;
	}
	return rgbimg;
}

/**
*Tiandy SDK的数据读取回调函数
*/
void __cdecl GetDecAVCbk( int _iID, const DecAVInfo *_pDecAVInfo )
{
	if (_pDecAVInfo->iType == 0)  //Video
	{
		memcpy(buf, _pDecAVInfo->pucData, _pDecAVInfo->iDataLen*sizeof(char));

		IplImage* frame = YUV420_To_IplImage_Opencv(buf, _pDecAVInfo->iWidth,  _pDecAVInfo->iHeight);
		//cvShowImage("fgImg", frame);
		//cvWaitKey(30);

		Mat frame_mat;
		int find_num = 0;
		vector<CvRect> rects; //存放找到的前景块位置
		vector<Point2d> centers; //存放找到的前景块中心
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;

		frame_mat = cvarrToMat(frame,true);  
		Mat edges;
		detectEdge_KNN(frame, m_KNN, edges);
		imshow("Canny边缘", edges);


		findContours(edges, contours, hierarchy, CV_RETR_EXTERNAL, CHAIN_APPROX_TC89_KCOS, Point());
		find_num = contours.size();															   

		if (find_num > (m_Thres_ObjNum * 3) || frame == NULL || m_iFrame_ind <= m_Thres_SkipBeginFrame) {
			m_iFrame_ind += 1;
			cvReleaseImage(&frame);
			return;
		}
		printf("frame:%d\tfind contour num:%d\n", m_iFrame_ind, find_num);

		//contour analysis
		if(contours.size()>0)
		{
			for( int i = 0; i < contours.size(); i++ )
			{
				Rect r = boundingRect(contours[i]);
				rects.push_back(r);
				centers.push_back((r.br()+r.tl())*0.5);
			}
		}

		for(int i=0; i<centers.size(); i++)
		{
			circle(frame_mat,centers[i],3,Scalar(0,255,0),1,CV_AA);
		}


		if(centers.size()>0)
		{
			m_tracker.Update(centers, rects, m_iFrame_ind, m_Thres_MinArea);

			cout << m_tracker.tracks.size()  << endl;

			for(int i=0;i<m_tracker.tracks.size();i++)
			{
				if(m_tracker.tracks[i]->trace.size()>1)
				{
					for(int j=0;j<m_tracker.tracks[i]->trace.size()-1;j++)
					{
						line(frame_mat,m_tracker.tracks[i]->trace[j],m_tracker.tracks[i]->trace[j+1],Colors[m_tracker.tracks[i]->track_id%9],2,CV_AA);
					}
				}
			}
		}

		imshow("Kalman", frame_mat);
		cvWaitKey(20);

		m_iFrame_ind += 1;
		rects.clear();
		cvReleaseImage(&frame);
	}

	if (_pDecAVInfo->iType == 1)  //Audio
	{
		;
	}
}