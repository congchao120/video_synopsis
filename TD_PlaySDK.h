/* TD_PlaySDK.h: interface for the CLS_PlaySDK class. */
/**********************************************************************************/
#ifndef TD_PLAYSDK_H
#define TD_PLAYSDK_H
#include "PublicDef.h"

/**********************************************************************************/

#define DECLARE_FUNCTION(funcName)	p##funcName funcName
#define IMPLEMENT_FUNCTION(funcName)  p##funcName funcName = NULL
#define EXPORT_FUNCTION(hDll, funcName)	funcName = (p##funcName)::GetProcAddress(hDll, #funcName)

typedef int (*pTC_CreateSystem)(HWND _hWnd);
typedef int (*pTC_DeleteSystem)();

typedef int (*pTC_CreatePlayerFromFile)
(
 HWND  _hWnd,
 char* _pcFileName,
 int   _iDownloadBufSz /*= 0*/,
 int   _iFileTrueSz    /*= 0*/,
 int*  _piNowSz        /*= NULL*/,
 int   _iLastFrmNo     /*= 0*/,
 int*  _piCompleteFlag/* = NULL*/);
typedef int (*pTC_DeletePlayer)(int _iID);
typedef int (*pTC_DeletePlayerEx)(int _iID, int _iParam);

typedef void(*pTC_RegisterEventMsg)(HWND _hEventWnd, UINT _uiEventMsg/* = MSG_PLAYSDKM4*/);
typedef int (*pTC_RegisterNotifyPlayToEnd)(void* _PlayEndFun);
typedef int (*pTC_RegisterNotifyGetDecAV)(int _iID, void* _GetDecAVCbk, bool _blDisplay);

typedef int (*pTC_RegisterDrawFun)(int _iID,void* _DrawFun,LONG nUser);

typedef int (*pTC_SetPlayRect)(int _iID, RECT* _pDrawRect);
typedef int (*pTC_SetPlayRectEx)(int _iID, RECT* _pDrawRect, DWORD _dwMask);
typedef int (*pTC_Play)(int _iID);
typedef int (*pTC_Stop)(int _iID);
typedef int (*pTC_GoBegin)(int _iID);
typedef int (*pTC_GoEnd)(int _iID);
typedef int (*pTC_StepForward)(int _iID);
typedef int (*pTC_StepBackward)(int _iID);
typedef int (*pTC_FastForward)(int _iID, int _iSpeed);
typedef int (*pTC_SlowForward)(int _iID, int _iSpeed);
typedef int (*pTC_FastBackward)(int _iID);
typedef int (*pTC_PlayAudio)(int _iID);
typedef int (*pTC_StopAudio)(int _iID);
typedef int (*pTC_SetAudioVolume)(unsigned short _ustVolume);
typedef int (*pTC_GetAudioVolume)(unsigned short &_ustVolume);
typedef int (*pTC_SetAudioVolumeEx)(int _iID, unsigned short _ustVolume);
typedef int (*pTC_GetAudioVolumeEx)(int _iID, unsigned short &_ustVolume);
typedef int (*pTC_Seek)(int _iID, int _iFrameNo);
typedef int (*pTC_SeekEx)(int _iID, int _iFrameNo);

typedef int (*pTC_GetFrameRate)(int _iID);
typedef int (*pTC_GetPlayingFrameNum)(int _iID);
typedef int (*pTC_GetFrameCount)(int _iID);
typedef int (*pTC_GetBeginEnd)(int _iID, int* _piBegin, int* _piEnd);
typedef int (*pTC_GetFileName)(int _iID, char* _pcFileName);

typedef int (*pTC_CapturePic)(int _iID, unsigned char** _ppucYUV);
typedef int (*pTC_CaptureBmpPic)(int _iID, char* _pcSaveFile);

typedef int (*pTC_CreatePlayerFromStream)(HWND _hWnd, unsigned char* _pucFileHeadBuf, int _iHeadSize);
typedef int (*pTC_CreatePlayerFromStreamEx)(HWND _hWnd, unsigned char* _pucFileHeadBuf, int _iHeadSize, int _iStreamBufferSize);
typedef int (*pTC_PutStreamToPlayer)(int _iID, unsigned char* _pucStreamData, int _iSize);

typedef int (*pTC_StartMonitorCPU)();
typedef int (*pTC_StopMonitorCPU)();

typedef int (*pTC_GetVersion)(SDK_VERSION *_ver);

//for vod
typedef int (*pTC_CreatePlayerFromVoD)(HWND _hWnd, unsigned char* _pucVideoHeadBuf, int _iHeadSize);
typedef int (*pTC_SetVoDPlayerOver)(int _iID);
typedef int (*pTC_SetCleanVoDBuffer)(int _iID);

typedef int (*pTC_GetPlayTime)(int _iID);

typedef int (*pTC_Pause)(int _iID);

typedef int (*pTC_GetVideoParam)(int _iID, int*, int*, int*);

//------> add 20100714
typedef int (*pTC_DrawRect)(int _iID, RECT *_rcDraw, int _iCount);
typedef int (*pTC_SetEZoom)(int _iID, int _iZoomID, RECT _pRectInVideo);
typedef int (*pTC_ResetEZoom)(int _iID, int _iZoomID);
typedef int (*pTC_AddEZoom)(int _iID, HWND _hWnd, RECT* _rcZoom, int _iCount);
typedef int (*pTC_RemoveEZoom)(int _iID, int _iZoomID);
typedef int (*pTC_ResetPlayWnd)(int _iID, HWND _hWnd);

typedef int (__cdecl *pTC_SearchVCAInfo)( int _iID, int _iSearchRule, int _iFrameStart, void* _pParamBuf, int _iBufSize,  int _iContinuePlay, unsigned int* _puiFrameResult );
typedef int (__cdecl *pTC_GetBeginEndTimeStamp)(int _iID, unsigned int*, unsigned int*);

typedef int (__cdecl *pTC_SetVideoDecryptKey)(int _iID, void* _lpBuf, int _iBufSize);
typedef int (__cdecl *pTC_GetVideoDecryptKey)(int _iID, void* _lpBuf, int _iBufSize);

typedef int (__cdecl *pTC_RegisterCommonEventCallBack)(pfCBCommonEventNotify _pf);
typedef int (__cdecl *pTC_RegisterCommonEventCallBackEx)(pfCBCommonEventNotifyEx _pf, void*);

typedef int (__cdecl *pTC_SetModeRule)(int _iID, int _iModeRule);
typedef int (__cdecl *pTC_DrawPolygonOnLocalVideo)(int _iID, POINT* _pPointArray, int _iPointCount, int _iEnableArrow);
typedef int (__cdecl *pTC_RegisterPsPackCallBack)(PSPACK_NOTIFY _cbkPsPackNotify, void* _pContext);

typedef int (__cdecl *pTC_GetMarkInfo)(int _iID, PVOID _pData,int _iSize);
typedef int (__cdecl *pTC_SetMarkInfo)(int _iID,int _iFrameNO,int _iType);
typedef int (__cdecl *pTC_SeekMark)(int _iID,int _iFrameNO);
typedef int (__cdecl *pTC_StartCaptureFile)(int _iID, char* _cFileName,int _iRecFileType);
typedef int (__cdecl *pTC_StopCaptureFile)(int _iID);
typedef int (__cdecl *pTC_SetStreamBufferSize)(int _iID, int _iBufSize);
typedef int (__cdecl *pTC_SetFrameListBufferSize)(int _iID, int _iBufSize, int _iMaxFrameCount/* = -1*/);
typedef int (__cdecl *pTC_SetVerticalSync)(int _iID,int _iFlag);

typedef int (__cdecl *pTC_StartAudioCapture)(int _iID, int _iWavein,int _iWaveout);
typedef int (__cdecl *pTC_StopAudioCapture)(int _iID);
typedef int (__cdecl *pTC_RegisterNotifyAudioCapture)(void* _pvAudioCaptureFun, void* _pvContext);

/**********************************************************************************/
class CLS_PlaySDK
{
public:
	CLS_PlaySDK();
	virtual ~CLS_PlaySDK();


	int  LoadDll();
	void ReleaseDll();

private:
	HINSTANCE                    m_hHandle;

	DECLARE_FUNCTION(TC_CreateSystem);
	DECLARE_FUNCTION(TC_DeleteSystem);

	DECLARE_FUNCTION(TC_CreatePlayerFromFile);
	DECLARE_FUNCTION(TC_DeletePlayer);

	DECLARE_FUNCTION(TC_RegisterEventMsg);
	DECLARE_FUNCTION(TC_RegisterNotifyPlayToEnd);
	DECLARE_FUNCTION(TC_RegisterNotifyGetDecAV);
	DECLARE_FUNCTION(TC_RegisterDrawFun);

	DECLARE_FUNCTION(TC_SetPlayRect);
	DECLARE_FUNCTION(TC_SetPlayRectEx);
	DECLARE_FUNCTION(TC_Play);
	DECLARE_FUNCTION(TC_Stop);
	DECLARE_FUNCTION(TC_GoBegin);
	DECLARE_FUNCTION(TC_GoEnd);
	DECLARE_FUNCTION(TC_StepForward);
	DECLARE_FUNCTION(TC_StepBackward);
	DECLARE_FUNCTION(TC_FastForward);
	DECLARE_FUNCTION(TC_SlowForward);
	DECLARE_FUNCTION(TC_FastBackward);
	DECLARE_FUNCTION(TC_PlayAudio);
	DECLARE_FUNCTION(TC_StopAudio);
	DECLARE_FUNCTION(TC_SetAudioVolume);
	DECLARE_FUNCTION(TC_GetAudioVolume);
	DECLARE_FUNCTION(TC_SetAudioVolumeEx);
	DECLARE_FUNCTION(TC_GetAudioVolumeEx);
	DECLARE_FUNCTION(TC_Seek);
	DECLARE_FUNCTION(TC_SeekEx);

	DECLARE_FUNCTION(TC_GetFrameRate);
	DECLARE_FUNCTION(TC_GetPlayingFrameNum);
	DECLARE_FUNCTION(TC_GetFrameCount);
	DECLARE_FUNCTION(TC_GetBeginEnd);
	DECLARE_FUNCTION(TC_GetFileName);

	DECLARE_FUNCTION(TC_CapturePic);
	DECLARE_FUNCTION(TC_CaptureBmpPic);

	DECLARE_FUNCTION(TC_CreatePlayerFromStream);
	DECLARE_FUNCTION(TC_CreatePlayerFromStreamEx);
	DECLARE_FUNCTION(TC_PutStreamToPlayer);

	DECLARE_FUNCTION(TC_StartMonitorCPU);
	DECLARE_FUNCTION(TC_StopMonitorCPU);
	DECLARE_FUNCTION(TC_GetVersion);

	//for vod
	DECLARE_FUNCTION(TC_CreatePlayerFromVoD);
	DECLARE_FUNCTION(TC_SetVoDPlayerOver);
	DECLARE_FUNCTION(TC_SetCleanVoDBuffer);
	DECLARE_FUNCTION(TC_GetPlayTime);
	DECLARE_FUNCTION(TC_Pause);
	DECLARE_FUNCTION(TC_GetVideoParam);

	//--------------> add 20100714 hyq
	DECLARE_FUNCTION(TC_DrawRect);
	DECLARE_FUNCTION(TC_ResetPlayWnd);
	DECLARE_FUNCTION(TC_AddEZoom);
	DECLARE_FUNCTION(TC_SetEZoom);
	DECLARE_FUNCTION(TC_RemoveEZoom);
	DECLARE_FUNCTION(TC_ResetEZoom);

	DECLARE_FUNCTION(TC_SearchVCAInfo);
	DECLARE_FUNCTION(TC_GetBeginEndTimeStamp);

	DECLARE_FUNCTION(TC_SetVideoDecryptKey);
	DECLARE_FUNCTION(TC_GetVideoDecryptKey);
	DECLARE_FUNCTION(TC_RegisterCommonEventCallBack);
	DECLARE_FUNCTION(TC_RegisterCommonEventCallBackEx);

	DECLARE_FUNCTION(TC_SetModeRule);

	DECLARE_FUNCTION(TC_DrawPolygonOnLocalVideo);
	DECLARE_FUNCTION(TC_RegisterPsPackCallBack);
	DECLARE_FUNCTION(TC_DeletePlayerEx);

	DECLARE_FUNCTION(TC_GetMarkInfo);
	DECLARE_FUNCTION(TC_SetMarkInfo);
	DECLARE_FUNCTION(TC_SeekMark);
	DECLARE_FUNCTION(TC_StartCaptureFile);
	DECLARE_FUNCTION(TC_StopCaptureFile);

	DECLARE_FUNCTION(TC_SetStreamBufferSize);
	DECLARE_FUNCTION(TC_SetFrameListBufferSize);
	DECLARE_FUNCTION(TC_SetVerticalSync);

	DECLARE_FUNCTION(TC_StartAudioCapture);
	DECLARE_FUNCTION(TC_StopAudioCapture);
	DECLARE_FUNCTION(TC_RegisterNotifyAudioCapture);

public:
	int  CreateSystem(HWND _hWnd);
	int  DeleteSystem();

	int  CreatePlayerFromFile(HWND _hWnd, char* _pcFileName, int _iBufSize = 0);
	int  DeletePlayer(int _iID);
	int  DeletePlayerEx(int _iID, int _iParam);

	void RegisterEventMsg(HWND _hEventWnd, UINT _uiEventMsg = MSG_PLAYSDKM4);
	int  RegisterNotifyPlayToEnd(void* _PlayEndFun);
	int  RegisterNotifyGetDecAV(int _iID, void* _GetDecAVCbk, bool _blDisplay);

	int  RegisterDrawFun(int _iID,void* _DrawFun,LONG nUser);

	int  SetPlayRect(int _iID, RECT* _pDrawRect);
	int  SetPlayRectEx(int _iID, RECT* _pDrawRect, DWORD _dwMask);
	int  Play(int _iID);
	int  Stop(int _iID);

	int  GoBegin(int _iID);
	int  GoEnd(int _iID);
	int  StepForward(int _iID);
	int  StepBackward(int _iID);
	int  FastForward(int _iID, int _iSpeed);
	int  SlowForward(int _iID, int _iSpeed);
	int  FastBackward(int _iID);
	int  PlayAudio(int _iID);
	int  StopAudio(int _iID);
	int  SetAudioVolume(unsigned short _ustVolume);
	int  GetAudioVolume(unsigned short &_ustVolume);
	int  SetAudioVolumeEx(int _iID, unsigned short _ustVolume);
	int  GetAudioVolumeEx(int _iID, unsigned short &_ustVolume);

	int  Seek(int _iID, int _iFrameNo);
	int  SeekEx(int _iID, int _iFrameNo);

	int  GetFrameRate(int _iID);
	int  GetPlayingFrameNum(int _iID);
	int  GetFrameCount(int _iID);
	int  GetBeginEnd(int _iID, int* _piBegin, int* _piEnd);
	int  GetFileName(int _iID, char* _pcFileName);

	int  CapturePic(int _iID, unsigned char** _ppucYUV);
	int  CaptureBmpPic(int _iID, char* _pcSaveFile);

	int  CreatePlayerFromStream(HWND _hWnd, unsigned char* _pucFileHeadBuf, int _iHeadSize);
	int  CreatePlayerFromStreamEx(HWND _hWnd, unsigned char* _pucFileHeadBuf, int _iHeadSize, int _iStreamBufferSize);
	int  PutStreamToPlayer(int _iID, unsigned char* _pucStreamData, int _iSize);

	int StartMonitorCPU();
	int StopMonitorCPU();

	int GetVersion(SDK_VERSION *_ver);

	int CreatePlayerFromVOD(HWND _hWnd, unsigned char* _pucVideoHeadBuf, int _iHeadSize);
	int SetVoDPlayerOver(int _iID);
	int SetCleanVoDBuffer(int _iID);
	int GetPlayTime(int _iID);
	int Pause(int _iID);

	int GetVideoParam(int, int*, int*, int*);

	int DrawRect(int _iID, RECT *_rcDraw, int _iCount);
	int SetEZoom(int _iID, int _iZoomID, RECT _pRectInVideo);
	int ResetEZoom(int _iID, int _iZoomID);
	int AddEZoom(int _iID, HWND _hWnd, RECT* _rcZoom, int _iCount);
	int RemoveEZoom(int _iID, int _iZoomID);
	int ResetPlayWnd(int _iID, HWND _hWnd);

	int SearchVCAInfo( int _iID, int _iSearchRule, int _iFrameStart, void* _pParamBuf, int _iBufSize, int _iContinue, unsigned int &_iFrame );

	int GetBeginEndTimeStamp(int _iID, unsigned int *_puiBegin, unsigned int * _puiEnd);

	int SetVideoDecryptKey(int _iID, char* _lpBuf, int _iBufSize);
	int GetVideoDecryptKey(int _iID, char* _lpBuf, int _iBufSize);
	int RegisterCommonEventCallBack(pfCBCommonEventNotify _pf);
	int RegisterCommonEventCallBackEx(pfCBCommonEventNotifyEx _pf, void*);

	int SetModeRule(int _iID, int _iModeRule);
	int DrawPolygonOnLocalVideo(int _iID, POINT* _pPointArray, int _iPointCount, int _iEnableArrow);

	int RegisterPsPackCallBack(PSPACK_NOTIFY _pf, void* _lpUserData);

	int GetMarkInfo(int _iID, PVOID _pData,int _iSize);
	int SetMarkInfo(int _iID,int _iFrameNO,int _iType);
	int SeekMark(int _iID,int _iFrameNO);
	int StartCaptureFile(int _iID, char* _cFileName,int _iRecFileType);
	int StopCaptureFile(int _iID);
	int SetStreamBufferSize(int _iID, int _iBufSize);
	int SetFrameListBufferSize(int _iID, int _iBufSize);
	int SetVerticalSync(int _iID,int _iFlag);

	int StartAudioCapture(int _iID, int _iWavein,int _iWaveout);
	int StopAudioCapture(int _iID);
	int RegisterNotifyAudioCapture(void* _pvAudioCaptureFun, void* _pvContext);
};

#endif 
