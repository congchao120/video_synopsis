
#include "stdafx.h"
#include "synopsis.h"
#include <io.h>
/**
 *背景建模
 */
void bgModeling(const char * videoFilePath, const char* outFilePath, const int frame_num_used, IplImage ** bgImg, \
	int& fps, int& frame_number, CvSize& size, int& code, const int size1, const int size2, const int sigma1, const int sigma2){
	//声明
	IplImage * frame = NULL;
	CvMat * frameMat = NULL;
	CvMat * bgMat = NULL;
	CvCapture* pCapture = NULL;
	IplImage * bgColorImg = NULL;
	int frame_no = 0;

	pCapture = cvCaptureFromFile(videoFilePath);//自己选取一段avi视频
	if(!pCapture){
		printf("Unable to open video file for background modeling!\n");
		return;
	}

	if(*bgImg != NULL){//非空需先清空*bgImg指向的内存
		cvReleaseImage(bgImg);
	}

	fps			= (int)cvGetCaptureProperty(pCapture, CV_CAP_PROP_FPS);
	frame_number = (int)cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_COUNT);
	code  = (int)cvGetCaptureProperty(pCapture, CV_CAP_PROP_FOURCC);
	size.width = (int)cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_WIDTH);
	size.height = (int)cvGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_HEIGHT);

	printf("Background Modeling...\n");
	//CvVideoWriter * writer = cvCreateVideoWriter(outFilePath, CV_FOURCC('h', '2', '6', '4'), fps, size, 1);

	//逐帧读取视频
	while(frame_no < frame_num_used){
		


		frame = cvQueryFrame(pCapture);
		frame_no += 1;
		//cvWriteFrame(writer, frame); //写帧
		if(frame_no == 1){
			//初始化
			bgColorImg = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, frame->nChannels);
			cvCopy(frame, bgColorImg);
			frameMat = cvCreateMat(frame->height, frame->width, CV_32FC3);
			bgMat = cvCreateMat((bgColorImg)->height, (bgColorImg)->width, CV_32FC3);
			cvConvert(frame, frameMat);
			cvConvert(bgColorImg, bgMat);
			continue;
		}
		//视频帧IplImage转CvMat
		cvConvert(frame, frameMat);
		//高斯滤波先，以平滑图像
		cvSmooth(frame, frame, CV_GAUSSIAN, size1, size2, sigma1, sigma2);
		//滑动平均更新背景(求平均)
		cvRunningAvg(frameMat, bgMat, (double)1/frame_num_used);

	}

	cvConvert(bgMat, bgColorImg);
	printf("Background Model has been achieved!\n");
	/*
	*bgImg = cvCreateImage(size, IPL_DEPTH_8U, 1);
	
	cvCvtColor(bgColorImg, *bgImg, CV_BGR2GRAY); //帧图像彩色转灰度
	*/
	*bgImg = bgColorImg;
	//释放内存
	cvReleaseCapture(&pCapture);
	cvReleaseMat(&frameMat);
	cvReleaseMat(&bgMat);

	//cvReleaseVideoWriter(&writer);
}


/**
 *基本数据单元构造函数
 */
tube::tube(CvRect rect, int t, IplImage * img):position(rect), t_sec(t){ 
	target = cvCreateImage(cvSize(img->width, img->height), img->depth, img->nChannels);
	cvCopy(img, target);
}

/**
 *基本数据单元析构函数
 */
tube::~tube(){
	cvReleaseImage(&target);
}


/**
 *提取前景图像 - 二值图像
 */
void getFgImage(const IplImage * frame, IplImage * fgImg, const IplImage * bgImg, const int threshold){
	if(frame == NULL || fgImg == NULL || bgImg == NULL){
		printf("Fail: There exists NULL input. Fail to get Foreground Image!\n");
		return;
	}

	if(frame->nChannels != 3 || fgImg->nChannels != 3 || bgImg->nChannels != 3){
		printf("Fail: All input image should be color!\nframe channel:%d\nfgImg channel:%d\nbgImg channel:%d\n",\
			frame->nChannels, fgImg->nChannels, bgImg->nChannels);
		return;
	}

	CvMat * frameMat = cvCreateMat(frame->height, frame->width, CV_32FC3);
	CvMat * fgMat	 = cvCreateMat(fgImg->height, fgImg->width, CV_32FC3);
	CvMat * bgMat	 = cvCreateMat(bgImg->height, bgImg->width, CV_32FC3);

	cvConvert(frame, frameMat);
	cvConvert(fgImg, fgMat);
	cvConvert(bgImg, bgMat);

	cvSmooth(frameMat, frameMat, CV_GAUSSIAN, 5, 5, 4, 4); //高斯滤波先，以平滑图像

	cvAbsDiff(frameMat, bgMat, fgMat); //当前帧跟背景图相减(求背景差并取绝对值)

	cvThreshold(fgMat, fgImg, threshold, 255, CV_THRESH_BINARY); //二值化前景图(这里采用特定阈值进行二值化)

	//进行形态学滤波，去掉噪音
	cvErode(fgImg, fgImg, 0, 1);
	cvDilate(fgImg, fgImg, 0, 1);

	//释放矩阵内存
	cvReleaseMat(&frameMat);
	cvReleaseMat(&fgMat);
	cvReleaseMat(&bgMat);
}


/**
 *判断两矩形是否重叠
 */
bool isOverlap(const CvRect & a, const CvRect & b){
	const CvRect * l_rect = &a,
				 * r_rect = &b;
	if(a.x > b.x){
		const CvRect * tmp = l_rect;
		l_rect = r_rect;
		r_rect = tmp;
	}

	if(l_rect->width < r_rect->x - l_rect->x)
		return false;
	else if(l_rect->y <= r_rect->y && l_rect->height >= r_rect->y - l_rect->y)
		return true;
	else if(l_rect->y > r_rect->y && r_rect->height >= l_rect->y - r_rect->y)
		return true;
	else
		return false;
}


/**
 *前景位置矩形块合并 - 减少单元数
 */
void mergeRects(list<CvRect> & rects){
	int x = 0, y = 0, width = 0, height = 0;//临时变量
	for(list<CvRect>::iterator i = rects.begin(); i != rects.end(); ){
		bool merged = false;//多引入一个变量判断i是否被merge非常有用！
		list<CvRect>::iterator j = i;
		for(j++; j != rects.end(); j++){
			if(isOverlap(*i, *j)){
				if(i->x < j->x){
					x = i->x;
					width = max(j->x - i->x + j->width, i->width);
				}
				else{
					x = j->x;
					width = max(i->x - j->x + i->width, j->width);
				}
				
				if(i->y < j->y){
					y = i->y;
					height = max(j->y - i->y + j->height, i->height);
				}
				else{
					y = j->y;
					height = max(i->y - j->y + i->height, j->height);
				}

				//合并
				j->x	  = x;
				j->y	  = y;
				j->width  = width;
				j->height = height;

				i = rects.erase(i);//删除被合并项。注意：删除前者（i）更新后者（j）！
				merged = true;
			}
		}
		if(!merged)
			i++;
	}
}


/**
 *判断是否属于同一跟踪目标
 */
bool isSameObj(const CvRect & a, const CvRect & b, const float category_factor){
	const CvRect * l_rect = &a,
				 * r_rect = &b;
	if(a.x > b.x){
		const CvRect * tmp = l_rect;
		l_rect = r_rect;
		r_rect = tmp;
	}

	int area = 0;//记录重叠区域面积
	if(l_rect->width <= r_rect->x - l_rect->x)
		return false;
	else if(l_rect->y <= r_rect->y && l_rect->height > r_rect->y - l_rect->y){
		area = (l_rect->x + l_rect->width - r_rect->x) * (l_rect->y + l_rect->height - r_rect->y);
		if(area > category_factor * a.width * a.height || area > category_factor * b.width * b.height)
			return true;
		else 
			return false;
	}
	else if(l_rect->y >= r_rect->y && r_rect->height > l_rect->y - r_rect->y){
		area = (l_rect->x + l_rect->width - r_rect->x) * (r_rect->y + r_rect->height - l_rect->y);
		if(area > category_factor * a.width * a.height || area > category_factor * b.width * b.height)
			return true;
		else 
			return false;
	}
	else
		return false;
}


/**
 *前景跟踪并建立数据库结构 - Kernel
 */
void buildTrackDB(const char * videoFilePath, const IplImage * bgImg, list< list<tube *> > & database,\
				const int threshold, const int min_area, const int obj_num, const float extend_factor,\
				const float category_factor, const bool save_mode){
	if(bgImg == NULL){
		printf("Fail: NULL background image!\n");
		return;
	}

	if(bgImg->nChannels != 3){
		printf("Fail: Background image should be grayscale!\n");
		return;
	}

	if(!database.empty()){
		printf("Fail: Database is not empty!\n");
		return;
	}

	if(extend_factor < 0 || extend_factor > 1 || category_factor < 0 || category_factor > 1){
		printf("Fail: Invalid use of params!\n0 <= extend_factor <= 1\n0 <= category_factor <= 1\n");
		return;
	}

	CvCapture * capture = cvCaptureFromFile(videoFilePath);
	if(!capture){
		printf("Fail: Unable to open video file!\n");
		return;
	}
	int fps			= (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FPS),
		frame_count = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT),
		frame_id	= 0;
	CvSize size		= cvSize( (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH),
							(int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT) );	
	
	IplImage * frame	  = NULL,
			 * frame_gray = cvCreateImage(size, IPL_DEPTH_8U, 1),
			 * fgImg	  = cvCreateImage(cvSize(bgImg->width, bgImg->height), bgImg->depth, bgImg->nChannels);
	
	//以下变量用于轮廓查找函数
	CvMemStorage * storage = cvCreateMemStorage();
	CvSeq * first_contour  = NULL;
	
	int find_num		   = 0,
		extend_w		   = 0,
		extend_h		   = 0;
	list<CvRect> rects; //存放找到的前景块位置
	char time[10]; //存放时间
	int m = 0, s = 0; //分、秒

	printf("Build track database...\n");
	while(frame = cvQueryFrame(capture)){
		IplImage * tmp		   = cvCreateImage(size, IPL_DEPTH_8U, 1);
		//cvCvtColor(frame, frame_gray, CV_BGR2GRAY); //帧图像彩色转灰度
		getFgImage(frame, fgImg, bgImg, threshold); //帧差法获取前景图像
		//cvShowImage("fg", fgImg);
		//cvWaitKey();
		cvCvtColor(fgImg, tmp, CV_BGR2GRAY); //帧图像彩色转灰度
		//cvCopy(fgImg, tmp);
		find_num = cvFindContours(tmp, storage, &first_contour, sizeof(CvContour), CV_RETR_CCOMP); //检测前景轮廓
		
		/**!!!Be careful with the param below!!!**/
		if(find_num > (obj_num * 10) || first_contour == NULL || frame == NULL){
			cvClearMemStorage(storage);
			cvReleaseImage(&tmp);
			continue;
		}
		
		printf("frame:%d\tfind contour num:%d\n", frame_id, find_num);
		//contour analysis
		for(CvSeq* c = first_contour; c != NULL; c = c->h_next){
			int left = 0x7fffffff, right = 0, top = 0x7fffffff, bottom = 0;
			int contourWidth  = 0,
				contourHeight = 0;
			for(int j = 0; j < c->total; j++){ 
				CvPoint * pt = (CvPoint *)cvGetSeqElem(c, j); 
				if(pt->x < left)
					left = pt->x;
				if(pt->x > right)
					right = pt->x;
				if(pt->y < top)
					top = pt->y;
				if(pt->y >bottom)
					bottom = pt->y;
			}
			contourWidth  = right - left + 1;
			contourHeight = bottom - top + 1;
			if(contourWidth * contourHeight < min_area)
				continue;
			extend_w = extend_factor * contourWidth;
			extend_h = extend_factor * contourHeight;
			left	 = (left >= extend_w ? left - extend_w : 0);
			top		 = (top >= extend_h ? top - extend_h : 0);
			right	 = ((right + extend_w) < size.width ? right + extend_w : size.width - 1);
			bottom	 = ((bottom + extend_h) < size.height ? bottom + extend_h : size.height - 1);
			rects.push_back(cvRect(left, top, right - left + 1, bottom - top + 1));
		}

		mergeRects(rects); //merge rects进一步筛选

		for(list<CvRect>::iterator iter = rects.begin(); iter != rects.end(); iter++){
			//cvRectangle(frame, cvPoint(iter->x, iter->y), cvPoint(iter->x + iter->width, iter->y + iter->height),\
						cvScalar(0,255,0), 1);//在frame中标示出最后保存的方框区域
			m = (frame_id / fps) / 60;
			s = (frame_id / fps) % 60;
			sprintf(time, "%02d:%02d", m, s);
			cvPutText(frame, time, cvPoint(iter->x + iter->width/3, iter->y + iter->height/3),\
						&cvFont(1, 1), cvScalar(0, 0, 255));//在frame中标记时间
			cvSetImageROI(frame, *iter);
			IplImage * target = cvCreateImage(cvSize((*iter).width, (*iter).height), frame->depth, frame->nChannels);
			cvCopy(frame, target);

			bool added = false;//判断通过下面的循环是否将tube加入到database
			for(list< list<tube *> >::iterator i = database.begin(); i != database.end(); i++){
				list<tube *>::iterator j = i->end();//如果 *i 为空则应该到 database 末端了，循环应已终止
				j--;//找到list<tube *>最后一个元素，及该目标的最新跟踪数据
				if(isSameObj(*iter, (*j)->position, category_factor)){//判断是同一跟踪目标
					i->push_back(new tube(*iter, frame_id / fps, target));
					added = true;
					break;
				}
			}
			if(!added)//扫描现有database未发现匹配项则新建跟踪目标链表
				database.push_back(list<tube *>(1, new tube(*iter, frame_id / fps, target)));

			cvResetImageROI(frame);
			cvReleaseImage(&target);
		}

		//The following 3 lines are for debuging, comment them when debug is done!
		//cvShowImage("frame", frame);
		//cvShowImage("fgImg", fgImg);
		//cvWaitKey(40);

		frame_id += 1;
		rects.clear();
		cvClearMemStorage(storage);
		cvReleaseImage(&tmp);
	}
	printf("Track DB has been saved! Total frame num:%d\tValid frame:%d\n", frame_count, frame_id);

	if(save_mode){//如果设置了保存模式，则将database中图像保存于“./DB”文件夹下
		if(access("./DB", 0) == 0)//判断是否存在文件夹“./DB”, 如果存在，先强制删除（避免原来有记录）
			system("rd /s /q DB");
		system("md DB");//创建文件夹“./DB"
		int i = 0, j = 0;
		char filename[50];// "./DB/obj_iii_seq_jjjj_px%d_py%d_t%d.jpg"
		for(list< list<tube *> >::iterator iter1 = database.begin(); iter1 != database.end(); iter1++, i++){
			j = 0;
			for(list<tube *>::iterator iter2 = iter1->begin(); iter2 != iter1->end(); iter2++, j++){
				sprintf(filename, "./DB/obj_%03d_seq_%04d_px%d_py%d_t%d.jpg", i, j, (*iter2)->position.x, (*iter2)->position.y, (*iter2)->t_sec);
				cvSaveImage(filename, (*iter2)->target);
				printf("File %s saved.\n", filename);
			}
		}
	}

	//释放内存
	cvReleaseMemStorage(&storage);

	cvReleaseImage(&frame_gray);
}


/**
 *融合跟踪数据库，输出视频摘要，同时释放Database内存
 */
void mergeDB(list< list<tube *> > & database, const char * videoFilePath, const int fps, const CvSize size, const IplImage * bgImg){
	if(database.empty()){
		printf("Fail: Database is empty, cannot merge!\n");
		return;
	}

	if(videoFilePath == NULL){
		printf("Fail: NULL output file path/name!\n");
		return;
	}

	CvVideoWriter * writer = cvCreateVideoWriter(videoFilePath, CV_FOURCC('h', '2', '6', '4'), fps, size, 1);
	if(writer == NULL){
		printf("Fail: Invalid video file path/name, cannot build a video writer!\n");
		return;
	}

	IplImage * frame = cvCreateImage(size, IPL_DEPTH_8U, 3); //输出帧
	tube * tmp = NULL; //临时tube指针
	vector<CvRect> rects; //保存每帧被填充的位置
	int frame_id = 0;

	printf("Merging track database...\n");
	while(!database.empty()){

		cvCopy(bgImg, frame); //每次开始用背景图片刷新输出帧

		for(list< list<tube *> >::iterator i = database.begin(); i != database.end();){ //遍历级联链表每个子链表首项
			if(i->empty()){
			    i = database.erase(i);
				continue;
			}

			bool canAdd = true;
			tmp = *(i->begin());
			
			for(size_t t = 0; t != rects.size(); t++){ //检查是否与已经填充的有重叠
				if(isOverlap(rects[t], tmp->position)){
					canAdd = false;
					break;
				}
			}
			
			if(canAdd){ //如果可以加入则将tube写入输出帧
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


/**
 *释放Database内存函数 - 辅助
 */
void freeDB(list< list<tube *> > & database){
	for(list< list<tube *> >::iterator iter1 = database.begin(); iter1 != database.end(); iter1++)
		for(list<tube *>::iterator iter2 = iter1->begin(); iter2 != iter1->end();){
			delete *iter2;
			iter2 = (*iter1).erase(iter2);
		}
}