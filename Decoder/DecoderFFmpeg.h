/** 可处理输入为H.264  H.265数据进行解码
*
*/

#ifndef DECODER_FFMPEG__H
#define DECODER_FFMPEG__H

// ffmpeg
#ifdef __cplusplus
extern "C"{
#endif
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#ifdef __cplusplus
};
#endif

#include "DecoderBase.h"
#include "Lock.h"
#include "SingleLock.h"
#include "DecoderHeader.h"
#include <list>

using namespace std;

namespace decoder{
class DecoderFFmpeg : public DecoderBase
{
public:
	// 该enum 用于createDecoder
	//enum{
	//	h264 = AV_CODEC_ID_H264,
	//	h265 = AV_CODEC_ID_H265,
	//	mpeg2 = AV_CODEC_ID_MPEG2VIDEO
	//};

public:
	DecoderFFmpeg(void);
	~DecoderFFmpeg(void);
	
	// 创建解码器、释放解码器
	int createDecoder(const int& codecId);
	void destroyDecoder();

	// 设置解码格式（宽高）， 视频所需要转码的格式, 不设置使用默认的
	int setDecodeFormat(const VideoFormat& fmt);
	
	// 获取解码之前、解码转换之后的视频格式
	VideoFormat getOriginalVideoFormat();
	VideoFormat getDecodedVideoFormat();

	// 开始解码, 0代表没有接触视频帧， 同时也没有异常发生
	int decodeVideo(tuint8* buf, tuint32 bufLen);
	// 获取解码后的YUV数据 packet格式 Y->U->V
	FrameData*	takeFirstFrame();

	//// 异步设置VideoPack, 暂时不实现具体业务(需要开一个解码线程，专用于解码)
	//void setPackToDecoder(tuint8* buf, tuint32	bufLen);
	//// 使用结束之后，需要调用release 进行释放， 不要直接调用delete
	//VideoPack* takeFirstPack();

private:
	AVCodecID	getCodecId(const int& codecId );
	CodecId		getCodecId2(AVCodecID id);
	AVPixelFormat getPixelFormat(const int& pixFmt);
	FramePixFmt	  getPixelFormat2(AVPixelFormat fmt);
	
	// 获取解码器
	AVCodec*	getCodec(int codecId);

	// 创建转换句柄
	int	createCvtCtx();

	// 打印当前帧类型
	void printPacketType(int typeId);

	// 根据视频帧YUV格式写入解码后的帧
	void writeScaledFrameData(FrameData* data);
	// 写入原始视频帧
	void writeOriginalFrameData(FrameData* data);

private:
	// 锁构造，防止多次注册，影响效率
	CLock		m_decoderLock;

	// 解码相关
	AVCodecID	m_iCodecId;
	AVCodec*	m_pCodec;
	AVCodecContext*	m_pCodecCtx;
	AVCodecParserContext*	m_pCodecParserCtx;
	AVFrame*	m_pFrame;
	

	///* 数据源
	//*	使用m_lsVideoPack， 深拷贝，将外部数据拷贝到该结构体中，外部数据即可释放， 可做异步解码
	//*   也可以直接使用外部的数据源，同步解码
	//*/
	//CLock	m_videoPackLock;
	//std::list<VideoPack*>	m_videoPackLst;
	
	// 解码之后的帧变换
	struct SwsContext*		m_pImgCvtCtx;
	AVFrame*	m_pCvtFrame;
	tuint8*		m_pCvtBuffer;
	
	// 原视频格式、解码转换之后的视频格式， 原无法修改， 解码之后可修改
	VideoFormat	m_stVideoFormatSrc;
	VideoFormat	m_stVideoFormatDst;	

	// 解码 之后保存的码流序列, 存在向解码器塞入很大的一个buffer，这些buffer中有很多帧
	CLock		m_frameLock;
	list<FrameData*>	m_frameDataLst;	

private:
	static bool m_isRegisteredCodec;
};
}
#endif