/* TD_PlaySDK.cpp: implementation of the CLS_PlaySDK class. */
/*****************************************************************************/
#include "TD_PlaySDK.h"
/*****************************************************************************/
/*-----------------------------------*/
/* Construction/Destruction */
/*-----------------------------------*/
CLS_PlaySDK::CLS_PlaySDK()
{
    m_hHandle                  = NULL;

    TC_CreateSystem            = NULL;
    TC_DeleteSystem            = NULL;

    TC_CreatePlayerFromFile    = NULL;
    TC_DeletePlayer            = NULL;
    TC_DeletePlayerEx          = NULL;

    TC_RegisterEventMsg        = NULL;
    TC_RegisterNotifyPlayToEnd = NULL;
    TC_RegisterNotifyGetDecAV  = NULL;

    //--
    TC_RegisterDrawFun         = NULL;

    TC_SetPlayRect             = NULL;
    TC_SetPlayRectEx           = NULL;
    TC_Play                    = NULL;
    TC_Stop                    = NULL;
    TC_GoBegin                 = NULL;
    TC_GoEnd                   = NULL;
    TC_StepForward             = NULL;
    TC_StepBackward            = NULL;
    TC_FastForward             = NULL;
    TC_SlowForward             = NULL;
    TC_FastBackward            = NULL;
    TC_PlayAudio               = NULL;
    TC_StopAudio               = NULL;
    TC_SetAudioVolume          = NULL;
    TC_GetAudioVolume          = NULL;
    TC_SetAudioVolumeEx        = NULL;
    TC_GetAudioVolumeEx        = NULL;
    TC_Seek                    = NULL;
    TC_SeekEx                  = NULL;

    TC_GetFrameRate            = NULL;
    TC_GetPlayingFrameNum      = NULL;
    TC_GetFrameCount           = NULL;
    TC_GetBeginEnd             = NULL;
    TC_GetFileName             = NULL;

    TC_CapturePic              = NULL;
    TC_CaptureBmpPic           = NULL;

    TC_CreatePlayerFromStream  = NULL;
    TC_CreatePlayerFromStreamEx = NULL;
    TC_PutStreamToPlayer       = NULL;

    TC_StartMonitorCPU         = NULL;
    TC_StopMonitorCPU          = NULL;

    TC_GetVersion              = NULL;


    //for vod
    TC_CreatePlayerFromVoD     = NULL;
    TC_SetVoDPlayerOver        = NULL;
    TC_SetCleanVoDBuffer       = NULL;

    TC_GetPlayTime             = NULL;

    TC_Pause                   = NULL;

    TC_GetVideoParam = NULL;
    TC_SetEZoom           = NULL;
    TC_ResetEZoom         = NULL;
    TC_AddEZoom           = NULL;
    TC_RemoveEZoom        = NULL;
    TC_ResetPlayWnd       = NULL;
    TC_DrawRect           = NULL;

    TC_SearchVCAInfo = NULL;
    TC_SetVideoDecryptKey = NULL;
    TC_GetVideoDecryptKey = NULL;
    TC_RegisterCommonEventCallBack = NULL;
    TC_RegisterCommonEventCallBackEx = NULL;

    TC_SetModeRule = NULL;

    TC_DrawPolygonOnLocalVideo = NULL;
    TC_RegisterPsPackCallBack = NULL;

    TC_GetMarkInfo = NULL;
    TC_SetMarkInfo = NULL;
    TC_SeekMark = NULL;
    TC_StartCaptureFile = NULL;
    TC_StopCaptureFile =  NULL;

    IMPLEMENT_FUNCTION(TC_SetVerticalSync);

    TC_StartAudioCapture = NULL;
    TC_StopAudioCapture = NULL;
    TC_RegisterNotifyAudioCapture = NULL;
}
CLS_PlaySDK::~CLS_PlaySDK()
{
    ReleaseDll();
}
/*-----------------------------------*/
/* LoadDll/ReleaseDll */
/*-----------------------------------*/
int CLS_PlaySDK::LoadDll()
{
	if (m_hHandle != NULL)
	{
		return -1;
	}

	m_hHandle = LoadLibrary(".\\lib\\PlaySdkM4.dll");
	if (m_hHandle == NULL)
	{
        m_hHandle = LoadLibrary("PlaySdkM4.dll");
        if (m_hHandle == NULL)
        {
		    MessageBox(NULL, "Can't load PlaySdkM4.dll!", "PlayDemo", MB_OK);
		    return -1;
        }
	}

	TC_CreateSystem            = (pTC_CreateSystem)           GetProcAddress(m_hHandle, "TC_CreateSystem");
	TC_DeleteSystem            = (pTC_DeleteSystem)           GetProcAddress(m_hHandle, "TC_DeleteSystem");

	TC_CreatePlayerFromFile    = (pTC_CreatePlayerFromFile)   GetProcAddress(m_hHandle, "TC_CreatePlayerFromFile");
    TC_DeletePlayer            = (pTC_DeletePlayer)           GetProcAddress(m_hHandle, "TC_DeletePlayer");

    TC_RegisterEventMsg        = (pTC_RegisterEventMsg)       GetProcAddress(m_hHandle, "TC_RegisterEventMsg");
    TC_RegisterNotifyPlayToEnd = (pTC_RegisterNotifyPlayToEnd)GetProcAddress(m_hHandle, "TC_RegisterNotifyPlayToEnd");
    TC_RegisterNotifyGetDecAV  = (pTC_RegisterNotifyGetDecAV) GetProcAddress(m_hHandle, "TC_RegisterNotifyGetDecAV");

    TC_RegisterDrawFun  = (pTC_RegisterDrawFun) GetProcAddress(m_hHandle, "TC_RigisterDrawFun");

    TC_SetPlayRect             = (pTC_SetPlayRect)            GetProcAddress(m_hHandle, "TC_SetPlayRect");
    TC_SetPlayRectEx           = (pTC_SetPlayRectEx)          GetProcAddress(m_hHandle, "TC_SetPlayRectEx");
	TC_Play				       = (pTC_Play)				      GetProcAddress(m_hHandle, "TC_Play");
	TC_Stop                    = (pTC_Stop)                   GetProcAddress(m_hHandle, "TC_Stop");
	TC_GoBegin                 = (pTC_GoBegin)                GetProcAddress(m_hHandle, "TC_GoBegin");
	TC_GoEnd				   = (pTC_GoEnd)                  GetProcAddress(m_hHandle, "TC_GoEnd");
	TC_StepForward             = (pTC_StepForward)            GetProcAddress(m_hHandle, "TC_StepForward");
	TC_StepBackward            = (pTC_StepBackward)           GetProcAddress(m_hHandle, "TC_StepBackward");
	TC_FastForward             = (pTC_FastForward)            GetProcAddress(m_hHandle, "TC_FastForward");
	TC_SlowForward             = (pTC_SlowForward)            GetProcAddress(m_hHandle, "TC_SlowForward");
	TC_FastBackward            = (pTC_FastBackward)           GetProcAddress(m_hHandle, "TC_FastBackward");
	TC_PlayAudio               = (pTC_PlayAudio)              GetProcAddress(m_hHandle, "TC_PlayAudio");
	TC_StopAudio               = (pTC_StopAudio)              GetProcAddress(m_hHandle, "TC_StopAudio");
	TC_SetAudioVolume          = (pTC_SetAudioVolume)         GetProcAddress(m_hHandle, "TC_SetAudioVolume");
    TC_GetAudioVolume          = (pTC_GetAudioVolume)         GetProcAddress(m_hHandle, "TC_GetAudioVolume");
    TC_SetAudioVolumeEx        = (pTC_SetAudioVolumeEx)       GetProcAddress(m_hHandle, "TC_SetAudioVolumeEx");
    TC_GetAudioVolumeEx        = (pTC_GetAudioVolumeEx)       GetProcAddress(m_hHandle, "TC_GetAudioVolumeEx");
    TC_Seek                    = (pTC_Seek)                   GetProcAddress(m_hHandle, "TC_Seek");
    TC_SeekEx                  = (pTC_SeekEx)                 GetProcAddress(m_hHandle, "TC_SeekEx");

    TC_GetFrameRate            = (pTC_GetFrameRate)           GetProcAddress(m_hHandle, "TC_GetFrameRate");
    TC_GetPlayingFrameNum      = (pTC_GetPlayingFrameNum)     GetProcAddress(m_hHandle, "TC_GetPlayingFrameNum");
    TC_GetFrameCount           = (pTC_GetFrameCount)          GetProcAddress(m_hHandle, "TC_GetFrameCount");
    TC_GetBeginEnd             = (pTC_GetBeginEnd)            GetProcAddress(m_hHandle, "TC_GetBeginEnd");
    TC_GetFileName             = (pTC_GetFileName)            GetProcAddress(m_hHandle, "TC_GetFileName");

    TC_CapturePic              = (pTC_CapturePic)             GetProcAddress(m_hHandle, "TC_CapturePic");
    TC_CaptureBmpPic           = (pTC_CaptureBmpPic)          GetProcAddress(m_hHandle, "TC_CaptureBmpPic");

    TC_CreatePlayerFromStream  = (pTC_CreatePlayerFromStream) GetProcAddress(m_hHandle, "TC_CreatePlayerFromStream");
    TC_CreatePlayerFromStreamEx  = (pTC_CreatePlayerFromStreamEx) GetProcAddress(m_hHandle, "TC_CreatePlayerFromStreamEx");
    TC_PutStreamToPlayer       = (pTC_PutStreamToPlayer)      GetProcAddress(m_hHandle, "TC_PutStreamToPlayer");

    TC_StartMonitorCPU         = (pTC_StartMonitorCPU)        GetProcAddress(m_hHandle, "TC_StartMonitorCPU");
    TC_StopMonitorCPU          = (pTC_StopMonitorCPU)         GetProcAddress(m_hHandle, "TC_StopMonitorCPU");

    TC_GetVersion              = (pTC_GetVersion)             GetProcAddress(m_hHandle, "TC_GetVersion");

    // for vod
    TC_CreatePlayerFromVoD     = (pTC_CreatePlayerFromVoD)    GetProcAddress(m_hHandle, "TC_CreatePlayerFromVoD");
    TC_SetVoDPlayerOver        = (pTC_SetVoDPlayerOver )      GetProcAddress(m_hHandle, "TC_SetVoDPlayerOver");
    TC_SetCleanVoDBuffer       = (pTC_SetCleanVoDBuffer)      GetProcAddress(m_hHandle, "TC_SetCleanVoDBuffer");

    TC_GetPlayTime             = (pTC_GetPlayTime)GetProcAddress(m_hHandle, "TC_GetPlayTime");

    TC_Pause                    = (pTC_Pause)                   GetProcAddress(m_hHandle, "TC_Pause");

    TC_GetVideoParam		    = (pTC_GetVideoParam)GetProcAddress(m_hHandle, "TC_GetVideoParam");

    TC_DrawRect			 = (pTC_DrawRect)GetProcAddress(m_hHandle, "TC_DrawRect");
    TC_SetEZoom          = (pTC_SetEZoom)GetProcAddress(m_hHandle, "TC_SetEZoom");
    TC_ResetEZoom        = (pTC_ResetEZoom)GetProcAddress(m_hHandle, "TC_ResetEZoom");
    TC_AddEZoom          = (pTC_AddEZoom)GetProcAddress(m_hHandle, "TC_AddEZoom");
    TC_RemoveEZoom       = (pTC_RemoveEZoom)GetProcAddress(m_hHandle, "TC_RemoveEZoom");
    TC_ResetPlayWnd      = (pTC_ResetPlayWnd)GetProcAddress(m_hHandle, "TC_ResetPlayWnd");

    TC_SearchVCAInfo = (pTC_SearchVCAInfo)GetProcAddress(m_hHandle, "TC_SearchVCAInfo");
    TC_GetBeginEndTimeStamp = (pTC_GetBeginEndTimeStamp)GetProcAddress(m_hHandle, "TC_GetBeginEndTimeStamp");

    EXPORT_FUNCTION(m_hHandle, TC_SetVideoDecryptKey);
    EXPORT_FUNCTION(m_hHandle, TC_GetVideoDecryptKey);
    EXPORT_FUNCTION(m_hHandle, TC_RegisterCommonEventCallBack);
    EXPORT_FUNCTION(m_hHandle, TC_RegisterCommonEventCallBackEx);
    EXPORT_FUNCTION(m_hHandle, TC_SetModeRule);
    EXPORT_FUNCTION(m_hHandle, TC_DrawPolygonOnLocalVideo);
    EXPORT_FUNCTION(m_hHandle, TC_RegisterPsPackCallBack);
    EXPORT_FUNCTION(m_hHandle, TC_DeletePlayerEx);

    EXPORT_FUNCTION(m_hHandle, TC_GetMarkInfo);
    EXPORT_FUNCTION(m_hHandle, TC_SetMarkInfo);
    EXPORT_FUNCTION(m_hHandle, TC_SeekMark);
    EXPORT_FUNCTION(m_hHandle, TC_StartCaptureFile);
    EXPORT_FUNCTION(m_hHandle, TC_StopCaptureFile);

    EXPORT_FUNCTION(m_hHandle, TC_SetVerticalSync);
    EXPORT_FUNCTION(m_hHandle, TC_StartAudioCapture);
    EXPORT_FUNCTION(m_hHandle, TC_StopAudioCapture);
    EXPORT_FUNCTION(m_hHandle, TC_RegisterNotifyAudioCapture);

	EXPORT_FUNCTION(m_hHandle, TC_SetStreamBufferSize);//add cyy
	EXPORT_FUNCTION(m_hHandle, TC_SetFrameListBufferSize);//add cyy

    return 0;
}
void CLS_PlaySDK::ReleaseDll()
{
    if (m_hHandle != NULL)
	{
		FreeLibrary(m_hHandle);
	}
}
/*****************************************************************************/
/*-----------------------------------*/
/* 系统创建/释放 */
/*-----------------------------------*/
int CLS_PlaySDK::CreateSystem(HWND _hWnd)
{
	if (TC_CreateSystem == NULL)
	{
		return -1;
	}

	return TC_CreateSystem(_hWnd);	
}
int CLS_PlaySDK::DeleteSystem()
{
	if (TC_DeleteSystem == NULL)
	{
		return -1;
	}

	return TC_DeleteSystem();	
}
/*-----------------------------------*/
/* 创建和关闭播放实例 */
/*-----------------------------------*/
int CLS_PlaySDK::CreatePlayerFromFile(HWND _hWnd, char* _pcFileName, int _iBufSize)
{
	if (TC_CreatePlayerFromFile == NULL)
	{
		return -1;
	}

	return TC_CreatePlayerFromFile(_hWnd, _pcFileName, _iBufSize,0, NULL, 0, NULL);
}
int CLS_PlaySDK::DeletePlayer(int _iID)
{
	if (TC_DeletePlayer == NULL)
	{
		return -1;
	}

	return TC_DeletePlayer(_iID);
}

int CLS_PlaySDK::DeletePlayerEx(int _iID, int _iParam)
{
       if (TC_DeletePlayerEx == NULL)
	{
		return -1;
	}

	return TC_DeletePlayerEx(_iID, _iParam);
}
/*-----------------------------------*/
/* 注册消息和回调 */
/*-----------------------------------*/
void CLS_PlaySDK::RegisterEventMsg(HWND _hEventWnd, UINT _uiEventMsg)
{
    if (TC_RegisterEventMsg == NULL)
	{
		return;
	}

    TC_RegisterEventMsg(_hEventWnd, _uiEventMsg);
}
int CLS_PlaySDK::RegisterNotifyPlayToEnd(void* _PlayEndFun)
{
    if (TC_RegisterNotifyPlayToEnd == NULL)
	{
		return -1;
	}

	return TC_RegisterNotifyPlayToEnd(_PlayEndFun);
}
int CLS_PlaySDK::RegisterNotifyGetDecAV(int _iID, void* _GetDecAVCbk, bool _blDisplay)
{
    if (TC_RegisterNotifyGetDecAV == NULL)
	{
		return -1;
	}

	return TC_RegisterNotifyGetDecAV(_iID, _GetDecAVCbk, _blDisplay);
}
/*-----------------------------------*/
/* 播放控制 */
/*-----------------------------------*/
int CLS_PlaySDK::SetPlayRect(int _iID, RECT* _pDrawRect)
{
    if (TC_SetPlayRect == NULL)
	{
		return -1;
	}

	return TC_SetPlayRect(_iID, _pDrawRect);
}
int CLS_PlaySDK::SetPlayRectEx(int _iID, RECT* _pDrawRect, DWORD _dwMask)
{
    if (TC_SetPlayRectEx == NULL)
	{
		return -1;
	}

	return TC_SetPlayRectEx(_iID, _pDrawRect, _dwMask);
}
int CLS_PlaySDK::Play(int _iID)
{
	if (TC_Play == NULL)
	{
		return -1;
	}

	return TC_Play(_iID);
}
int CLS_PlaySDK::Stop(int _iID)
{
	if (TC_Stop == NULL)
	{
		return -1;
	}

	return TC_Stop(_iID);
}
int CLS_PlaySDK::GoBegin(int _iID)
{
	if (TC_GoBegin == NULL)
	{
		return -1;
	}

	return TC_GoBegin(_iID);
}
int CLS_PlaySDK::GoEnd(int _iID)
{
	if (TC_GoEnd == NULL)
	{
		return -1;
	}

	return TC_GoEnd(_iID);
}
int CLS_PlaySDK::StepForward(int _iID)
{
	if (TC_StepForward == NULL)
	{
		return -1;
	}

	return TC_StepForward(_iID);
}
int CLS_PlaySDK::StepBackward(int _iID)
{
	if (TC_StepBackward == NULL)
	{
		return -1;
	}

	return TC_StepBackward(_iID);
}
int CLS_PlaySDK::FastForward(int _iID, int _iSpeed)
{
	if (TC_FastForward == NULL)
	{
		return -1;
	}

	return TC_FastForward(_iID, _iSpeed);
}
int CLS_PlaySDK::SlowForward(int _iID, int _iSpeed)
{
	if (TC_SlowForward == NULL)
	{
		return -1;
	}

	return TC_SlowForward(_iID, _iSpeed);
}
int CLS_PlaySDK::FastBackward(int _iID)
{
	if (TC_FastBackward == NULL)
	{
		return -1;
	}

	return TC_FastBackward(_iID);
}
int CLS_PlaySDK::PlayAudio(int _iID)
{
	if (TC_PlayAudio == NULL)
	{
		return -1;
	}

	return TC_PlayAudio(_iID);
}
int CLS_PlaySDK::StopAudio(int _iID)
{
	if (TC_StopAudio == NULL)
	{
		return -1;
	}

	return TC_StopAudio(_iID);
}
int CLS_PlaySDK::SetAudioVolume(unsigned short _ustVolume)
{
	if (TC_SetAudioVolume == NULL)
	{
		return -1;
	}

	return TC_SetAudioVolume(_ustVolume);
}

int CLS_PlaySDK::GetAudioVolume(unsigned short &_ustVolume)
{
    if (TC_GetAudioVolume == NULL)
	{
		return -1;
	}

	return TC_GetAudioVolume(_ustVolume);
}

int CLS_PlaySDK::SetAudioVolumeEx(int _iID, unsigned short _ustVolume)
{
	if (TC_SetAudioVolumeEx == NULL)
	{
		return -1;
	}

	return TC_SetAudioVolumeEx(_iID, _ustVolume);
}

int CLS_PlaySDK::GetAudioVolumeEx(int _iID, unsigned short &_ustVolume)
{
    if (TC_GetAudioVolumeEx == NULL)
	{
		return -1;
	}

	return TC_GetAudioVolumeEx(_iID, _ustVolume);
}

int CLS_PlaySDK::Seek(int _iID, int _iFrameNo)
{
	if (TC_Seek == NULL)
	{
		return -1;
	}

	return TC_Seek(_iID, _iFrameNo);
}
int CLS_PlaySDK::SeekEx(int _iID, int _iFrameNo)
{
	if (TC_SeekEx == NULL)
	{
		return -1;
	}

	return TC_SeekEx(_iID, _iFrameNo);
}
/*-----------------------------------*/
/*  获取播放信息 */
/*-----------------------------------*/
int CLS_PlaySDK::GetFrameRate(int _iID)
{
	if (TC_GetFrameRate == NULL)
	{
		return -1;
	}

	return TC_GetFrameRate(_iID);
}
int CLS_PlaySDK::GetPlayingFrameNum(int _iID)
{
	if (TC_GetPlayingFrameNum == NULL)
	{
		return -1;
	}

	return TC_GetPlayingFrameNum(_iID);
}
int CLS_PlaySDK::GetFrameCount(int _iID)
{
	if (TC_GetFrameCount == NULL)
	{
		return -1;
	}

	return TC_GetFrameCount(_iID);
}
int CLS_PlaySDK::GetBeginEnd(int _iID, int* _piBegin, int* _piEnd)
{
	if (TC_GetBeginEnd == NULL)
	{
		return -1;
	}

	return TC_GetBeginEnd(_iID, _piBegin, _piEnd);
}
int CLS_PlaySDK::GetFileName(int _iID, char* _pcFileName)
{
	if (TC_GetFileName == NULL)
	{
		return -1;
	}

	return TC_GetFileName(_iID, _pcFileName);
}
/*-----------------------------------*/
/*  抓拍 */
/*-----------------------------------*/
int CLS_PlaySDK::CapturePic(int _iID, unsigned char** _ppucYUV)
{
    if (TC_CapturePic == NULL)
	{
		return -1;
	}

	return TC_CapturePic(_iID, _ppucYUV);
}
int CLS_PlaySDK::CaptureBmpPic(int _iID, char* _pcSaveFile)
{
    if (TC_CaptureBmpPic == NULL)
	{
		return -1;
	}

	return TC_CaptureBmpPic(_iID, _pcSaveFile);
}
/*-----------------------------------*/
/*  流播放 */
/*-----------------------------------*/
int CLS_PlaySDK::CreatePlayerFromStream(HWND _hWnd, unsigned char* _pucFileHeadBuf, int _iHeadSize)
{
	if (TC_CreatePlayerFromStream == NULL)
	{
		return -1;
	}

	return TC_CreatePlayerFromStream(_hWnd, _pucFileHeadBuf, _iHeadSize);
}

int CLS_PlaySDK::CreatePlayerFromStreamEx(HWND _hWnd, unsigned char* _pucFileHeadBuf, int _iHeadSize, int _iStreamBufferSize)
{
	if (TC_CreatePlayerFromStreamEx == NULL)
	{
		return -1;
	}

	return TC_CreatePlayerFromStreamEx(_hWnd, _pucFileHeadBuf, _iHeadSize, _iStreamBufferSize);
}

int CLS_PlaySDK::PutStreamToPlayer(int _iID, unsigned char* _pucStreamData, int _iSize)
{
	if (TC_PutStreamToPlayer == NULL)
	{
		return -1;
	}

	return TC_PutStreamToPlayer(_iID, _pucStreamData, _iSize);
}
/*-----------------------------------*/
/* CPU监控 */
/*-----------------------------------*/
int CLS_PlaySDK::StartMonitorCPU()
{
    if (TC_StartMonitorCPU == NULL)
	{
		return -1;
	}

	return TC_StartMonitorCPU();
}
int CLS_PlaySDK::StopMonitorCPU()
{
    if (TC_StopMonitorCPU == NULL)
	{
		return -1;
	}

	return TC_StopMonitorCPU();
}
/*-----------------------------------*/
/* 版本信息 */
/*-----------------------------------*/
int CLS_PlaySDK::GetVersion(SDK_VERSION *_ver)
{
    if (TC_GetVersion == NULL)
	{
		return -1;
	}

	return TC_GetVersion(_ver);
}

int CLS_PlaySDK::CreatePlayerFromVOD(HWND _hWnd, unsigned char* _pucVideoHeadBuf, int _iHeadSize)
{
    if (TC_CreatePlayerFromVoD == NULL)
	{
		return -1;
	}

	return TC_CreatePlayerFromVoD(_hWnd, _pucVideoHeadBuf, _iHeadSize);
}
int CLS_PlaySDK::SetVoDPlayerOver(int _iID)
{
    if (TC_SetVoDPlayerOver == NULL)
	{
		return -1;
	}

	return TC_SetVoDPlayerOver(_iID);
}
int CLS_PlaySDK::SetCleanVoDBuffer(int _iID)
{
    if (TC_SetCleanVoDBuffer == NULL)
	{
		return -1;
	}

	return TC_SetCleanVoDBuffer(_iID);
}
int CLS_PlaySDK::GetPlayTime(int _iID)
{
    if (TC_GetPlayTime == NULL)
	{
		return -1;
	}

	return TC_GetPlayTime(_iID);
}
int CLS_PlaySDK::RegisterDrawFun(int _iID,void* _DrawFun,LONG nUser)
{
    if (TC_RegisterDrawFun == NULL)
	{
		return -1;
	}

	return TC_RegisterDrawFun(_iID,_DrawFun,nUser);
}
int CLS_PlaySDK::Pause(int _iID)
{
    if (TC_Pause == NULL)
	{
		return -1;
	}

	return TC_Pause(_iID);
}

int CLS_PlaySDK::SetEZoom(int _iID, int _iZoomID, RECT _pRectInVideo)
{
    if (!TC_SetEZoom)
        return -1;

    return TC_SetEZoom(_iID, _iZoomID, _pRectInVideo);
}

int CLS_PlaySDK::ResetEZoom(int _iID , int _iZoomID)
{
    if (!TC_ResetEZoom)
        return -1;

    return TC_ResetEZoom(_iID, _iZoomID);
}

int CLS_PlaySDK::AddEZoom(int _iID, HWND _hWnd, RECT *_rcZoom, int _iCount)
{
    if (!TC_AddEZoom)
        return -1;

    int iZoomID = TC_AddEZoom(_iID, _hWnd, _rcZoom, _iCount);

    return iZoomID;
}

int CLS_PlaySDK::RemoveEZoom(int _iID , int _iZoomID)
{
    if (!TC_RemoveEZoom)
        return -1;

    return TC_RemoveEZoom(_iID, _iZoomID);
}

int CLS_PlaySDK::ResetPlayWnd(int _iID, HWND _hWnd)
{
    if (!TC_ResetPlayWnd)
        return -1;

    return TC_ResetPlayWnd(_iID, _hWnd);
}

int CLS_PlaySDK::GetVideoParam(int _iID, int* _pWidth, int* _pHeight, int* _pFrameRate)
{
    if (!TC_GetVideoParam)
        return -1;

    return TC_GetVideoParam(_iID, _pWidth, _pHeight, _pFrameRate);
}

int CLS_PlaySDK::DrawRect(int _iID, RECT *_rcDraw, int _iCount)
{
    if(!TC_DrawRect)
        return -1;
        
    return TC_DrawRect(_iID, _rcDraw, _iCount);
}

int CLS_PlaySDK::SearchVCAInfo( int _iID, int _iSearchRule, int _iFrameStart, void* _pParamBuf, int _iBufSize, int _iContinue, unsigned int &_iFrame)
{
    if (!TC_SearchVCAInfo)
        return -1;

    unsigned int iFrame = 0;
    return TC_SearchVCAInfo(_iID, _iSearchRule, _iFrameStart, _pParamBuf, _iBufSize, _iContinue, &_iFrame);
}

int CLS_PlaySDK::GetBeginEndTimeStamp(int _iID, unsigned int *_puiBegin, unsigned int * _puiEnd)
{
    if (TC_GetBeginEndTimeStamp == NULL)
        return -1;

    return TC_GetBeginEndTimeStamp(_iID, _puiBegin, _puiEnd);
}

int CLS_PlaySDK::SetVideoDecryptKey(int _iID, char* _lpBuf, int _iBufSize)
{
    if (TC_SetVideoDecryptKey == NULL)
        return -1;
    return TC_SetVideoDecryptKey(_iID, _lpBuf, _iBufSize);
}

int CLS_PlaySDK::GetVideoDecryptKey(int _iID, char* _lpBuf, int _iBufSize)
{
    if (TC_GetVideoDecryptKey == NULL)
        return -1;
    return TC_GetVideoDecryptKey(_iID, _lpBuf, _iBufSize);
}

int CLS_PlaySDK::RegisterCommonEventCallBack(pfCBCommonEventNotify _pf)
{
    if (TC_RegisterCommonEventCallBack == NULL)
        return -1;
    return TC_RegisterCommonEventCallBack(_pf);
}

int CLS_PlaySDK::RegisterCommonEventCallBackEx(pfCBCommonEventNotifyEx _pf, void* _lpUserData)
{
    if (TC_RegisterCommonEventCallBackEx == NULL)
        return -1;
    return TC_RegisterCommonEventCallBackEx(_pf, _lpUserData);
}

int CLS_PlaySDK::SetModeRule(int _iID, int _iModeRule)
{
    if (TC_SetModeRule == NULL)
        return -1;

    return TC_SetModeRule(_iID, _iModeRule);
}

int CLS_PlaySDK::DrawPolygonOnLocalVideo(int _iID, POINT* _pPointArray, int _iPointCount, int _iEnableArrow)
{
    if (TC_DrawPolygonOnLocalVideo == NULL)
        return -1;

    return TC_DrawPolygonOnLocalVideo(_iID, _pPointArray, _iPointCount, _iEnableArrow);
}

int CLS_PlaySDK::RegisterPsPackCallBack(PSPACK_NOTIFY _pf, void* _lpUserData)
{
    if (TC_RegisterPsPackCallBack == NULL)
        return -1;

    return TC_RegisterPsPackCallBack(_pf, _lpUserData);
}

 int CLS_PlaySDK::GetMarkInfo(int _iID, PVOID _pData,int _iSize)
 {
     if (TC_GetMarkInfo == NULL)
        return -1;

     return TC_GetMarkInfo(_iID,_pData,_iSize);
 }

 int CLS_PlaySDK::SetMarkInfo(int _iID,int _iFrameNO,int _iType)
 {
     if (TC_SetMarkInfo == NULL)
        return -1;

     return TC_SetMarkInfo(_iID,_iFrameNO,_iType);
 }

 int CLS_PlaySDK::SeekMark(int _iID,int _iFrameNO)
 {
     if (TC_SeekMark == NULL)
        return -1;

     return TC_SeekMark(_iID,_iFrameNO);
 }

 int CLS_PlaySDK::StartCaptureFile(int _iID, char* _cFileName,int _iRecFileType)
 {
     if (TC_StartCaptureFile == NULL)
        return -1;

     return TC_StartCaptureFile(_iID,_cFileName, _iRecFileType);
 }

 int CLS_PlaySDK::StopCaptureFile(int _iID)
 {
     if (TC_StopCaptureFile == NULL)
        return -1;

     return TC_StopCaptureFile(_iID);
 }
 int CLS_PlaySDK::SetStreamBufferSize(int _iID, int _iBufSize)
 {
     if (TC_SetStreamBufferSize == NULL)
        return -1;

     return TC_SetStreamBufferSize(_iID,_iBufSize);
 }
 int CLS_PlaySDK::SetFrameListBufferSize(int _iID, int _iBufSize)
 {
     if (TC_SetFrameListBufferSize == NULL)
     return -1;

     return TC_SetFrameListBufferSize(_iID, _iBufSize, -1);
 }

int CLS_PlaySDK::SetVerticalSync(int _iID,int _iFlag)
 {
     if (TC_SetVerticalSync == NULL)
        return -1;

     return TC_SetVerticalSync(_iID,_iFlag);
 }

int CLS_PlaySDK::StartAudioCapture(int _iID, int _iWavein,int _iWaveout)
 {
     if (TC_StartAudioCapture == NULL)
        return -1;

     return TC_StartAudioCapture(_iID, _iWavein, _iWaveout);
 }

 int CLS_PlaySDK::StopAudioCapture(int _iID)
 {
     if (TC_StopAudioCapture == NULL)
        return -1;

     return TC_StopAudioCapture(_iID);
 }

 int CLS_PlaySDK::RegisterNotifyAudioCapture(void* _pvAudioCaptureFun, void* _pvContext)
 {
     if (TC_RegisterNotifyAudioCapture == NULL)
        return -1;

     return TC_RegisterNotifyAudioCapture(_pvAudioCaptureFun, _pvContext);
 }