/** �������
 * ��������˳��
 *	1������
 *	2��createDecoder
 *	3��setDecodeFormat�� �����Ҫ�Զ����������Ŀ�ߣ� ���ڽ�������е��ã� Ҳ���Բ����á�ʹ��ԭʼ��Ƶ�Ĵ�С
 *	4��decodeVideo�� ��ʼ���룬 ����ֵΪ�����������Ƶ֡������
 *	5��takeFirstFrame�� ����֮�󣬶�ȡ��Ƶ֡�� ��Ҫ�ֶ�����release���ͷſռ�
 */
#ifndef DECODER_BASE__H
#define DECODER_BASE__H

#include "DecoderHeader.h"

namespace decoder{
class DecoderBase
{
public:
	DecoderBase(void){};
	~DecoderBase(void){};

	// ����Ϊenum
	virtual int createDecoder(const int& codecId) = 0;
	virtual void destroyDecoder() = 0;

	// ���ý����ʽ����ߣ��� ��Ƶ����Ҫת��ĸ�ʽ, ������ʹ��Ĭ�ϵ�
	virtual int setDecodeFormat(const VideoFormat& fmt) = 0;

	// ��ȡ����֮ǰ������ת��֮�����Ƶ��ʽ
	virtual VideoFormat getOriginalVideoFormat() = 0;
	virtual VideoFormat getDecodedVideoFormat() = 0;

	// ��ʼ����, 0����û�нӴ���Ƶ֡�� ͬʱҲû���쳣����
	virtual int decodeVideo(tuint8* buf, tuint32 bufLen) = 0;
	// ��ȡ������YUV���� packet��ʽ Y->U->V
	virtual FrameData*	takeFirstFrame() = 0;
};
}
#endif

