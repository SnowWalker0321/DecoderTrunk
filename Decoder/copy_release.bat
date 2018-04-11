
xcopy "DecoderImpl.h" 	"..\..\lib\release\Decoder\include" /Y /D 


xcopy "DecoderBase.h" 	"..\..\lib\release\Decoder\include" /Y /D 


xcopy "DecoderHeader.h" "..\..\lib\release\Decoder\include" /Y /D


xcopy "DecoderHeader.h"   "..\..\lib\release\Decoder\include" /Y /D

IF NOT EXIST "..\..\lib\release\Decoder\dll" mkdir "..\..\lib\release\Decoder\dll" > nul
xcopy "..\release\Decoder.dll"   "..\..\lib\release\Decoder\dll" /Y /D

IF NOT EXIST "..\..\lib\release\decoder\lib" mkdir "..\..\lib\release\decoder\lib" > nul
xcopy "..\release\Decoder.lib"   "..\..\lib\release\decoder\lib" /Y /D
