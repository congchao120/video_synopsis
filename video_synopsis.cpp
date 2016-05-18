
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
#include <opencv2/flann.hpp>

#include "LS_DlgVideo.h"
#include "TD_PlaySDK.h"
#define M_PI       3.14159265358979323846
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
static CTracker m_tracker(0.1,0.2, m_database.tube_seqs, 30.0,10,100);
static Ptr<BackgroundSubtractorKNN> m_KNN = createBackgroundSubtractorKNN(100, 200);//history = 500, dist2Thres = 500
static Ptr<BackgroundSubtractorMOG2> m_GMM = createBackgroundSubtractorMOG2();
static Scalar Colors[]={Scalar(255,0,0),Scalar(0,255,0),Scalar(0,0,255),Scalar(255,255,0),Scalar(0,255,255),Scalar(255,0,255),Scalar(255,127,255),Scalar(127,0,255),Scalar(127,0,127)};


/**
*基本数据单元构造函数
*/
simple_tube::simple_tube(CvRect rect, int f) :position(rect), t_f(f) {
}

/**
*基本数据单元构造函数
*/
simple_tube::simple_tube() :position(CvRect()), t_f(0) {
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
tube_seq::tube_seq(hash_map<int, simple_tube> _seq, int _iframe, int _index) 
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
*两矩形块合并，无视是否相交
*/
void mergeRects(CvRect& main_rect, CvRect& sub_rect) {
	int x = 0, y = 0, width = 0, height = 0;//临时变量
	if (main_rect.x < sub_rect.x) {
		x = main_rect.x;
		width = max(sub_rect.x - main_rect.x + sub_rect.width, main_rect.width);
	}
	else {
		x = sub_rect.x;
		width = max(main_rect.x - sub_rect.x + main_rect.width, sub_rect.width);
	}

	if (main_rect.y < sub_rect.y) {
		y = main_rect.y;
		height = max(sub_rect.y - main_rect.y + sub_rect.height, main_rect.height);
	}
	else {
		y = sub_rect.y;
		height = max(main_rect.y - sub_rect.y + main_rect.height, sub_rect.height);
	}

	//合并
	main_rect.x = x;
	main_rect.y = y;
	main_rect.width = width;
	main_rect.height = height;
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

	m_database.tube_seqs.clear();
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
		int find_num = 0;
		int last_find_num = 0;

		printf("Build track database...\n");
		while (frame = cvQueryFrame(capture)) {

			Mat frame_mat;
			vector<CvRect> rects; //存放找到的前景块位置
			vector<Point2d> centers; //存放找到的前景块中心
			vector<vector<Point> > contours;
			vector<Vec4i> hierarchy;

			frame_mat = cvarrToMat(frame,true);  
			Mat edges;
			detectEdge_KNN(frame, m_KNN, edges);
			//detectEdge_GMM(frame, m_GMM, edges);
			imshow("Canny边缘", edges);


			findContours(edges, contours, hierarchy, CV_RETR_EXTERNAL, CHAIN_APPROX_TC89_KCOS, Point());
			last_find_num = find_num;
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
		m_database.i_total = m_database.tube_seqs.size();
		m_database.i_frame = m_iFrame_ind;
		printf("Tube DB has been built! Total tube num:%d\n", m_database.tube_seqs.size());
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
	if (s_database.tube_seqs.empty()) {
		printf("Database is empty, reading csv file to read the database\n");
		FILE* fp_read = fopen("D:\\tube_database.csv", "r");
		if(!fp_read)
		{
			printf("Cannot read database!\n");
			return;
		}
		char s[1024];
		int _i_total, _i_frame, _i_width, _i_height;
		int _i_index, _i_start, _i_end;
		int _x, _y, _width, _height, _t_f;
		while((fgets(s,1024,fp_read))!=NULL)
        {
			if(sscanf(s, "Tube Database,%d,%d,%d,%d", _i_total, _i_frame, _i_width, _i_height) != 0)
			{
				s_database.i_frame = _i_frame;
				s_database.i_total = _i_total;
				s_database.i_width = _i_width;
				s_database.i_height = _i_height;
				continue;
			}
			if(sscanf(s, "Tube Seq,%d,%d,%d", _i_index, _i_start, _i_end) != 0)
			{
				hash_map<int, simple_tube> _seq;
				tube_seq _tube_seq(_seq, _i_start, _i_index);
				_tube_seq.i_end = _i_end;
				s_database.tube_seqs[_i_index] = _tube_seq;
				continue;
			}
			if(sscanf(s, "%d,%d,%d,%d,%d", _x, _y, _width, _height, _t_f) != 0)
			{
				simple_tube _tube;
				_tube.position.x = _x;
				_tube.position.y = _y;
				_tube.position.width = _width;
				_tube.position.height = _height;
				_tube.t_f = _t_f;

				s_database.tube_seqs[_i_index].tubes[_t_f] = _tube;
				continue;
			}
        }

	}
	else
	{//Write database to file
		printf("Database is not empty, writing to csv file.\n");
		if (s_database.i_width <= 0 || s_database.i_height <= 0) {
			printf("Fail: Information in database is incorrect, cannot refine!\n");
			return;
		}

		FILE* fp = fopen("D:\\tube_database.csv", "wt");
		fprintf(fp, "Tube Database,%d,%d,%d,%d\n", s_database.i_total, s_database.i_frame, s_database.i_width, s_database.i_height);   
		for(int i_index = 0; i_index < s_database.i_total; i_index++)
		{
			fprintf(fp, "Tube Seq,%d,%d,%d\n", s_database.tube_seqs[i_index].i_index, s_database.tube_seqs[i_index].i_start, s_database.tube_seqs[i_index].i_end);   

			for (hash_map<int, simple_tube>::iterator jter = s_database.tube_seqs[i_index].tubes.begin(); jter != s_database.tube_seqs[i_index].tubes.end(); jter++)
			{
				simple_tube ptube = (simple_tube)(*jter).second;
				fprintf(fp, "%d,%d,%d,%d,%d\n", ptube.position.x, ptube.position.y, ptube.position.width, ptube.position.height, ptube.t_f);   

			}
		}
		fclose(fp);
	}

	
	
	Mat clustering_ele;  
	int nMaxClusters = 50;
	int nFeatures = 5;  //centerx, centery, centertime, speedx, speedy
	Mat centers(nMaxClusters, nFeatures, CV_32FC1);

	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		printf("refine tube seqs: %d\t\n", i_index);
		if(s_database.tube_seqs[i_index].tubes.size() <= thres_main_duration)
		{
			continue;
		}
		else
		{
			//判断RT与NT
			for(int j_index = i_index + 1; j_index < s_database.i_total; j_index++)
			{
				if(s_database.tube_seqs[j_index].i_start < s_database.tube_seqs[i_index].i_start)
					continue;
				if(s_database.tube_seqs[j_index].i_start -  s_database.tube_seqs[j_index].i_end > relate_thres_frame)
					break;

				bool f_relate = false;
				bool f_neighbour = false;

				for (hash_map<int, simple_tube>::iterator iiter = s_database.tube_seqs[i_index].tubes.begin(); iiter != s_database.tube_seqs[i_index].tubes.end(); iiter++)
				{
					if(f_relate || f_neighbour)
						break;
					simple_tube ptube_i = (simple_tube)(*iiter).second;
					CvRect rect_i = ptube_i.position;
					int time_frame_i = ptube_i.t_f;
					for (hash_map<int, simple_tube>::iterator jjter = s_database.tube_seqs[j_index].tubes.begin(); jjter != s_database.tube_seqs[j_index].tubes.end(); jjter++)
					{
						simple_tube ptube_j = (simple_tube)(*jjter).second;
						CvRect rect_j = ptube_j.position;
						int time_frame_j = ptube_j.t_f;
						if(abs(time_frame_i - time_frame_j) < relate_thres_frame && isOverlap(rect_i, rect_j, relate_thres_distance))
						{
							f_relate = true;
							s_database.tube_seqs[j_index].f_relate = true;
							s_database.tube_seqs[i_index].relate_tube_seq.push_back(j_index);
							break;
						}
						else if((time_frame_i == time_frame_j)&&isOverlap(rect_i, rect_j, neighbour_thres_distance)&&
							(s_database.tube_seqs[j_index].i_end - s_database.tube_seqs[j_index].i_start) >= thres_min_duration)
						{
							f_neighbour = true;
							s_database.tube_seqs[i_index].neighbour_tube_seq.push_back(j_index);
							break;
						}
					}

				}

			}
			//判断TOT
			for(int j_index = i_index + 1; j_index < s_database.i_total; j_index++)
			{
				if(s_database.tube_seqs[j_index].i_start < s_database.tube_seqs[i_index].i_start)
					continue;
				if(s_database.tube_seqs[j_index].i_start >= s_database.tube_seqs[i_index].i_end)
					break;
				int i_duration_i = s_database.tube_seqs[i_index].i_end - s_database.tube_seqs[i_index].i_start;
				int i_duration_j = s_database.tube_seqs[j_index].i_end - s_database.tube_seqs[j_index].i_start;
				int i_duration = i_duration_i <= i_duration_j ? i_duration_i : i_duration_j;
				if(s_database.tube_seqs[i_index].i_end -  s_database.tube_seqs[j_index].i_start >= i_duration*to_thres_overlap && 
					(s_database.tube_seqs[j_index].i_end - s_database.tube_seqs[j_index].i_start) >= thres_min_duration)
				{
					s_database.tube_seqs[i_index].overlap_tube_seq.push_back(j_index);
				}
			}
			//判断TST
			Mat M_i = Mat::zeros(s_database.i_height, s_database.i_width, CV_8UC1);
			Mat M_j = Mat::zeros(s_database.i_height, s_database.i_width, CV_8UC1);
			Mat M_i_xor_j = Mat::zeros(s_database.i_height, s_database.i_width, CV_8UC1);
			for (hash_map<int, simple_tube>::iterator iiter = s_database.tube_seqs[i_index].tubes.begin(); iiter != s_database.tube_seqs[i_index].tubes.end(); iiter++)
			{
				simple_tube ptube_i = (simple_tube)(*iiter).second;
				CvRect rect_i = ptube_i.position;
				Mat M_i_i(M_i, rect_i);
				M_i_i.setTo(1);

			}
			int area_i = countNonZero(M_i);

			for(int j_index = i_index + 1; j_index < s_database.i_total; j_index++)
			{
				if(s_database.tube_seqs[j_index].i_start < s_database.tube_seqs[i_index].i_start)
					continue;
				if(s_database.tube_seqs[j_index].i_start -  s_database.tube_seqs[i_index].i_end > ts_thres_frame)
					break;


				for (hash_map<int, simple_tube>::iterator jjter = s_database.tube_seqs[j_index].tubes.begin(); jjter != s_database.tube_seqs[j_index].tubes.end(); jjter++)
				{
					simple_tube ptube_j = (simple_tube)(*jjter).second;
					CvRect rect_j = ptube_j.position;
					Mat M_j_j(M_j, rect_j);
					M_j_j.setTo(1);
				}

				bitwise_xor(M_i,M_j,M_i_xor_j);
				int area_xor = countNonZero(M_i_xor_j);
				int area_j = countNonZero(M_j);
				int area_max = area_i >= area_j ? area_i : area_j;
				if(area_xor <= area_max &&(s_database.tube_seqs[j_index].i_end - s_database.tube_seqs[j_index].i_start) >= thres_min_duration)
				{
					s_database.tube_seqs[i_index].later_tube_seq.push_back(j_index);
				}

			}
		}
		
	}
	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		printf("Recheck tubes: %d\t\n",i_index);
		if(s_database.tube_seqs[i_index].f_relate == false && (s_database.tube_seqs[i_index].i_end - s_database.tube_seqs[i_index].i_start) <= thres_min_duration)
		{
			s_database.tube_seqs.erase(i_index);
		}
	}


	FILE* fp2 = fopen("D:\\tube_database_refine.csv", "wt");  
	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		hash_map<int, tube_seq>::iterator it;
		it = s_database.tube_seqs.find(i_index);
		if (it == s_database.tube_seqs.end())
		{
			continue;
		}
		fprintf(fp2, "Index: %d, Duration: %d, Dependency: %d, RT: ", s_database.tube_seqs[i_index].i_index, s_database.tube_seqs[i_index].i_end-s_database.tube_seqs[i_index].i_start, s_database.tube_seqs[i_index].f_relate);
		for (list<int>::iterator iter = s_database.tube_seqs[i_index].relate_tube_seq.begin(); iter != s_database.tube_seqs[i_index].relate_tube_seq.end(); iter++)
		{
			fprintf(fp2, "%d ", (*iter));
		}
		fprintf(fp2, ",NT: ");
		for (list<int>::iterator iter = s_database.tube_seqs[i_index].neighbour_tube_seq.begin(); iter != s_database.tube_seqs[i_index].neighbour_tube_seq.end(); iter++)
		{
			fprintf(fp2, "%d ", (*iter));
		}
		fprintf(fp2, ",TOT: ");
		for (list<int>::iterator iter = s_database.tube_seqs[i_index].overlap_tube_seq.begin(); iter != s_database.tube_seqs[i_index].overlap_tube_seq.end(); iter++)
		{
			fprintf(fp2, "%d ", (*iter));
		}
		fprintf(fp2, ",TST: ");
		for (list<int>::iterator iter = s_database.tube_seqs[i_index].later_tube_seq.begin(); iter != s_database.tube_seqs[i_index].later_tube_seq.end(); iter++)
		{
			fprintf(fp2, "%d ", (*iter));
		}
		fprintf(fp2, ",\n");

		for (hash_map<int, simple_tube>::iterator iter = s_database.tube_seqs[i_index].tubes.begin(); iter != s_database.tube_seqs[i_index].tubes.end(); iter++)
		{
			simple_tube ptube = (simple_tube)(*iter).second;
			fprintf(fp2, "%d,%d,%d,%d,%d,%d\n", s_database.tube_seqs[i_index].i_index, ptube.position.x, ptube.position.y, ptube.position.width, ptube.position.height, ptube.t_f);   

		}
	}
	fclose(fp2);
}



/**
*  提炼tube数据库，判断独立tube，相关tubes，邻近tubes，顺序tubes，时间重叠tubes
*  首先判断相关与否
*  随后筛选不相关tube，5帧内删除
*  判断邻近，类似相关，25*25*1判断
*  判断顺序
*  判断时间重叠
*/
void refineDB_Mat(tube_database& s_database, const int thres_main_duration, \
		const int relate_thres_distance, const int relate_thres_frame, const int thres_min_duration, \
		const int neighbour_thres_distance, const int ts_thres_frame, const float to_thres_overlap) {
	if (s_database.tube_seqs.empty()) {
		printf("Database is empty, reading csv file to read the database\n");
		FILE* fp_read = fopen("D:\\tube_database.csv", "r");
		if(!fp_read)
		{
			printf("Cannot read database!\n");
			return;
		}
		char s[1024];
		int _i_total, _i_frame, _i_width, _i_height;
		int _i_index, _i_start, _i_end;
		int _x, _y, _width, _height, _t_f;
		while((fgets(s,1024,fp_read))!=NULL)
        {
			if(sscanf(s, "Tube Database,%d,%d,%d,%d", &_i_total, &_i_frame, &_i_width, &_i_height) != 0)
			{
				s_database.i_frame = _i_frame;
				s_database.i_total = _i_total;
				s_database.i_width = _i_width;
				s_database.i_height = _i_height;
				continue;
			}
			if(sscanf(s, "Tube Seq,%d,%d,%d", &_i_index, &_i_start, &_i_end) != 0)
			{
				hash_map<int, simple_tube> _seq;
				tube_seq _tube_seq(_seq, _i_start, _i_index);
				_tube_seq.i_end = _i_end;
				s_database.tube_seqs[_i_index] = _tube_seq;
				continue;
			}
			if(sscanf(s, "%d,%d,%d,%d,%d", &_x, &_y, &_width, &_height, &_t_f) != 0)
			{
				simple_tube _tube;
				_tube.position.x = _x;
				_tube.position.y = _y;
				_tube.position.width = _width;
				_tube.position.height = _height;
				_tube.t_f = _t_f;

				s_database.tube_seqs[_i_index].tubes[_t_f] = _tube;
				continue;
			}
        }

	}
	else
	{//Write database to file
		printf("Database is not empty, writing to csv file.\n");
		if (s_database.i_width <= 0 || s_database.i_height <= 0) {
			printf("Fail: Information in database is incorrect, cannot refine!\n");
			return;
		}

		FILE* fp = fopen("D:\\tube_database.csv", "wt");
		fprintf(fp, "Tube Database,%d,%d,%d,%d\n", s_database.i_total, s_database.i_frame, s_database.i_width, s_database.i_height);   
		for(int i_index = 0; i_index < s_database.i_total; i_index++)
		{
			fprintf(fp, "Tube Seq,%d,%d,%d\n", s_database.tube_seqs[i_index].i_index, s_database.tube_seqs[i_index].i_start, s_database.tube_seqs[i_index].i_end);   

			for (hash_map<int, simple_tube>::iterator jter = s_database.tube_seqs[i_index].tubes.begin(); jter != s_database.tube_seqs[i_index].tubes.end(); jter++)
			{
				simple_tube ptube = (simple_tube)(*jter).second;
				fprintf(fp, "%d,%d,%d,%d,%d\n", ptube.position.x, ptube.position.y, ptube.position.width, ptube.position.height, ptube.t_f);   

			}
		}
		fclose(fp);
	}
	
	printf("Clustering tubes\n");

	//计算特征矩阵
	//int nMaxClusters = 100;
	//int nFeatures = 5;//centerx, centery, centertime, speedx, speedy
	int nMaxClusters = 9;
	int nFeatures = 8;//centerx, centery, centertime, durationtime, speedx, speedy, area, area_rate
	Mat clustering_ele(s_database.i_total, nFeatures, CV_32FC1);  
	Mat centers(nMaxClusters, nFeatures, CV_32FC1);
	float max_speed_x, max_speed_y;//归一化用
	float thres_speed_x;
	float thres_speed_y;
	max_speed_x = 0.0f;
	max_speed_y = 0.0f;

	//计算特征矩阵
	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		float center_time = (s_database.tube_seqs[i_index].i_end + s_database.tube_seqs[i_index].i_start)*0.5f;
		float speed_x, speed_y, center_x, center_y;
		float start_x, start_y, end_x, end_y;
		float duration;

		hash_map<int, simple_tube>::iterator iter = s_database.tube_seqs[i_index].tubes.begin();
		simple_tube ptube_i = (simple_tube)(*iter).second;
		start_x = (ptube_i.position.x + ptube_i.position.width)*0.5f;
		start_y = (ptube_i.position.y + ptube_i.position.height)*0.5f;

		center_x = 0.0f;
		center_y = 0.0f;
		for (hash_map<int, simple_tube>::iterator iiter = s_database.tube_seqs[i_index].tubes.begin(); iiter != s_database.tube_seqs[i_index].tubes.end(); iiter++)
		{
			ptube_i = (simple_tube)(*iiter).second;
			center_x += (ptube_i.position.x + ptube_i.position.width)*0.5f;
			center_y += (ptube_i.position.y + ptube_i.position.height)*0.5f;

			end_x = (ptube_i.position.x + ptube_i.position.width)*0.5f;
			end_y = (ptube_i.position.y + ptube_i.position.height)*0.5f;

		}
		center_x /= s_database.tube_seqs[i_index].tubes.size();
		center_y /= s_database.tube_seqs[i_index].tubes.size();
		if(s_database.tube_seqs[i_index].tubes.size() <= 1)
		{
			speed_x = 0.0f;
			speed_y = 0.0f;
		}
		else
		{
			speed_x = float(end_x - start_x)/float(s_database.tube_seqs[i_index].i_end - s_database.tube_seqs[i_index].i_start);
			speed_y = float(end_y - start_y)/float(s_database.tube_seqs[i_index].i_end - s_database.tube_seqs[i_index].i_start);
		}
		duration = float(s_database.tube_seqs[i_index].i_end - s_database.tube_seqs[i_index].i_start);
		if(fabs(speed_x) >= max_speed_x)
			max_speed_x = fabs(speed_x);
		if(fabs(speed_y) >= max_speed_y)
			max_speed_y = fabs(speed_y);

		clustering_ele.at<float>(i_index, 0) = center_x;
		clustering_ele.at<float>(i_index, 1) = center_y;
		clustering_ele.at<float>(i_index, 2) = center_time;
		clustering_ele.at<float>(i_index, 3) = duration;
		clustering_ele.at<float>(i_index, 4) = speed_x;
		clustering_ele.at<float>(i_index, 5) = speed_y;
	}

	//设定八方向与速度阈值，进行分类
	thres_speed_x = max_speed_x/100.0f;
	thres_speed_y = max_speed_y/100.0f;
	float classify_angle[8] = {-135, -90, -45, 0, 45, 90, 135, 180};


	Mat labels(s_database.i_total, 1, CV_32SC1);;
	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		int center_index = -1;
		float tmp_speed_x = clustering_ele.at<float>(i_index, 4);
		float tmp_speed_y = clustering_ele.at<float>(i_index, 5);
		if(fabs(tmp_speed_x) <= thres_speed_x && fabs(tmp_speed_y) <= thres_speed_y)
		{
			center_index = 0;
		}
		else
		{
			float tmp_angle = atan2(tmp_speed_y, tmp_speed_x) + M_PI;
			center_index = int(floorf(tmp_angle/M_PI*180.0f))/45+1;
		}
		
		labels.at<int>(i_index, 0) = center_index;
	}

	//kmeans(clustering_ele, nMaxClusters, labels, TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 20, 0.001),
    //           3, KMEANS_USE_INITIAL_LABELS, centers);

	
	FILE* fp3 = fopen("D:\\tube_clustering_element.csv", "wt");  
	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		fprintf(fp3, "%d, %f,%f,%f,%f,%f,%f,%d\n", i_index, clustering_ele.at<float>(i_index, 0), clustering_ele.at<float>(i_index, 1),
			clustering_ele.at<float>(i_index, 2), clustering_ele.at<float>(i_index, 3), clustering_ele.at<float>(i_index, 4), 
			clustering_ele.at<float>(i_index, 5), labels.at<int>(i_index, 0));  
	}
	fclose(fp3);

	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		printf("refine tubes: %d\t\n", i_index);
		if(s_database.tube_seqs[i_index].tubes.size() <= thres_main_duration)
		{
			continue;
		}
		else
		{
			//判断RT
			Mat M_i = Mat::zeros(s_database.i_height, s_database.i_width, CV_16UC1);
			for (hash_map<int, simple_tube>::iterator iiter = s_database.tube_seqs[i_index].tubes.begin(); iiter != s_database.tube_seqs[i_index].tubes.end(); iiter++)
			{
				simple_tube ptube_i = (simple_tube)(*iiter).second;
				CvRect rect_i = ptube_i.position;
				rect_i.x -= relate_thres_distance;
				rect_i.y -= relate_thres_distance;
				if(rect_i.x < 0)
					rect_i.x = 0;
				if(rect_i.y < 0)
					rect_i.y = 0;
				rect_i.width += 2*relate_thres_distance;
				rect_i.height += 2*relate_thres_distance;
				if(rect_i.x + rect_i.width >= s_database.i_width)
					rect_i.width = s_database.i_width - rect_i.x;
				if(rect_i.y + rect_i.height >= s_database.i_height)
					rect_i.height = s_database.i_height - rect_i.y;
				Mat M_i_i(M_i, rect_i);
				M_i_i.setTo(ptube_i.t_f);
			}
			//int area_i = countNonZero(M_i);

			for(int j_index = i_index + 1; j_index < s_database.i_total; j_index++)
			{
				if(s_database.tube_seqs[j_index].i_start < s_database.tube_seqs[i_index].i_start)
					continue;
				if(s_database.tube_seqs[j_index].i_start -  s_database.tube_seqs[j_index].i_end > relate_thres_frame)
					break;

				bool f_relate = false;
				bool f_neighbour = false;

				for (hash_map<int, simple_tube>::iterator jjter = s_database.tube_seqs[j_index].tubes.begin(); jjter != s_database.tube_seqs[j_index].tubes.end(); jjter++)
				{
					simple_tube ptube_j = (simple_tube)(*jjter).second;
					CvRect rect_j = ptube_j.position;
					int time_frame_j = ptube_j.t_f;
					CvPoint pt1 = CvPoint(rect_j.x, rect_j.y);
					CvPoint pt2 = CvPoint(rect_j.x+rect_j.width/2, rect_j.y+rect_j.height/2);
					CvPoint pt3 = CvPoint(rect_j.x+rect_j.width, rect_j.y+rect_j.height);
					int val1 = M_i.at<unsigned short int>(pt1.y, pt1.x);
					int val2 = M_i.at<unsigned short int>(pt2.y, pt2.x);
					int val3 = M_i.at<unsigned short int>(pt3.y, pt3.x);
					if(val1 > 0 && time_frame_j - val1 > 0 && time_frame_j - val1 <= relate_thres_frame)
					{
						s_database.tube_seqs[j_index].f_relate = true;
						s_database.tube_seqs[i_index].relate_tube_seq.push_back(j_index);
						break;
					}
					else if(val2 > 0 && time_frame_j - val2 > 0 && time_frame_j - val2 <= relate_thres_frame)
					{
						s_database.tube_seqs[j_index].f_relate = true;
						s_database.tube_seqs[i_index].relate_tube_seq.push_back(j_index);
						break;
					}
					else if(val3 > 0 && time_frame_j - val3 > 0 && time_frame_j - val3 <= relate_thres_frame)
					{
						s_database.tube_seqs[j_index].f_relate = true;
						s_database.tube_seqs[i_index].relate_tube_seq.push_back(j_index);
						break;
					}
				}

			}

			//判断NT
			M_i = Mat::zeros(s_database.i_height, s_database.i_width, CV_16UC1);
			for (hash_map<int, simple_tube>::iterator iiter = s_database.tube_seqs[i_index].tubes.begin(); iiter != s_database.tube_seqs[i_index].tubes.end(); iiter++)
			{
				simple_tube ptube_i = (simple_tube)(*iiter).second;
				CvRect rect_i = ptube_i.position;
				rect_i.x -= neighbour_thres_distance;
				rect_i.y -= neighbour_thres_distance;
				if(rect_i.x < 0)
					rect_i.x = 0;
				if(rect_i.y < 0)
					rect_i.y = 0;
				rect_i.width += 2*neighbour_thres_distance;
				rect_i.height += 2*neighbour_thres_distance;
				if(rect_i.x + rect_i.width >= s_database.i_width)
					rect_i.width = s_database.i_width - rect_i.x;
				if(rect_i.y + rect_i.height >= s_database.i_height)
					rect_i.height = s_database.i_height - rect_i.y;
				Mat M_i_i(M_i, rect_i);
				M_i_i.setTo(ptube_i.t_f);

			}
			//int area_i = countNonZero(M_i);

			for(int j_index = i_index + 1; j_index < s_database.i_total; j_index++)
			{
				if(s_database.tube_seqs[j_index].i_start < s_database.tube_seqs[i_index].i_start)
					continue;
				if(s_database.tube_seqs[j_index].i_start -  s_database.tube_seqs[j_index].i_end > relate_thres_frame)
					break;

				bool f_relate = false;
				bool f_neighbour = false;

				for (hash_map<int, simple_tube>::iterator jjter = s_database.tube_seqs[j_index].tubes.begin(); jjter != s_database.tube_seqs[j_index].tubes.end(); jjter++)
				{
					simple_tube ptube_j = (simple_tube)(*jjter).second;
					CvRect rect_j = ptube_j.position;
					int time_frame_j = ptube_j.t_f;
					CvPoint pt1 = CvPoint(rect_j.x, rect_j.y);
					CvPoint pt2 = CvPoint(rect_j.x+rect_j.width/2, rect_j.y+rect_j.height/2);
					CvPoint pt3 = CvPoint(rect_j.x+rect_j.width, rect_j.y+rect_j.height);
					int val1 = M_i.at<unsigned short int>(pt1.y, pt1.x);
					int val2 = M_i.at<unsigned short int>(pt2.y, pt2.x);
					int val3 = M_i.at<unsigned short int>(pt3.y, pt3.x);
					if(val1 > 0 && time_frame_j - val1 > 0 && time_frame_j - val1 <= relate_thres_frame)
					{
						s_database.tube_seqs[j_index].f_relate = true;
						s_database.tube_seqs[i_index].neighbour_tube_seq.push_back(j_index);
						break;
					}
					else if(val2 > 0 && time_frame_j - val2 > 0 && time_frame_j - val2 <= relate_thres_frame)
					{
						s_database.tube_seqs[j_index].f_relate = true;
						s_database.tube_seqs[i_index].neighbour_tube_seq.push_back(j_index);
						break;
					}
					else if(val3 > 0 && time_frame_j - val3 > 0 && time_frame_j - val3 <= relate_thres_frame)
					{
						s_database.tube_seqs[j_index].f_relate = true;
						s_database.tube_seqs[i_index].neighbour_tube_seq.push_back(j_index);
						break;
					}
				}

			}

			//判断TOT
			for(int j_index = i_index + 1; j_index < s_database.i_total; j_index++)
			{
				if(s_database.tube_seqs[j_index].i_start < s_database.tube_seqs[i_index].i_start)
					continue;
				if(s_database.tube_seqs[j_index].i_start >= s_database.tube_seqs[i_index].i_end)
					break;
				int i_duration_i = s_database.tube_seqs[i_index].i_end - s_database.tube_seqs[i_index].i_start;
				int i_duration_j = s_database.tube_seqs[j_index].i_end - s_database.tube_seqs[j_index].i_start;
				int i_duration = i_duration_i <= i_duration_j ? i_duration_i : i_duration_j;
				if(s_database.tube_seqs[i_index].i_end -  s_database.tube_seqs[j_index].i_start >= i_duration*to_thres_overlap && 
					(s_database.tube_seqs[j_index].i_end - s_database.tube_seqs[j_index].i_start) >= thres_min_duration)
				{
					s_database.tube_seqs[i_index].overlap_tube_seq.push_back(j_index);
				}
			}
			//判断TST
			M_i = Mat::zeros(s_database.i_height, s_database.i_width, CV_8UC1);
			Mat M_j = Mat::zeros(s_database.i_height, s_database.i_width, CV_8UC1);
			Mat M_i_xor_j = Mat::zeros(s_database.i_height, s_database.i_width, CV_8UC1);
			for (hash_map<int, simple_tube>::iterator iiter = s_database.tube_seqs[i_index].tubes.begin(); iiter != s_database.tube_seqs[i_index].tubes.end(); iiter++)
			{
				simple_tube ptube_i = (simple_tube)(*iiter).second;
				CvRect rect_i = ptube_i.position;
				Mat M_i_i(M_i, rect_i);
				M_i_i.setTo(1);

			}
			int area_i = countNonZero(M_i);

			for(int j_index = i_index + 1; j_index < s_database.i_total; j_index++)
			{
				if(s_database.tube_seqs[j_index].i_start < s_database.tube_seqs[i_index].i_start)
					continue;
				if(s_database.tube_seqs[j_index].i_start -  s_database.tube_seqs[i_index].i_end > ts_thres_frame)
					break;


				for (hash_map<int, simple_tube>::iterator jjter = s_database.tube_seqs[j_index].tubes.begin(); jjter != s_database.tube_seqs[j_index].tubes.end(); jjter++)
				{
					simple_tube ptube_j = (simple_tube)(*jjter).second;
					CvRect rect_j = ptube_j.position;
					Mat M_j_j(M_j, rect_j);
					M_j_j.setTo(1);
				}

				bitwise_xor(M_i,M_j,M_i_xor_j);
				int area_xor = countNonZero(M_i_xor_j);
				int area_j = countNonZero(M_j);
				int area_max = area_i >= area_j ? area_i : area_j;
				if(area_xor <= area_max &&(s_database.tube_seqs[j_index].i_end - s_database.tube_seqs[j_index].i_start) >= thres_min_duration)
				{
					s_database.tube_seqs[i_index].later_tube_seq.push_back(j_index);
				}

			}
		}
		
	}

	//tube seq补点，线性
	for(int i_index = s_database.i_total - 1; i_index >= 0; i_index--)
	{
		tube_seq& main_seq = s_database.tube_seqs[i_index];
		int frame_dur = main_seq.i_end - main_seq.i_start + 1;
		int frame_num = 0;
		simple_tube prev_tube, next_tube;
		if(frame_dur <= 1)
		{
			continue;
		}
		for(int i = main_seq.i_start; i <= main_seq.i_end; i++)
		{
			hash_map<int, simple_tube>::iterator it;
			hash_map<int, simple_tube>::iterator it_prev, it_next;
			it = main_seq.tubes.find(i);
			if(it == main_seq.tubes.end())
			{//需要补点
				if(i == main_seq.i_start)
				{
					break;
				}
				if(i == main_seq.i_end)
				{
					main_seq.tubes[i] = prev_tube;
					break;
				}

				int i_prev = i-1;
				int i_next = i+1;

				while(i_prev != main_seq.i_start)
				{
					it_prev = main_seq.tubes.find(i_prev);
					if(it_prev == main_seq.tubes.end())
					{
						i_prev--;
					}
					else{
						break;
					}
				}
				prev_tube = main_seq.tubes[i_prev];

				while(i_next != main_seq.i_end)
				{
					it_next = main_seq.tubes.find(i_next);
					if(it_next == main_seq.tubes.end())
					{
						i_next++;
					}
					else{
						break;
					}
				}
				next_tube = main_seq.tubes[i_next];

				//线性插值
				simple_tube interpol_tube;
				interpol_tube.t_f = i;
				interpol_tube.position.x = prev_tube.position.x + float(next_tube.position.x - prev_tube.position.x)/float(i_next - i_prev)*float(i - i_prev);
				interpol_tube.position.y = prev_tube.position.y + float(next_tube.position.y - prev_tube.position.y)/float(i_next - i_prev)*float(i - i_prev);
				interpol_tube.position.width = prev_tube.position.width + float(next_tube.position.width - prev_tube.position.width)/float(i_next - i_prev)*float(i - i_prev);
				interpol_tube.position.height = prev_tube.position.height + float(next_tube.position.height - prev_tube.position.height)/float(i_next - i_prev)*float(i - i_prev);
				main_seq.tubes[i] = interpol_tube;
			}
			else
			{
				;
			}
		}
	}

	
	FILE* fp2 = fopen("D:\\tube_database_refine.csv", "wt");  
	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		hash_map<int, tube_seq>::iterator it;
		it = s_database.tube_seqs.find(i_index);
		if (it == s_database.tube_seqs.end())
		{
			fprintf(fp2, "Index: %d, Eliminated", i_index);
			fprintf(fp2, ",\n");
			continue;
		}
		fprintf(fp2, "Index: %d, Duration: %d, Dependency: %d, RT: ", s_database.tube_seqs[i_index].i_index, s_database.tube_seqs[i_index].i_end-s_database.tube_seqs[i_index].i_start, s_database.tube_seqs[i_index].f_relate);
		for (list<int>::iterator iter = s_database.tube_seqs[i_index].relate_tube_seq.begin(); iter != s_database.tube_seqs[i_index].relate_tube_seq.end(); iter++)
		{
			fprintf(fp2, "%d ", (*iter));
		}
		fprintf(fp2, ",NT: ");
		for (list<int>::iterator iter = s_database.tube_seqs[i_index].neighbour_tube_seq.begin(); iter != s_database.tube_seqs[i_index].neighbour_tube_seq.end(); iter++)
		{
			fprintf(fp2, "%d ", (*iter));
		}
		fprintf(fp2, ",TOT: ");
		for (list<int>::iterator iter = s_database.tube_seqs[i_index].overlap_tube_seq.begin(); iter != s_database.tube_seqs[i_index].overlap_tube_seq.end(); iter++)
		{
			fprintf(fp2, "%d ", (*iter));
		}
		fprintf(fp2, ",TST: ");
		for (list<int>::iterator iter = s_database.tube_seqs[i_index].later_tube_seq.begin(); iter != s_database.tube_seqs[i_index].later_tube_seq.end(); iter++)
		{
			fprintf(fp2, "%d ", (*iter));
		}
		fprintf(fp2, ",\n");

		for (hash_map<int, simple_tube>::iterator iter = s_database.tube_seqs[i_index].tubes.begin(); iter != s_database.tube_seqs[i_index].tubes.end(); iter++)
		{
			simple_tube ptube = (simple_tube)(*iter).second;
			fprintf(fp2, "%d,%d,%d,%d,%d,%d\n", s_database.tube_seqs[i_index].i_index, ptube.position.x, ptube.position.y, ptube.position.width, ptube.position.height, ptube.t_f);   

		}
	}
	fclose(fp2);

	//逆序融合tube
	for(int i_index = s_database.i_total - 1; i_index >= 0; i_index--)
	{
		printf("Merge tubes: %d\t\n",i_index);
		//逆序融合tube，根据聚类结果与RT综合判断是否将RT融合进主tube中
		tube_seq& main_seq = s_database.tube_seqs[i_index];
		int center_i = labels.at<int>(i_index, 0);
		for (list<int>::iterator iter = main_seq.relate_tube_seq.begin(); iter != main_seq.relate_tube_seq.end(); iter++)
		{
			int j_index = (*iter);
			hash_map<int, tube_seq>::iterator iter_seq;
			iter_seq = s_database.tube_seqs.find(j_index);
			if(iter_seq == s_database.tube_seqs.end())
			{
				continue;
			}
			tube_seq& sub_seq = s_database.tube_seqs[j_index];
			int center_j = labels.at<int>(j_index, 0);
			if(center_j == 0 || fabs(float(center_j - center_i)) <= 1.0f || fabs(float(center_j - center_i)) >= 7.0f)
			{//融合
				//交叉部分融合
				for(int ii_index = main_seq.i_start; ii_index <= main_seq.i_end; ii_index++)
				{
					//判断是否存在
					hash_map<int, simple_tube>::iterator it, jt;
					it = main_seq.tubes.find(ii_index);
					jt = sub_seq.tubes.find(ii_index);
					if (it == main_seq.tubes.end() || jt == sub_seq.tubes.end())
					{
						continue;
					}

					//存在，融合main tube和sub tube
					simple_tube &main_tube = main_seq.tubes[ii_index];
					simple_tube &sub_tube = sub_seq.tubes[ii_index];

					mergeRects(main_tube.position, sub_tube.position);
				}
				//前补
				if(main_seq.i_start > sub_seq.i_start)
				{
					for(int ii_index = sub_seq.i_start; ii_index < main_seq.i_start; ii_index++)
					{
						main_seq.tubes[ii_index] = sub_seq.tubes[ii_index];
					}
					main_seq.i_start = sub_seq.i_start;
				}
				//后补
				if(main_seq.i_end <= sub_seq.i_end)
				{
					for(int ii_index = main_seq.i_end + 1; ii_index <= sub_seq.i_end; ii_index++)
					{
						main_seq.tubes[ii_index] = sub_seq.tubes[ii_index];
					}
					main_seq.i_end = sub_seq.i_end;
				}

				//删除sub seq
				s_database.tube_seqs.erase(j_index);
			}
		}
	}

	//删除独立tube
	for(int i_index = s_database.i_total - 1; i_index >= 0; i_index--)
	{
		hash_map<int, tube_seq>::iterator it;
		it = s_database.tube_seqs.find(i_index);
		if (it == s_database.tube_seqs.end())
		{
			continue;
		}
		if(s_database.tube_seqs[i_index].f_relate == false && (s_database.tube_seqs[i_index].i_end - s_database.tube_seqs[i_index].i_start) <= thres_min_duration)
		{
			printf("Erase independent tubes: %d\t\n",i_index);
			s_database.tube_seqs.erase(i_index);
		}
	}

	//二次融合tube
	
	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		hash_map<int, tube_seq>::iterator iter_seq;
		iter_seq = s_database.tube_seqs.find(i_index);
		if (iter_seq == s_database.tube_seqs.end())
		{
			continue;
		}

		for(int j_index = 0; j_index < s_database.i_total; j_index++)
		{
			iter_seq = s_database.tube_seqs.find(j_index);
			if (iter_seq == s_database.tube_seqs.end())
			{
				continue;
			}
			if(i_index == j_index)
			{
				continue;
			}
			Mat	M_i = Mat::zeros(s_database.i_height, s_database.i_width, CV_8UC1);
			Mat M_j = Mat::zeros(s_database.i_height, s_database.i_width, CV_8UC1);
			Mat M_i_xor_j = Mat::zeros(s_database.i_height, s_database.i_width, CV_8UC1);
			int i_duration_i = s_database.tube_seqs[i_index].i_end - s_database.tube_seqs[i_index].i_start;
			int i_duration_j = s_database.tube_seqs[j_index].i_end - s_database.tube_seqs[j_index].i_start;
			int _duration = min(i_duration_i, i_duration_j);
			if(s_database.tube_seqs[i_index].i_end <= s_database.tube_seqs[j_index].i_start || 
				s_database.tube_seqs[j_index].i_end <= s_database.tube_seqs[i_index].i_start)
			{
				continue;
			}
			int _start = max(s_database.tube_seqs[i_index].i_start, s_database.tube_seqs[j_index].i_start);
			int _end = min(s_database.tube_seqs[i_index].i_end, s_database.tube_seqs[j_index].i_end);

			if(_end - _start >= _duration*0.8)
			{//满足条件：overlap time >= 80%
				for(int i_j_index = _start; i_j_index <= _end; i_j_index++)
				{
					simple_tube ptube_i = s_database.tube_seqs[i_index].tubes[i_j_index];
					simple_tube ptube_j = s_database.tube_seqs[j_index].tubes[i_j_index];
					CvRect rect_i = ptube_i.position;
					CvRect rect_j = ptube_j.position;

					Mat M_i_i(M_i, rect_i);
					Mat M_j_j(M_j, rect_j);
					M_i_i.setTo(255);
					M_j_j.setTo(255);
				}
				
				int area_i = countNonZero(M_i);
				int area_j = countNonZero(M_j);

				bitwise_xor(M_i,M_j,M_i_xor_j);
				int area_xor = countNonZero(M_i_xor_j);
				int area_max = max(area_i, area_j);
				int area_min = min(area_i, area_j);
				if(area_xor <= area_max )
				{//满足条件：overlap area >= 50%，融合
					tube_seq& main_seq = s_database.tube_seqs[i_index];
					tube_seq& sub_seq = s_database.tube_seqs[j_index];
					//imshow("main", M_i);
					//imshow("sub", M_j);
					//cvWaitKey();
					//交叉部分融合
 					for(int ii_index = main_seq.i_start; ii_index <= main_seq.i_end; ii_index++)
					{
						//判断是否存在
						hash_map<int, simple_tube>::iterator it, jt;
						it = main_seq.tubes.find(ii_index);
						jt = sub_seq.tubes.find(ii_index);
						if (it == main_seq.tubes.end() || jt == sub_seq.tubes.end())
						{
							continue;
						}

						//存在，融合main tube和sub tube
						simple_tube &main_tube = main_seq.tubes[ii_index];
						simple_tube &sub_tube = sub_seq.tubes[ii_index];

						mergeRects(main_tube.position, sub_tube.position);
					}
					//前补
					if(main_seq.i_start > sub_seq.i_start)
					{
						for(int ii_index = sub_seq.i_start; ii_index < main_seq.i_start; ii_index++)
						{
							main_seq.tubes[ii_index] = sub_seq.tubes[ii_index];
						}
						main_seq.i_start = sub_seq.i_start;
					}
					//后补
					if(main_seq.i_end <= sub_seq.i_end)
					{
						for(int ii_index = main_seq.i_end + 1; ii_index <= sub_seq.i_end; ii_index++)
						{
							main_seq.tubes[ii_index] = sub_seq.tubes[ii_index];
						}
						main_seq.i_end = sub_seq.i_end;
					}

					//删除sub seq
					s_database.tube_seqs.erase(j_index);
				}
			}
			else{
				continue;
			}
		}

	}
	

	//重新计算特征矩阵

	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		hash_map<int, tube_seq>::iterator iter_seq;
		iter_seq = s_database.tube_seqs.find(i_index);
		if (iter_seq == s_database.tube_seqs.end())
		{
			continue;
		}
		float center_time = (s_database.tube_seqs[i_index].i_end + s_database.tube_seqs[i_index].i_start)*0.5f;
		float speed_x, speed_y, center_x, center_y;
		float start_x, start_y, end_x, end_y;
		float duration;

		hash_map<int, simple_tube>::iterator iter = s_database.tube_seqs[i_index].tubes.begin();
		simple_tube ptube_i = (simple_tube)(*iter).second;
		start_x = (ptube_i.position.x + ptube_i.position.width)*0.5f;
		start_y = (ptube_i.position.y + ptube_i.position.height)*0.5f;

		center_x = 0.0f;
		center_y = 0.0f;
		for (hash_map<int, simple_tube>::iterator iiter = s_database.tube_seqs[i_index].tubes.begin(); iiter != s_database.tube_seqs[i_index].tubes.end(); iiter++)
		{
			ptube_i = (simple_tube)(*iiter).second;
			center_x += (ptube_i.position.x + ptube_i.position.width)*0.5f;
			center_y += (ptube_i.position.y + ptube_i.position.height)*0.5f;

			end_x = (ptube_i.position.x + ptube_i.position.width)*0.5f;
			end_y = (ptube_i.position.y + ptube_i.position.height)*0.5f;

		}
		center_x /= s_database.tube_seqs[i_index].tubes.size();
		center_y /= s_database.tube_seqs[i_index].tubes.size();
		if(s_database.tube_seqs[i_index].tubes.size() <= 1)
		{
			speed_x = 0.0f;
			speed_y = 0.0f;
		}
		else
		{
			speed_x = float(end_x - start_x)/float(s_database.tube_seqs[i_index].i_end - s_database.tube_seqs[i_index].i_start);
			speed_y = float(end_y - start_y)/float(s_database.tube_seqs[i_index].i_end - s_database.tube_seqs[i_index].i_start);
		}
		duration = float(s_database.tube_seqs[i_index].i_end - s_database.tube_seqs[i_index].i_start);
		if(fabs(speed_x) >= max_speed_x)
			max_speed_x = fabs(speed_x);
		if(fabs(speed_y) >= max_speed_y)
			max_speed_y = fabs(speed_y);

		Mat	M_i = Mat::zeros(s_database.i_height, s_database.i_width, CV_8UC1);
		int _start = s_database.tube_seqs[i_index].i_start;
		int _end = s_database.tube_seqs[i_index].i_end;
		for(int i_j_index = _start; i_j_index <= _end; i_j_index++)
		{
			simple_tube ptube_i = s_database.tube_seqs[i_index].tubes[i_j_index];
			CvRect rect_i = ptube_i.position;
			Mat M_i_i(M_i, rect_i);
			M_i_i.setTo(255);
		}
		int area_i = countNonZero(M_i);
		float area_rate = float(area_i)/float(_end - _start + 1);

		clustering_ele.at<float>(i_index, 0) = center_x;
		clustering_ele.at<float>(i_index, 1) = center_y;
		clustering_ele.at<float>(i_index, 2) = center_time;
		clustering_ele.at<float>(i_index, 3) = duration;
		clustering_ele.at<float>(i_index, 4) = speed_x;
		clustering_ele.at<float>(i_index, 5) = speed_y;
		clustering_ele.at<float>(i_index, 6) = area_i;
		clustering_ele.at<float>(i_index, 7) = area_rate;
	}

	//设定八方向与速度阈值，进行分类
	thres_speed_x = max_speed_x/100.0f;
	thres_speed_y = max_speed_y/100.0f;

	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		int center_index = -1;
		float tmp_speed_x = clustering_ele.at<float>(i_index, 4);
		float tmp_speed_y = clustering_ele.at<float>(i_index, 5);
		if(fabs(tmp_speed_x) <= thres_speed_x && fabs(tmp_speed_y) <= thres_speed_y)
		{
			center_index = 0;
		}
		else
		{
			float tmp_angle = atan2(tmp_speed_y, tmp_speed_x) + M_PI;
			center_index = int(floorf(tmp_angle/M_PI*180.0f))/45+1;
		}
		
		labels.at<int>(i_index, 0) = center_index;
	}

	
	FILE* fp6 = fopen("D:\\tube_clustering_element_refine.csv", "wt");  
	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		fprintf(fp6, "%d, %f,%f,%f,%f,%f,%f,%f,%f,%d\n", i_index, clustering_ele.at<float>(i_index, 0), clustering_ele.at<float>(i_index, 1),
			clustering_ele.at<float>(i_index, 2), clustering_ele.at<float>(i_index, 3), clustering_ele.at<float>(i_index, 4), 
			clustering_ele.at<float>(i_index, 5), clustering_ele.at<float>(i_index, 6), 
			clustering_ele.at<float>(i_index, 7), labels.at<int>(i_index, 0));  
	}
	fclose(fp6);
	//删除非活动与小tube

	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		hash_map<int, tube_seq>::iterator it;
		it = s_database.tube_seqs.find(i_index);
		if (it == s_database.tube_seqs.end())
		{
			continue;
		}

		if(clustering_ele.at<float>(i_index, 6) <= 1000 && clustering_ele.at<float>(i_index, 7) <= 10.0f )
		{
			s_database.tube_seqs.erase(i_index);
		}
		//非活动tube
		//小tube
		if(clustering_ele.at<float>(i_index, 3) <= thres_min_duration && labels.at<int>(i_index, 0) == 0)
		{
			s_database.tube_seqs.erase(i_index);
		}
	}

	//重新判断RT，NT，TST，TOT
	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		printf("refine tubes: %d\t\n", i_index);
		hash_map<int, tube_seq>::iterator it;
		it = s_database.tube_seqs.find(i_index);
		if (it == s_database.tube_seqs.end())
		{
			continue;
		}
		if(s_database.tube_seqs[i_index].tubes.size() <= thres_main_duration)
		{
			continue;
		}
		else
		{
			//清空
			s_database.tube_seqs[i_index].later_tube_seq.clear();
			s_database.tube_seqs[i_index].relate_tube_seq.clear();
			s_database.tube_seqs[i_index].neighbour_tube_seq.clear();
			s_database.tube_seqs[i_index].overlap_tube_seq.clear();
			//判断RT
			Mat M_i = Mat::zeros(s_database.i_height, s_database.i_width, CV_16UC1);
			for (hash_map<int, simple_tube>::iterator iiter = s_database.tube_seqs[i_index].tubes.begin(); iiter != s_database.tube_seqs[i_index].tubes.end(); iiter++)
			{
				simple_tube ptube_i = (simple_tube)(*iiter).second;
				CvRect rect_i = ptube_i.position;
				rect_i.x -= relate_thres_distance;
				rect_i.y -= relate_thres_distance;
				if(rect_i.x < 0)
					rect_i.x = 0;
				if(rect_i.y < 0)
					rect_i.y = 0;
				rect_i.width += 2*relate_thres_distance;
				rect_i.height += 2*relate_thres_distance;
				if(rect_i.x + rect_i.width >= s_database.i_width)
					rect_i.width = s_database.i_width - rect_i.x;
				if(rect_i.y + rect_i.height >= s_database.i_height)
					rect_i.height = s_database.i_height - rect_i.y;
				Mat M_i_i(M_i, rect_i);
				M_i_i.setTo(ptube_i.t_f);
			}
			//int area_i = countNonZero(M_i);

			for(int j_index = i_index + 1; j_index < s_database.i_total; j_index++)
			{
				if(s_database.tube_seqs[j_index].i_start < s_database.tube_seqs[i_index].i_start)
					continue;
				if(s_database.tube_seqs[j_index].i_start -  s_database.tube_seqs[j_index].i_end > relate_thres_frame)
					break;

				bool f_relate = false;
				bool f_neighbour = false;

				for (hash_map<int, simple_tube>::iterator jjter = s_database.tube_seqs[j_index].tubes.begin(); jjter != s_database.tube_seqs[j_index].tubes.end(); jjter++)
				{
					simple_tube ptube_j = (simple_tube)(*jjter).second;
					CvRect rect_j = ptube_j.position;
					int time_frame_j = ptube_j.t_f;
					CvPoint pt1 = CvPoint(rect_j.x, rect_j.y);
					CvPoint pt2 = CvPoint(rect_j.x+rect_j.width/2, rect_j.y+rect_j.height/2);
					CvPoint pt3 = CvPoint(rect_j.x+rect_j.width, rect_j.y+rect_j.height);
					int val1 = M_i.at<unsigned short int>(pt1.y, pt1.x);
					int val2 = M_i.at<unsigned short int>(pt2.y, pt2.x);
					int val3 = M_i.at<unsigned short int>(pt3.y, pt3.x);
					if(val1 > 0 && time_frame_j - val1 > 0 && time_frame_j - val1 <= relate_thres_frame)
					{
						s_database.tube_seqs[j_index].f_relate = true;
						s_database.tube_seqs[i_index].relate_tube_seq.push_back(j_index);
						break;
					}
					else if(val2 > 0 && time_frame_j - val2 > 0 && time_frame_j - val2 <= relate_thres_frame)
					{
						s_database.tube_seqs[j_index].f_relate = true;
						s_database.tube_seqs[i_index].relate_tube_seq.push_back(j_index);
						break;
					}
					else if(val3 > 0 && time_frame_j - val3 > 0 && time_frame_j - val3 <= relate_thres_frame)
					{
						s_database.tube_seqs[j_index].f_relate = true;
						s_database.tube_seqs[i_index].relate_tube_seq.push_back(j_index);
						break;
					}
				}

			}

			//判断NT
			M_i = Mat::zeros(s_database.i_height, s_database.i_width, CV_16UC1);
			for (hash_map<int, simple_tube>::iterator iiter = s_database.tube_seqs[i_index].tubes.begin(); iiter != s_database.tube_seqs[i_index].tubes.end(); iiter++)
			{
				simple_tube ptube_i = (simple_tube)(*iiter).second;
				CvRect rect_i = ptube_i.position;
				rect_i.x -= neighbour_thres_distance;
				rect_i.y -= neighbour_thres_distance;
				if(rect_i.x < 0)
					rect_i.x = 0;
				if(rect_i.y < 0)
					rect_i.y = 0;
				rect_i.width += 2*neighbour_thres_distance;
				rect_i.height += 2*neighbour_thres_distance;
				if(rect_i.x + rect_i.width >= s_database.i_width)
					rect_i.width = s_database.i_width - rect_i.x;
				if(rect_i.y + rect_i.height >= s_database.i_height)
					rect_i.height = s_database.i_height - rect_i.y;
				Mat M_i_i(M_i, rect_i);
				M_i_i.setTo(ptube_i.t_f);

			}
			//int area_i = countNonZero(M_i);

			for(int j_index = i_index + 1; j_index < s_database.i_total; j_index++)
			{
				if(s_database.tube_seqs[j_index].i_start < s_database.tube_seqs[i_index].i_start)
					continue;
				if(s_database.tube_seqs[j_index].i_start -  s_database.tube_seqs[j_index].i_end > relate_thres_frame)
					break;

				bool f_relate = false;
				bool f_neighbour = false;

				for (hash_map<int, simple_tube>::iterator jjter = s_database.tube_seqs[j_index].tubes.begin(); jjter != s_database.tube_seqs[j_index].tubes.end(); jjter++)
				{
					simple_tube ptube_j = (simple_tube)(*jjter).second;
					CvRect rect_j = ptube_j.position;
					int time_frame_j = ptube_j.t_f;
					CvPoint pt1 = CvPoint(rect_j.x, rect_j.y);
					CvPoint pt2 = CvPoint(rect_j.x+rect_j.width/2, rect_j.y+rect_j.height/2);
					CvPoint pt3 = CvPoint(rect_j.x+rect_j.width, rect_j.y+rect_j.height);
					int val1 = M_i.at<unsigned short int>(pt1.y, pt1.x);
					int val2 = M_i.at<unsigned short int>(pt2.y, pt2.x);
					int val3 = M_i.at<unsigned short int>(pt3.y, pt3.x);
					if(val1 > 0 && time_frame_j - val1 > 0 && time_frame_j - val1 <= relate_thres_frame)
					{
						s_database.tube_seqs[j_index].f_relate = true;
						s_database.tube_seqs[i_index].neighbour_tube_seq.push_back(j_index);
						break;
					}
					else if(val2 > 0 && time_frame_j - val2 > 0 && time_frame_j - val2 <= relate_thres_frame)
					{
						s_database.tube_seqs[j_index].f_relate = true;
						s_database.tube_seqs[i_index].neighbour_tube_seq.push_back(j_index);
						break;
					}
					else if(val3 > 0 && time_frame_j - val3 > 0 && time_frame_j - val3 <= relate_thres_frame)
					{
						s_database.tube_seqs[j_index].f_relate = true;
						s_database.tube_seqs[i_index].neighbour_tube_seq.push_back(j_index);
						break;
					}
				}

			}

			//判断TOT
			for(int j_index = i_index + 1; j_index < s_database.i_total; j_index++)
			{
				if(s_database.tube_seqs[j_index].i_start < s_database.tube_seqs[i_index].i_start)
					continue;
				if(s_database.tube_seqs[j_index].i_start >= s_database.tube_seqs[i_index].i_end)
					break;
				int i_duration_i = s_database.tube_seqs[i_index].i_end - s_database.tube_seqs[i_index].i_start;
				int i_duration_j = s_database.tube_seqs[j_index].i_end - s_database.tube_seqs[j_index].i_start;
				int i_duration = i_duration_i <= i_duration_j ? i_duration_i : i_duration_j;
				if(s_database.tube_seqs[i_index].i_end -  s_database.tube_seqs[j_index].i_start >= i_duration*to_thres_overlap && 
					(s_database.tube_seqs[j_index].i_end - s_database.tube_seqs[j_index].i_start) >= thres_min_duration)
				{
					s_database.tube_seqs[i_index].overlap_tube_seq.push_back(j_index);
				}
			}
			//判断TST
			M_i = Mat::zeros(s_database.i_height, s_database.i_width, CV_8UC1);
			Mat M_j = Mat::zeros(s_database.i_height, s_database.i_width, CV_8UC1);
			Mat M_i_xor_j = Mat::zeros(s_database.i_height, s_database.i_width, CV_8UC1);
			for (hash_map<int, simple_tube>::iterator iiter = s_database.tube_seqs[i_index].tubes.begin(); iiter != s_database.tube_seqs[i_index].tubes.end(); iiter++)
			{
				simple_tube ptube_i = (simple_tube)(*iiter).second;
				CvRect rect_i = ptube_i.position;
				Mat M_i_i(M_i, rect_i);
				M_i_i.setTo(1);

			}
			int area_i = countNonZero(M_i);

			for(int j_index = i_index + 1; j_index < s_database.i_total; j_index++)
			{
				if(s_database.tube_seqs[j_index].i_start < s_database.tube_seqs[i_index].i_start)
					continue;
				if(s_database.tube_seqs[j_index].i_start -  s_database.tube_seqs[i_index].i_end > ts_thres_frame)
					break;


				for (hash_map<int, simple_tube>::iterator jjter = s_database.tube_seqs[j_index].tubes.begin(); jjter != s_database.tube_seqs[j_index].tubes.end(); jjter++)
				{
					simple_tube ptube_j = (simple_tube)(*jjter).second;
					CvRect rect_j = ptube_j.position;
					Mat M_j_j(M_j, rect_j);
					M_j_j.setTo(1);
				}

				bitwise_xor(M_i,M_j,M_i_xor_j);
				int area_xor = countNonZero(M_i_xor_j);
				int area_j = countNonZero(M_j);
				int area_max = area_i >= area_j ? area_i : area_j;
				if(area_xor <= area_max &&(s_database.tube_seqs[j_index].i_end - s_database.tube_seqs[j_index].i_start) >= thres_min_duration)
				{
					s_database.tube_seqs[i_index].later_tube_seq.push_back(j_index);
				}

			}
		}
		
	}


	FILE* fp4 = fopen("D:\\tube_database_refine_step2.csv", "wt");  
	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		hash_map<int, tube_seq>::iterator it;
		it = s_database.tube_seqs.find(i_index);
		if (it == s_database.tube_seqs.end())
		{
			fprintf(fp4, "Index: %d, Eliminated", i_index);
			fprintf(fp4, ",\n");
			continue;
		}
		fprintf(fp4, "Index: %d, Duration: %d, Dependency: %d, RT: ", s_database.tube_seqs[i_index].i_index, s_database.tube_seqs[i_index].i_end-s_database.tube_seqs[i_index].i_start, s_database.tube_seqs[i_index].f_relate);
		for (list<int>::iterator iter = s_database.tube_seqs[i_index].relate_tube_seq.begin(); iter != s_database.tube_seqs[i_index].relate_tube_seq.end(); iter++)
		{
			fprintf(fp4, "%d ", (*iter));
		}
		fprintf(fp4, ",NT: ");
		for (list<int>::iterator iter = s_database.tube_seqs[i_index].neighbour_tube_seq.begin(); iter != s_database.tube_seqs[i_index].neighbour_tube_seq.end(); iter++)
		{
			fprintf(fp4, "%d ", (*iter));
		}
		fprintf(fp4, ",TOT: ");
		for (list<int>::iterator iter = s_database.tube_seqs[i_index].overlap_tube_seq.begin(); iter != s_database.tube_seqs[i_index].overlap_tube_seq.end(); iter++)
		{
			fprintf(fp4, "%d ", (*iter));
		}
		fprintf(fp4, ",TST: ");
		for (list<int>::iterator iter = s_database.tube_seqs[i_index].later_tube_seq.begin(); iter != s_database.tube_seqs[i_index].later_tube_seq.end(); iter++)
		{
			fprintf(fp4, "%d ", (*iter));
		}
		fprintf(fp4, ",\n");

		for (hash_map<int, simple_tube>::iterator iter = s_database.tube_seqs[i_index].tubes.begin(); iter != s_database.tube_seqs[i_index].tubes.end(); iter++)
		{
			simple_tube ptube = (simple_tube)(*iter).second;
			fprintf(fp4, "%d,%d,%d,%d,%d,%d\n", s_database.tube_seqs[i_index].i_index, ptube.position.x, ptube.position.y, ptube.position.width, ptube.position.height, ptube.t_f);   

		}
	}
	fclose(fp4);
}

/**
*Tube seq database重组优化
*/
void reArrangeDB(tube_database & s_database, const char * videoFilePath)
{
	hash_map<int, hash_map<int, simple_tube>> database_vec;
	hash_map<int, hash_map<int, simple_tube>>::iterator vec_it;
	for(int i_index = 0; i_index < s_database.i_total; i_index++)
	{
		hash_map<int, tube_seq>::iterator it;
		it = s_database.tube_seqs.find(i_index);
		if (it == s_database.tube_seqs.end())
		{
			continue;
		}
		for (hash_map<int, simple_tube>::iterator iter = s_database.tube_seqs[i_index].tubes.begin(); iter != s_database.tube_seqs[i_index].tubes.end(); iter++)
		{
			simple_tube ptube = (simple_tube)(*iter).second;
			int i_frame = ptube.t_f;
			vec_it = database_vec.find(i_frame);
			if(vec_it == database_vec.end())
			{//create a new map
				hash_map<int, simple_tube> tube_map;
				tube_map[i_index] = ptube;
				database_vec[i_frame] = tube_map;
			}
			else
			{
				database_vec[i_frame][i_index] = ptube;
			}
		}
	}

	CvCapture * capture = cvCaptureFromFile(videoFilePath);
	IplImage * frame = NULL;
	int i_frame = 0;
	while (frame = cvQueryFrame(capture)) {
		Mat frame_mat;
		frame_mat = cvarrToMat(frame,true); 

		vec_it = database_vec.find(i_frame);
		if(vec_it == database_vec.end())
		{
			;
		}
		else
		{
			for (hash_map<int, simple_tube>::iterator iter = database_vec[i_frame].begin(); iter != database_vec[i_frame].end(); iter++)
			{
				int i_index = (*iter).first;
				simple_tube ptube = (simple_tube)(*iter).second;
				char txt[255];
				sprintf(txt, "%d",i_index);
				putText(frame_mat,txt,Point(ptube.position.x, ptube.position.y),CV_FONT_HERSHEY_COMPLEX, 1, Colors[i_index%9]);

				rectangle(frame_mat,ptube.position,Colors[i_index%9],2);
			}
		}

		imshow("Result", frame_mat);
		cvWaitKey(20);

		i_frame++;
	}

	return;
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
	medianBlur(frame_mat, frame_mat, 5);
	knn->apply(frame_mat, fgmask_knn, -1);//-1默认
	threshold(fgmask_knn, fgmask_knn, 50, 255, CV_THRESH_BINARY);  

	CvMat fgMat_knn = fgmask_knn;
	cvDilate(&fgMat_knn, &fgMat_knn, 0, 3);
	cvErode(&fgMat_knn, &fgMat_knn, 0, 3);

	cvErode(&fgMat_knn, &fgMat_knn, 0, 3);
	cvDilate(&fgMat_knn, &fgMat_knn, 0, 3);

	fgmask_knn = cvarrToMat(&fgMat_knn, true);
	//knn->getBackgroundImage(bgimg_knn);

	edges=fgmask_knn.clone();
	Canny(fgmask_knn, edges, canny_thres_1, canny_thres_2, canny_apperture);

	imshow("KNN前景", fgmask_knn);
}

/**
*功能：	使用GMM方法与Canny算子实现背景与前景边缘提取
*参数：	frame	-	输入图像
*		knn		-	knn算子指针
*		edges	-	输出边缘
*/
void detectEdge_GMM(IplImage* frame, Ptr<BackgroundSubtractorMOG2>& gmm, Mat& edges, \
	int canny_thres_1, int canny_thres_2, int canny_apperture)
{
	if(frame == NULL || gmm == NULL)
	{
		printf("Error input in detectEdge function.\n");
		return;
	}
	Mat frame_mat, bgimg_gmm, fgmask_gmm;
	frame_mat = cvarrToMat(frame,true);  

	gmm->apply(frame_mat, fgmask_gmm, -1);//-1默认

	CvMat fgMat_gmm= fgmask_gmm;
	cvErode(&fgMat_gmm, &fgMat_gmm, 0, 2);
	cvDilate(&fgMat_gmm, &fgMat_gmm, 0, 2);


	cvDilate(&fgMat_gmm, &fgMat_gmm, 0, 5);
	cvErode(&fgMat_gmm, &fgMat_gmm, 0, 5);

	fgmask_gmm = cvarrToMat(&fgMat_gmm, true);
	//knn->getBackgroundImage(bgimg_gmm);

	edges=fgmask_gmm.clone();
	Canny(fgmask_gmm, edges, canny_thres_1, canny_thres_2, canny_apperture);

	imshow("gmm前景", fgmask_gmm);
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