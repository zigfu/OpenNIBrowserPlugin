@echo off
REM usage: param 1 should be path to the MSI to include, param 2 should be
REM        the output directory with no trailing slash
REM example usage from my system:
REM build.bat ..\..\..\..\build\bin\ZigJS\Release\ZigJSv1.0.0.msi ..\..\..\..\build\bin\ZigJS\Release

REM set the path to the source MSI
set ZIGJSMSI=%1
call buildOpenNI.bat %2
call buildKinectSDK.bat %2
