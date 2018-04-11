#include "DecoderImpl.h"
#include "DecoderFFmpeg.h"

//DecoderImpl::DecoderImpl()
//{
//
//}
//
//DecoderImpl::~DecoderImpl()
//{
//	if(decoderEngine != nullptr)
//		delete decoderEngine;
//}
//
//DecoderBase* DecoderImpl::createDecoderWith(CodecEngine env)
//{
//	switch(env){
//	case ffmpeg:
//		decoderEngine = new DecoderFFmpeg();
//		break;
//	default:
//		if(decoderEngine != nullptr){
//			delete decoderEngine;
//			decoderEngine = nullptr;
//		}
//		break;;
//	}
//
//	return decoderEngine;
//}
//
//void DecoderImpl::releaseDecoder(DecoderBase* decoder)
//{
//	delete decoderEngine;
//}


DecoderBase* createDecoderWith(CodecEngine env)
{
	DecoderBase* decoderEngine = nullptr;

	switch(env){
	case ffmpeg:
		decoderEngine = new DecoderFFmpeg();
		break;
	default:
		break;
	}
	return decoderEngine;
}

void releaseDecoder(DecoderBase* decoder)
{
	delete decoder;
}