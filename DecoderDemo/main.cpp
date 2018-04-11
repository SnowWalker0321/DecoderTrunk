#include <iostream>

#include "DecoderImpl.h"
using namespace decoder;

int main(int argc, char** argv)
{
	////////////////////////////////////////////////////////////////////////
	FILE* fp = fopen("../../_rc/test.hevc", "rb");
	if(fp == nullptr)
		return 0;
	FILE* wfp = nullptr;
	DecoderBase* decoder = createDecoderWith(ffmpeg);
	int ret = decoder->createDecoder(h265);
	if(ret != DECODER_ERR_OK)
		return 0;

	VideoFormat fmt;
	fmt.m_iWidth = 960;
	fmt.m_iHeight = 540;
	ret = decoder->setDecodeFormat(fmt);

	bool firstFrame = true;
	int bufLen = 4096;
	tuint8* buf = new tuint8[bufLen];
	while(1){
		memset(buf, 0, bufLen);
		int len = fread(buf, 1, bufLen, fp);
		if(len == 0)
			break;

		int frmCount = decoder->decodeVideo(buf, len);
		if(frmCount <= 0)
			continue;

		if(firstFrame){
			firstFrame = false;
			VideoFormat fmt = decoder->getDecodedVideoFormat();
			char name[512] = {0};
			sprintf(name, "../../_rc/test264_%d_%d.yuv", fmt.m_iWidth, fmt.m_iHeight);
			wfp = fopen(name, "wb");
			if(wfp == nullptr)
				break;
		}
		for(int i=0; i<frmCount; i++){
			FrameData* frame = decoder->takeFirstFrame();
			fwrite(frame->m_pFrame, 1, frame->m_stHeader.m_iYUVSize, wfp);
			fflush(wfp);
			frame->Release();
		}
	}

	if(fp!= nullptr)
		fclose(fp);
	if(wfp != nullptr)
		fclose(wfp);
	delete decoder;
	return 0;
}