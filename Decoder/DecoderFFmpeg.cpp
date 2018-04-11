#include "DecoderFFmpeg.h"
using namespace decoder;

bool DecoderFFmpeg::m_isRegisteredCodec = false;

DecoderFFmpeg::DecoderFFmpeg(void)
	: m_pCodec(nullptr)
	, m_pCodecCtx(nullptr)
	, m_iCodecId(AVCodecID(-1))
	, m_pCodecParserCtx(nullptr)
	, m_pFrame(nullptr)
	, m_pImgCvtCtx(nullptr)
	, m_pCvtFrame(nullptr)
	, m_pCvtBuffer(nullptr)
{
	if(!m_isRegisteredCodec){
		m_isRegisteredCodec = true;
		avcodec_register_all();
	}
}


DecoderFFmpeg::~DecoderFFmpeg(void)
{
	destroyDecoder();
}

int DecoderFFmpeg::createDecoder(const int& codecId)
{
	SingleLock lock(m_decoderLock);
	m_stVideoFormatSrc.m_iCodecId = codecId;
	m_stVideoFormatDst.m_iCodecId = codecId;

	m_iCodecId = getCodecId(codecId);
	// AVCodec
	m_pCodec = getCodec(m_iCodecId);
	if(m_pCodec == nullptr)
		return DECODER_ERR_SYS;

	m_pCodecCtx = avcodec_alloc_context3(m_pCodec);
	if (m_pCodecCtx == nullptr)
		return DECODER_ERR_SYS;

	if(avcodec_open2(m_pCodecCtx, m_pCodec, nullptr) < 0){
		destroyDecoder();
		return DECODER_ERR_SYS;
	}

	m_pCodecParserCtx = av_parser_init(m_iCodecId);
	if(m_pCodecParserCtx == nullptr){
		destroyDecoder();
		return DECODER_ERR_SYS;
	}

	m_pFrame = av_frame_alloc();
	if(m_pFrame == nullptr){
		destroyDecoder();
		return DECODER_ERR_SYS;
	}

	return DECODER_ERR_OK;
}

void DecoderFFmpeg::destroyDecoder()
{
	SingleLock lock(m_decoderLock);

	if(m_pImgCvtCtx != nullptr){
		sws_freeContext(m_pImgCvtCtx);
		m_pImgCvtCtx = nullptr;
	}
	if(m_pCvtBuffer != nullptr){
		av_free(m_pCvtBuffer);
		m_pCvtBuffer = nullptr;
	}

	if(m_pCodecParserCtx != nullptr){
		av_parser_close(m_pCodecParserCtx);
		m_pCodecParserCtx = nullptr;
	}

	if(m_pFrame != nullptr){
		av_frame_free(&m_pFrame);
		m_pFrame = nullptr;
	}
	if(m_pCvtFrame != nullptr){
		av_frame_free(&m_pCvtFrame);
		m_pCvtFrame = nullptr;
	}

	if(m_pCodecCtx != nullptr){
		avcodec_close(m_pCodecCtx);
		av_free(m_pCodecCtx);
		m_pCodecCtx = nullptr;
	}

}

// 设置解码之后， 视频所需要转码的格式
int DecoderFFmpeg::setDecodeFormat(const VideoFormat& fmt)
{
	SingleLock lock(m_decoderLock);
	// 如果转换器没有初始化，直接返回
	if(m_pImgCvtCtx == nullptr){
		m_stVideoFormatDst.m_iWidth = fmt.m_iWidth;
		m_stVideoFormatDst.m_iHeight = fmt.m_iHeight;
		if(getPixelFormat(fmt.m_iPixFmt) != AV_PIX_FMT_NONE)
			m_stVideoFormatDst.m_iPixFmt = fmt.m_iPixFmt;

		return DECODER_ERR_OK;
	}

	// 视频的宽高发生变化
	if(m_stVideoFormatDst.m_iWidth != fmt.m_iWidth || m_stVideoFormatDst.m_iHeight != fmt.m_iHeight){
		m_stVideoFormatDst.m_iWidth = fmt.m_iWidth;
		m_stVideoFormatDst.m_iHeight = fmt.m_iHeight;
		if(getPixelFormat(fmt.m_iPixFmt) != AV_PIX_FMT_NONE){
			m_stVideoFormatDst.m_iPixFmt = fmt.m_iPixFmt;
		}
		// 释放对象, 以便于重建对象
		sws_freeContext(m_pImgCvtCtx);
		m_pImgCvtCtx = nullptr;

		av_frame_free(&m_pCvtFrame);
		m_pCvtFrame = nullptr;
		
		av_free(m_pCvtBuffer);
		m_pCvtBuffer = nullptr;

		return createCvtCtx();
	}

	return DECODER_ERR_OK;
}

// 获取解码之前、解码转换之后的视频格式
VideoFormat DecoderFFmpeg::getOriginalVideoFormat()
{
	return m_stVideoFormatSrc;
}

VideoFormat DecoderFFmpeg::getDecodedVideoFormat()
{
	return m_stVideoFormatDst;
}

// 开始解码
int DecoderFFmpeg::decodeVideo(tuint8* buf, tuint32 bufLen)
{
	tuint32 cur_len = bufLen;
	tuint8* cur_ptr = buf;

	// 解码过程中不允许 创建 释放解码句柄
	SingleLock lock(m_decoderLock);
	AVPacket pkt;
	while(cur_len > 0){
		// 读取pkt
		int len = av_parser_parse2(m_pCodecParserCtx, m_pCodecCtx, &pkt.data, &pkt.size, cur_ptr, cur_len,AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
		cur_len -= len;
		cur_ptr += len;
		if(pkt.size == 0)
			continue;

		// 解析pkt
		printPacketType(m_pCodecParserCtx->pict_type);

		bool isKeyFrame = (m_pCodecParserCtx->pict_type == AV_PICTURE_TYPE_I);
		int decoded = 0;
		int ret = avcodec_decode_video2(m_pCodecCtx, m_pFrame, &decoded, &pkt);
		if(ret < 0){
			return DECODER_ERR_SYS;
		}
		// 如果解码成功
		if(decoded){
			if(0 == m_stVideoFormatSrc.m_iWidth && 0 == m_stVideoFormatSrc.m_iHeight){
				m_stVideoFormatSrc.m_iHeight = m_pCodecCtx->height;
				m_stVideoFormatSrc.m_iWidth = m_pCodecCtx->width;
				m_stVideoFormatSrc.m_iCodecId = getCodecId2(m_pCodecCtx->codec_id);
				m_stVideoFormatSrc.m_iPixFmt = getPixelFormat2(m_pCodecCtx->pix_fmt);
			}
			if(0 == m_stVideoFormatDst.m_iWidth && 0 == m_stVideoFormatDst.m_iHeight){
				m_stVideoFormatDst.m_iHeight = m_pCodecCtx->height;
				m_stVideoFormatDst.m_iWidth = m_pCodecCtx->width;
				m_stVideoFormatDst.m_iCodecId = getCodecId2(m_pCodecCtx->codec_id);
			}

			// 转换, 如果转换失败， 则保存原始帧信息
			bool useOrgData = true;
			FrameData* data = new FrameData;
			data->m_stHeader.m_isKeyFrame = isKeyFrame;
			data->AddRef();
			
			if(createCvtCtx() == DECODER_ERR_OK){
				ret = sws_scale(m_pImgCvtCtx, m_pFrame->data, m_pFrame->linesize, 0, m_pCodecCtx->height, m_pCvtFrame->data, m_pCvtFrame->linesize);
				if(ret > 0){
					useOrgData = false;
					writeScaledFrameData(data);
				}
			}
			
			// 只有在变换失败的时候， 写原始数据
			if(useOrgData){
				writeOriginalFrameData(data);
			}

			SingleLock frameLock(m_frameLock);
			m_frameDataLst.push_back(data);
		}
	}

	return m_frameDataLst.size();
}

FrameData* DecoderFFmpeg::takeFirstFrame()
{
	SingleLock lock(m_frameLock);
	if(m_frameDataLst.empty())
		return nullptr;

	FrameData* ptr = *(m_frameDataLst.begin());
	m_frameDataLst.erase(m_frameDataLst.begin());

	return ptr;
}


//////////////////////////////////////////////////////////////////////////
// 暂时无用
//void DecoderFFmpeg::setPackToDecoder(tuint8* buf, tuint32	bufLen)
//{
//	m_videoPackLock.Lock();
//
//	Decoder::VideoPack* pck = new Decoder::VideoPack;
//	pck->AddRef();
//	pck->m_pInBufferSize = bufLen;
//	memcpy(pck->m_pInBuffer, buf, bufLen);
//	m_videoPackLst.push_back(pck);
//
//	m_videoPackLock.Unlock();
//}
//
//Decoder::VideoPack* DecoderFFmpeg::takeFirstPack()
//{
//	m_videoPackLock.Lock();
//	
//	Decoder::VideoPack* ptr = nullptr;
//	if(m_videoPackLst.empty()){
//		ptr = nullptr;
//	}
//	else{
//		ptr = *(m_videoPackLst.begin());
//		m_videoPackLst.erase(m_videoPackLst.begin());
//	}
//	
//	m_videoPackLock.Unlock();
//
//	return ptr;
//}




//////////////////////////////////////////////////////////////////////////
// private
AVCodecID DecoderFFmpeg::getCodecId(const int& codecId)
{
	switch(codecId){
	case h264:
		return AV_CODEC_ID_H264;
	case h265:
		return AV_CODEC_ID_H265;
	case mpeg2:
		return AV_CODEC_ID_MPEG2VIDEO;
	default:
		break;
	}
	return (AVCodecID)codecId;
}

CodecId DecoderFFmpeg::getCodecId2(AVCodecID codecId)
{
	switch(codecId){
	case AV_CODEC_ID_H264:
		return h264;
	case AV_CODEC_ID_H265:
		return h265;
	case AV_CODEC_ID_MPEG2VIDEO:
		return mpeg2;
	default:
		break;
	}
	return (CodecId)codecId;
}

AVPixelFormat DecoderFFmpeg::getPixelFormat(const int& pixFmt)
{
	switch(pixFmt){
	case none:
		return AV_PIX_FMT_NONE;
	case yuv420p:
		return AV_PIX_FMT_YUV420P;
	default:
		break;
	}
	return (AVPixelFormat)pixFmt;
}

FramePixFmt DecoderFFmpeg::getPixelFormat2(AVPixelFormat fmt)
{
	switch(fmt){
	case AV_PIX_FMT_NONE:
		return none;
	case AV_PIX_FMT_YUV420P:
		return yuv420p;
	default:
		break;
	}
	return (FramePixFmt)fmt;
}

AVCodec* DecoderFFmpeg::getCodec(int codecId)
{
	return avcodec_find_decoder((AVCodecID)codecId);
}

int DecoderFFmpeg::createCvtCtx()
{
	if(m_pImgCvtCtx == nullptr){
		m_pImgCvtCtx = sws_getContext(m_stVideoFormatSrc.m_iWidth, m_stVideoFormatSrc.m_iHeight, getPixelFormat(m_stVideoFormatSrc.m_iPixFmt), \
			m_stVideoFormatDst.m_iWidth, m_stVideoFormatDst.m_iHeight, getPixelFormat(m_stVideoFormatDst.m_iPixFmt), SWS_BICUBIC, nullptr, nullptr, nullptr);

		if(m_pImgCvtCtx == nullptr)
			return DECODER_ERR_SYS;
	}

	if(m_pCvtFrame ==nullptr){
		m_pCvtFrame = av_frame_alloc();
		if(m_pCvtFrame == nullptr){
			return DECODER_ERR_SYS;
		}
		m_pCvtBuffer = (unsigned char *)av_malloc_array(1, av_image_get_buffer_size(AV_PIX_FMT_YUV420P,  m_stVideoFormatDst.m_iWidth, m_stVideoFormatDst.m_iHeight,1));
		av_image_fill_arrays(m_pCvtFrame->data, m_pCvtFrame->linesize,m_pCvtBuffer, AV_PIX_FMT_YUV420P, m_stVideoFormatDst.m_iWidth, m_stVideoFormatDst.m_iHeight,1);
	}
	return DECODER_ERR_OK;

}

void DecoderFFmpeg::writeScaledFrameData(FrameData* data)
{
	int fmt = getPixelFormat(m_stVideoFormatDst.m_iPixFmt);
	switch(fmt){
	case AV_PIX_FMT_YUV420P:
		{
			int ysize = m_stVideoFormatDst.m_iWidth * m_stVideoFormatDst.m_iHeight;
			int usize = ysize / 4;
			int vsize = ysize / 4;
			data->mallocFrame(ysize + usize + vsize);

			tuint8* tmp = data->m_pFrame;
			memcpy(tmp, m_pCvtFrame->data[0], ysize);
			tmp += ysize;

			memcpy(tmp, m_pCvtFrame->data[1], usize);
			tmp += usize;

			memcpy(tmp, m_pCvtFrame->data[1], vsize);
			tmp += vsize;
		}
		break;
	default:
		break;
	}
}

void DecoderFFmpeg::writeOriginalFrameData(FrameData* data)
{
	int yh = m_pFrame->height;
	int uh = yh / 2;
	int vh = yh / 2;

	int yw = m_pFrame->width;
	int uw = yw / 2;
	int vw = yw / 2;

	data->mallocFrame(yh*yw + uh*uw + vh*vw);

	tuint8* tmp = data->m_pFrame;
	for(int i=0; i<yh; i++){
		memcpy(tmp, m_pFrame->data[0]+m_pFrame->linesize[0]*i, yw);
		tmp += yw;
	}

	for(int i=0; i<uh; i++){
		memcpy(tmp, m_pFrame->data[1]+m_pFrame->linesize[1]*i, uw);
		tmp += uw;
	}

	for(int i=0; i<vh; i++){
		memcpy(tmp, m_pFrame->data[2]+m_pFrame->linesize[2]*i, vw);
		tmp += vw;
	}
}

void DecoderFFmpeg::printPacketType(int typeId)
{
	switch(typeId){
	case AV_PICTURE_TYPE_I:
		printf("frame I\n");
		break;
	case AV_PICTURE_TYPE_B:
		printf("frame B\n");
		break;
	case AV_PICTURE_TYPE_P:
		printf("frame P\n");
		break;
	default:
		printf("frame O\n");
		break;
	}
}