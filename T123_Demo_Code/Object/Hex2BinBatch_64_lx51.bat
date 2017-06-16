cd Object
IF EXIST %1.bin del %1.bin
IF EXIST %1_tw.bin del %1_tw.bin

set /a current=0

IF EXIST %1.Hex Hex2Bin.exe %1.Hex

IF EXIST %1.bin cnp_v1.3.exe -r %1.bin %1_tw.bin %2

IF EXIST combine.bin del combine.bin
IF EXIST patch.pch del patch.pch
IF EXIST %1_tw.bin combine_oj_v1.11.exe %1_tw.bin t123_res.bin