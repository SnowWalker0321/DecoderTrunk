/** 解码基类
 * 函数调用顺序
 *	1、构造
 *	2、createDecoder
 *	3、setDecodeFormat， 如果需要自定义解码输出的宽高， 可在解码过程中调用， 也可以不调用、使用原始视频的大小
 *	4、decodeVideo， 开始解码， 返回值为解码出来的视频帧的数量
 *	5、takeFirstFrame， 解码之后，读取视频帧， 需要手动调用release以释放空间
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

	// 输入为enum
	virtual int createDecoder(const int& codecId) = 0;
	virtual void destroyDecoder() = 0;

	// 设置解码格式（宽高）， 视频所需要转码的格式, 不设置使用默认的
	virtual int setDecodeFormat(const VideoFormat& fmt) = 0;

	// 获取解码之前、解码转换之后的视频格式
	virtual VideoFormat getOriginalVideoFormat() = 0;
	virtual VideoFormat getDecodedVideoFormat() = 0;

	// 开始解码, 0代表没有接触视频帧， 同时也没有异常发生
	virtual int decodeVideo(tuint8* buf, tuint32 bufLen) = 0;
	// 获取解码后的YUV数据 packet格式 Y->U->V
	virtual FrameData*	takeFirstFrame() = 0;
};
}
#endif

