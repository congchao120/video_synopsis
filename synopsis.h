#ifndef _SYNOPSIS_H
#define _SYNOPSIS_H


#ifndef _DLL_API  
#define _DLL_API _declspec(dllexport)  
#else  
#define _DLL_API _declspec(dllimport)  
#endif  

#include <cv.h>
#include <highgui.h>
#include <opencv2/video/background_segm.hpp>
#include <list>
#include <stdio.h>
#include <hash_map> 

using namespace cv;
using namespace std;

struct DecAVInfo;

/**
*简单tube
*/
struct simple_tube {
	//functions
	simple_tube(CvRect rect, int f); //构造函数
	~simple_tube(); //析构函数

			 //variables
	CvRect position; //团块在源图中位置
	int t_f; //所在帧

};

/**
*tube序列
*/
struct tube_seq {
	//functions
	tube_seq(); //构造函数
	tube_seq(list<simple_tube> _seq, int _iframe, int _index); //构造函数
	~tube_seq(); //析构函数

			 //variables
	list<simple_tube> tubes; //本序列所有simple_tube
	list<int> relate_tube_seq; //相关tube_seq表，相关tube_seq指所有由本序列分裂的、组合的、冲突的tube_seq
	list<int> neighbour_tube_seq; //空间邻近tube_seq表
	list<int> later_tube_seq; //时间顺序tube_seq表（只统计之后）
	list<int> overlap_tube_seq; //时间重叠tube_seq表

	int i_start;//开始帧
	int i_end;//结束帧
	bool f_relate;
	int i_index;//编号
};

/**
*tube数据库
*/
struct tube_database {
	//functions
	tube_database(); //构造函数
	~tube_database(); //析构函数

	//variables
	hash_map<int, tube_seq> tubes;
	int i_width;//图像宽
	int i_height;//图像高
	int i_total;//所有（曾经）tubes
};


/**
*功能：	使用KNN方法实现的检测并跟踪运动前景团块tube，将tubes保存按级联链表结构存储，可以选择保存数据结果
*参数：	videoFilePath	-	视频文件路径 + 文件名
*		database		-	级联链表数据的引用（输出）
*		threshold		-	前景和背景分割的阈值
*		min_area		-	检测到的连通区域最小允许面积
*		obj_num			-	估算视频中待检测目标的个数
*		extend_factor	-	检测框扩展因子（四周扩展 extend_factor * width）
*		category_factor	-	重合度因子（重合度超过因子数则归类为同一跟踪目标）
*		save_mode		-	是否保存跟踪结果图像(保存结果需要在当前路径建立文件夹“DB”)
*       skip_begin_frame - 跳过开头几帧
*/
_DLL_API tube_database buildTrackDB_KNN(char * videoFilePath, \
	const int min_area = 16, const int obj_num = 30, const float extend_factor = 0.2, const float category_factor = 0.5, \
	const bool save_mode = false, const int skip_begin_frame = 30);

/**
*功能：	使用KNN方法与Canny算子实现背景与前景边缘提取
*参数：	frame	-	输入图像
*		knn		-	knn算子指针
*		edges	-	输出边缘
*/
_DLL_API void detectEdge_KNN(IplImage* frame, Ptr<BackgroundSubtractorKNN>& knn, Mat& edges, \
	int canny_thres_1 = 20, int canny_thres_2 = 190, int canny_apperture = 3);

/**
*  提炼tube数据库，判断独立tube，相关tubes，邻近tubes，顺序tubes，时间重叠tubes
*  首先判断相关与否
*  随后筛选不相关tube，5帧内删除
*  判断邻近，类似相关，25*25*1判断
*  判断顺序
*  判断时间重叠
*/
_DLL_API void refineDB(tube_database & s_database, \
		const int thres_main_duration = 60, const int relate_thres_distance = 2, const int relate_thres_frame = 2,\
		const int thres_min_duration = 30, const int neighbour_thres_distance = 10, const int ts_thres_frame = 150, \
		const float to_thres_overlap = 0.5);


/**
*功能：	融合跟踪数据库形成视频摘要
*参数：	database		-	级联链表数据的引用（输入）
*		videoFilePath	-	输出摘要视频文件路径 + 文件名（例如“E:/synopsis.avi”）
*/
//_DLL_API void mergeDB(list< list<tube *> > & database, const char * videoFilePath, const int fps, const CvSize size, const IplImage * bgImg);


/**
*功能： 释放database占据的内存空间
*参数： database	-	级联链表数据的引用
*/
//_DLL_API void freeDB(list< list<tube *> > & database);

static void  __cdecl GetDecAVCbk(int _iID, const DecAVInfo *_pDecAVInfo);


bool isOverlap(const CvRect & a, const CvRect & b);

bool isOverlap(CvRect a, CvRect b, int thres);

#endif