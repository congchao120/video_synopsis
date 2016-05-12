#ifndef GlobalH
#define GlobalH
//---------------------------------------------------------------------------
//文件头结构
typedef struct
{
    unsigned short  FrameRate;	//帧率
    unsigned short  Width;		//图像宽
    unsigned short  Height;		//图像高
    unsigned short  Mode;		//编码模式：11（MP4)， 21(H264)
    unsigned short  bAudio;		//是否带音频
    unsigned short  Year;		//H264编码不用以下字段，全部置0xFF
    unsigned short  Month;
    unsigned short  Day;
    unsigned short  Hour;
    unsigned short  Minute;
    unsigned short  Second;
    unsigned short  CoverMask;
    char cCovWord[64];		//这里可以设成"Private MEDIA STREAM H264"
}S_header;
//---------------------------------------------------------------------------
#endif

 