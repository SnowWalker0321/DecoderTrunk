/** �ɴ�������ΪH.264  H.265���ݽ��н���
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
	// ��enum ����createDecoder
	//enum{
	//	h264 = AV_CODEC_ID_H264,
	//	h265 = AV_CODEC_ID_H265,
	//	mpeg2 = AV_CODEC_ID_MPEG2VIDEO
	//};

public:
	DecoderFFmpeg(void);
	~DecoderFFmpeg(void);
	
	// �������������ͷŽ�����
	int createDecoder(const int& codecId);
	void destroyDecoder();

	// ���ý����ʽ����ߣ��� ��Ƶ����Ҫת��ĸ�ʽ, ������ʹ��Ĭ�ϵ�
	int setDecodeFormat(const VideoFormat& fmt);
	
	// ��ȡ����֮ǰ������ת��֮�����Ƶ��ʽ
	VideoFormat getOriginalVideoFormat();
	VideoFormat getDecodedVideoFormat();

	// ��ʼ����, 0����û�нӴ���Ƶ֡�� ͬʱҲû���쳣����
	int decodeVideo(tuint8* buf, tuint32 bufLen);
	// ��ȡ������YUV���� packet��ʽ Y->U->V
	FrameData*	takeFirstFrame();

	//// �첽����VideoPack, ��ʱ��ʵ�־���ҵ��(��Ҫ��һ�������̣߳�ר���ڽ���)
	//void setPackToDecoder(tuint8* buf, tuint32	bufLen);
	//// ʹ�ý���֮����Ҫ����release �����ͷţ� ��Ҫֱ�ӵ���delete
	//VideoPack* takeFirstPack();

private:
	AVCodecID	getCodecId(const int& codecId );
	CodecId		getCodecId2(AVCodecID id);
	AVPixelFormat getPixelFormat(const int& pixFmt);
	FramePixFmt	  getPixelFormat2(AVPixelFormat fmt);
	
	// ��ȡ������
	AVCodec*	getCodec(int codecId);

	// ����ת�����
	int	createCvtCtx();

	// ��ӡ��ǰ֡����
	void printPacketType(int typeId);

	// ������Ƶ֡YUV��ʽд�������֡
	void writeScaledFrameData(FrameData* data);
	// д��ԭʼ��Ƶ֡
	void writeOriginalFrameData(FrameData* data);

private:
	// �����죬��ֹ���ע�ᣬӰ��Ч��
	CLock		m_decoderLock;

	// �������
	AVCodecID	m_iCodecId;
	AVCodec*	m_pCodec;
	AVCodecContext*	m_pCodecCtx;
	AVCodecParserContext*	m_pCodecParserCtx;
	AVFrame*	m_pFrame;
	

	///* ����Դ
	//*	ʹ��m_lsVideoPack�� ��������ⲿ���ݿ������ýṹ���У��ⲿ���ݼ����ͷţ� �����첽����
	//*   Ҳ����ֱ��ʹ���ⲿ������Դ��ͬ������
	//*/
	//CLock	m_videoPackLock;
	//std::list<VideoPack*>	m_videoPackLst;
	
	// ����֮���֡�任
	struct SwsContext*		m_pImgCvtCtx;
	AVFrame*	m_pCvtFrame;
	tuint8*		m_pCvtBuffer;
	
	// ԭ��Ƶ��ʽ������ת��֮�����Ƶ��ʽ�� ԭ�޷��޸ģ� ����֮����޸�
	VideoFormat	m_stVideoFormatSrc;
	VideoFormat	m_stVideoFormatDst;	

	// ���� ֮�󱣴����������, ���������������ܴ��һ��buffer����Щbuffer���кܶ�֡
	CLock		m_frameLock;
	list<FrameData*>	m_frameDataLst;	

private:
	static bool m_isRegisteredCodec;
};
}
#endif