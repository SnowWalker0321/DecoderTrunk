#ifndef DECODER_IMPL__H
#define DECODER_IMPL__H

#ifdef DecoderDllExport
#define DecoderDllExport __declspec(dllexport)
#else
#define DecoderDllExport __declspec(dllimport)
#endif

#include "DecoderHeader.h"
#include "DecoderBase.h"
using namespace decoder;

//class DecoderDllExport DecoderImpl
//{
//public:
//	DecoderImpl();
//	~DecoderImpl();
//	DecoderBase* createDecoderWith(CodecEngine env);
//	void		 releaseDecoder(DecoderBase* decoder);
//
//private:
//	DecoderBase* decoderEngine;
//};

DecoderDllExport DecoderBase* createDecoderWith(CodecEngine env);
DecoderDllExport void releaseDecoder(DecoderBase* decoder);

#endif
