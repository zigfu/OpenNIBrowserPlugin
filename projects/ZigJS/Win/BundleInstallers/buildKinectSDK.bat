candle installerKinectSDK.wxs
IF %ERRORLEVEL% GTR 0 goto error
light -ext WixBalExtension -o %1\ZigJSKinectSDK.exe installerKinectSDK.wixobj
IF %ERRORLEVEL% GTR 0 goto error
REM thank you internet for this "variable" hack
set /p passphrase= <../../../../../codesign/passphrase.txt

"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Bin\signtool.exe" sign /f ../../../../../codesign/zigcert.pfx /p %passphrase% /t http://timestamp.digicert.com/ %1\ZigJSKinectSDK.exe
goto quit
:error
echo Failed creating KinectSDK bundle :(
:quit
