#ifndef DECODER_HEADER__H
#define DECODER_HEADER__H

#include "base_type_define.h"
#include "RefCounter.h"

#define MAX_INPUT_SIZE	4096

#define DECODER_ERR_OK		0	// ����
#define DECODER_ERR_SYS		-1	// FFMPEG �ڲ��쳣
#define DECODER_ERR_INNER	-2  // decoder �쳣


#pragma pack(4)
namespace decoder{
	enum CodecEngine{
		ffmpeg
	};
	enum CodecId{
		h264, 
		h265,
		mpeg2
	};

	enum FramePixFmt{
		none, 
		yuv420p,
		yv12,
	};
	
	class VideoFormat
	{
	public:
		// Ĭ�ϸ�ʽΪ 0*0
		VideoFormat() : m_iWidth(0), m_iHeight(0), m_iPixFmt(yuv420p){}
		tuint32 m_iWidth;
		tuint32 m_iHeight;

		tint32	m_iPixFmt;		// AVPixelFormat�� pixfmt.h, �Խ���֮���ת����Ч
		tint32	m_iCodecId;		// AVCodecID, avcodec.h�� ����
	};

	// ��ʱ����
	class VideoPack : public RefCounter
	{
	public:
		VideoPack(){
			memset(m_pInBuffer, 0, sizeof(m_pInBuffer));
			m_pInBufferSize = 0;
		}

		tuint8		m_pInBuffer[MAX_INPUT_SIZE + 16];
		tuint32		m_pInBufferSize;
	};

	// ����֮�����Ƶ��Ϣ
	struct FrameHeader{
		bool	m_isKeyFrame;	
		char	m_reserve[3];
		tuint32	m_iTotalLen;	// �ܳ��ȣ� ������ͷ, ��ʱ��Ч���漰���������л����õ�
		tuint32	m_iYUVSize;		// ��Ƶ֡�ĳ��ȣ� packet Y->U->V
	};

	class FrameData : public RefCounter
	{
	public:
		FrameData(){}
		~FrameData(){
			if(nullptr != m_pFrame){
				delete[] m_pFrame;
				m_pFrame = nullptr;
			}
		}

		void mallocFrame(int len){
			m_pFrame = new tuint8[len];
			m_stHeader.m_iTotalLen = len + sizeof(FrameHeader);
			m_stHeader.m_iYUVSize = len;
		}

		FrameHeader	m_stHeader;
		tuint8*		m_pFrame;
	};
}
#endif
