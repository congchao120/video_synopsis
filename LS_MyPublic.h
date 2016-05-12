#pragma once 

#include "TD_PlaySDK.h"
#include "Global.h"
#include <map>
#include <stdio.h>
#include <time.h>
#include <share.h>
#include <string>

#define MAX_MOTION_DETECTION_ROW                18
#define MAX_MOTION_DETECTION_COL                22

#define WM_EVENT_MESSAGE        WM_USER+5678
#define MSG_FILE_PLAY_END       0
#define MSG_NEED_DECRYPT_KEY    1
#define MSG_DECRYPT_KEY_FAILED  2
#define MSG_DECRYPT_SUCCESS     3
#define MSG_FILE_SEARCH_END     4
#define MSG_STREAM_SEARCH_END   5

#define MAX_WINDOWNUM           16
#define MAX_STREAM_SPEED	15000 //CYY ADD   *modify by yangerxing 2014-02-13*
#define MIN_SPEED               20    //yangerxing  2014-02-13
#define OPENFILE_NORMAL			100
#define OPENFILE_VOD			101
#define CONST_TIMER_GET_PLAYTIME 1002
#define CONST_INPUT_STREAM	     1003
#define CONST_MAIN_TIMER		1004



#define WM_ITEMLBUTTON_DOWN WM_USER+1000	//左键按下的消息
#define WM_ITEMLBUTTON_UP WM_USER+1001	//左键按下的消息
#define WM_ITEMLBUTTON_MOVE WM_USER+1002	//左键按下的消息

#define WM_ITEMLBUTTON_DBCLICK WM_USER+1004	//左键按下的消息
//#define _USE_PSPACK_NOTIFY_
enum
{
	SPEED_ADD = 0,
	SPEED_DEC ,
	SPEED_SET
};

struct vca_TPoint
{
	int iX;
	int	iY;
};					//	sizeof = 2*4 = 8

struct vca_TLine
{
	vca_TPoint 	stStart;
	vca_TPoint 	stEnd;
};						//	sizeof = 2*8 = 16

struct vca_TTripwire
{
	int					iValid;					//	是否有效,本来由规则中的iEventID判断哪个事件有效即可，
	//	但如果所有规则都不生效时iEventID总会指向一个事件，上层无法判断是否真的有效，只能加一个事件有效字段
	int					iTargetTypeCheck;		//	目标类型限制
	int					iMinDistance;			//	最小距离,目标运动距离必须大于此阈值
	int					iMinTime;				//	最短时间,目标运动距离必须大于此阈值	
	int					iType;					//	表示穿越类型
	int					iDirection;				//	单绊线禁止方向角度
	vca_TLine			stLine;					//	绊线坐标
}; // 单绊线参数

struct vca_TPolygon
{
	int 		iPointNum;
	vca_TPoint 	stPoints[VCA_MAX_POLYGON_POINT_NUM];   
};	

struct vca_TPerimeter
{
	int					iValid;					//	是否有效,本来由规则中的iEventID判断哪个事件有效即可，
	//	但如果所有规则都不生效时iEventID总会指向一个事件，上层无法判断是否真的有效，只能加一个事件有效字段
	int					iTargetTypeCheck;		//	区分人车
	int					iMode;					//	监测模式
	int 				iMinDistance;			//	最小距离
	int 				iMinTime;				//	最短时间		
	int					iType;					//	是否做方向限制
	int 				iDirection;				//	禁止方向角度(单位: 角度)
	vca_TPolygon		stRegion;				//	区域范围	
};		// 周界参数

void __cdecl trace_r(const char* _pszFormat, ...);

void __cdecl trace_d(const char* _pszFormat, ...);

int __cdecl CheckFileSize(const char* _pszFileName);

std::string __cdecl IntToStr( int _iValue );