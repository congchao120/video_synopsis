#ifndef PUBLICDEF_H
#define PUBLICDEF_H
#include <windows.h>

/*****************************************************************************/
#define  MAX_PLAYER_NUM  64
/*****************************************************************************/
#define  MSG_PLAYSDKM4  (WM_USER+0x01FF)	//回放器库消息的默认值

/************************************************************************
*	TC_PutStreamToPlayer返回值说明                                                                     
************************************************************************/
#define		RET_BUFFER_FULL					(-11)		//	缓冲区满，数据没有放入缓冲区
#define		RET_BUFFER_WILL_BE_FULL			(-18)		//	即将满，降低送入数据的频率
#define		RET_BUFFER_WILL_BE_EMPTY		(-19)		//	即将空，提高送入数据的频率
#define     RET_BUFFER_IS_OK                (-20)       //  正常可以放数据

/*回放器库消息的lParam参数取值列表*/
#define  LPARAM_FILE_PLAY_END			0	//	播放完毕
#define  LPARAM_FILE_SEARCH_END			1	//  寻找到文件末尾
#define  LPARAM_NEED_DECRYPT_KEY		2	//	需要解密密码
#define  LPARAM_DECRYPT_KEY_FAILED		3	//	解密密码错误
#define  LPARAM_DECRYPT_SUCCESS			4	//	解密成功
#define  LPARAM_STREAM_SEARCH_END		5   //  流缓冲区中没有数据了


/************************************************************************
* Decrypt frame return value                                                                     
************************************************************************/
#define DECRYPT_SUCCESS				0				//	解密成功
#define DECRYPT_INVALID_PARAM		(-1)			//	参数错误
#define DECRYPT_KEY_NOT_READY		(-2)			//	没有设定解密密钥
#define DECRYPT_FRAME_NOT_ENCRYPT	(-3)			//	帧不是加密帧
#define DECRYPT_FAILED				(-4)			//	帧解密失败
/*****************************************************************************/

#define MODERULE_AUTO_ADJUST_DRAW_YES					(0x01)		//	自适应不同分辨率数据--是
#define MODERULE_AUTO_ADJUST_DRAW_NO					(0x00)		//	自适应不同分辨率数据--否
#define MODERULE_AUTO_ADJUST_STREAM_PLAY_SPEED_YES		(0x11)		//	自适应调整流模式的播放速率--是
#define MODERULE_AUTO_ADJUST_STREAM_PLAY_SPEED_NO		(0x10)		//	自适应调整流模式的播放速率--否
#define MODERULE_STREAM_PLAY_HIGH_SMOOTH				(0x21)		//	设置为高流畅模式
#define MODERULE_STREAM_PLAY_LOW_DELAY					(0x20)		//	设置为低延时模式

/*****************************************************************************/
#define GET_USERDATA_INFO_MIN                            (0)                           
#define GET_USERDATA_INFO_TIME							 (GET_USERDATA_INFO_MIN )		//获得用户数据中的时间信息
#define GET_USERDATA_INFO_OSD							 (GET_USERDATA_INFO_MIN +1)		//获得用户数据中的OSD信息
#define GET_USERDATA_INFO_GPS                            (GET_USERDATA_INFO_MIN +2)		//获得用户数据中的GPS信息
#define GET_USERDATA_INFO_VCA                            (GET_USERDATA_INFO_MIN +3)		//获得用户数据中的VCA信息
#define GET_USERDATA_INFO_MAX                            (GET_USERDATA_INFO_MIN +4)	
/*****************************************************************************/
typedef struct SDK_VERSION
{    
	unsigned short    m_ulMajorVersion;
	unsigned short    m_ulMinorVersion;
	unsigned short    m_ulBuilder;
	char*             m_cVerInfo;					//	"X.X.X.XXX"
}*PSDK_VERSION;

//////////////////////////////////////////////////////////////////////////
/***********************  供上层应用程序导出使用，多屏显示 **************/
#define MAX_MONITOR_DESCLEN 512
typedef struct MONITOR_INFO					
{
	char        cDriverDesc[MAX_MONITOR_DESCLEN];
	char        cDriverName[MAX_MONITOR_DESCLEN];	
	RECT		rect;
}*PMONITOR_INFO;                                                                     

//////////////////////////////////////////////////////////////////
/*****************     录像文件中储存的智能分析信息   ***************/
#define MAX_SAVE_TARGET_NUM  16

#define VCAINFO_SEARCHRULE_MIN				0
#define VCAINFO_SEARCHRULE_FRAME			(VCAINFO_SEARCHRULE_MIN+0)
#define VCAINFO_SEARCHRULE_POLYGON			(VCAINFO_SEARCHRULE_MIN+1)
#define VCAINFO_SEARCHRULE_EVENT			(VCAINFO_SEARCHRULE_MIN+2)
#define VCAINFO_SEARCHRULE_MAX				(VCAINFO_SEARCHRULE_MIN+3)

#define VCA_MAX_POLYGON_POINT_NUM		32				//	每个多边形所允许的最多点个数
#define VCA_MAX_POLYGON_NUM				8				//	每条VCA规则所允许的最多的多边形个数

struct vca_TRect
{
	unsigned short usLeft;
	unsigned short usTop;
	unsigned short usRight;
	unsigned short usBottom;
};

struct vca_TTargetInfo
{
	vca_TRect		rect;
	unsigned short	usType;
	unsigned short	usVelocity;
	unsigned short	usDirection;
	unsigned short	usEvent;
};

struct vca_TTargetSet
{
	unsigned short		usCount;
	vca_TTargetInfo		targets[MAX_SAVE_TARGET_NUM]; 
};

struct vca_TTotalInfo
{
	unsigned short		usType;
	unsigned short		usLength;
	vca_TTargetSet		targetSet;
};


typedef struct DecAVInfo
{
    int iType;
    int iFormat;
    int iWidth;
    int iHeight;
    const unsigned char *pucData;
    int iDataLen;
}*PDecAVInfo;

typedef struct DecAVInfoEx
{
	DecAVInfo	avInfo;

	int			iOtherInfoSize;		//	根据OtherInfoSize判断后面的具体信息结构体, 1:TimeStamp(unsigned int), 2:FrameRate(int)
	void*		pOtherInfo;	

}*PDecAVInfoEx;

typedef struct PSPACK_INFO
{
	unsigned long nWidth;
	unsigned long nHeight;
	unsigned long nStamp;
	unsigned long nType;
	unsigned long nFrameRate;
	unsigned long nReserved;
}*PPSPACK_INFO;


/*****************************************************************************/
typedef void (__cdecl *pfCBPlayEnd)(int _iID);
typedef void (__cdecl *pfCBGetDecAV)(int _iID, const DecAVInfo* _pDecAVInfo);
typedef void (__cdecl *pfCBVideoEdit)(int _iNotifyCode);
typedef int (__cdecl *pfCBDrawFun)(long _lHandle,HDC _hDc,long _lUserData);

typedef int (__cdecl* pfCBCommonEventNotify)(int _iPlayerID, int _iEventMessage);
typedef int (__cdecl* pfCBCommonEventNotifyEx)(int _iPlayerID, int _iEventMessage, void* _lpUserData);
typedef int (__cdecl* PSPACK_NOTIFY)(int _iPlayerID, unsigned char* _ucData, int _iSize, PSPACK_INFO* _pPsPackInfo, void* _pContext);
/********************************************************************************/
#endif