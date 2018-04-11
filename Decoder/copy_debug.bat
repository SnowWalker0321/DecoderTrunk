
xcopy "DecoderImpl.h" 	"..\..\lib\debug\Decoder\include" /Y /D 


xcopy "DecoderBase.h" 	"..\..\lib\debug\Decoder\include" /Y /D 


xcopy "DecoderHeader.h" "..\..\lib\debug\Decoder\include" /Y /D


xcopy "DecoderHeader.h"   "..\..\lib\debug\Decoder\include" /Y /D

IF NOT EXIST "..\..\lib\debug\Decoder\dll" mkdir "..\..\lib\debug\Decoder\dll" > nul
xcopy "..\Debug\Decoder.dll"   "..\..\lib\debug\Decoder\dll" /Y /D

IF NOT EXIST "..\..\lib\debug\decoder\lib" mkdir "..\..\lib\debug\decoder\lib" > nul
xcopy "..\Debug\Decoder.lib"   "..\..\lib\debug\decoder\lib" /Y /D