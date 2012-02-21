candle installerKinectSDK.wxs
IF %ERRORLEVEL% GTR 0 goto error
light -ext WixBalExtension -o %1\ZigJSKinectSDK.exe installerKinectSDK.wixobj
IF %ERRORLEVEL% GTR 0 goto error
echo sleeping for 5 seconds to ensure ZigJSOpenNI.exe is not 
REM creating a vbs script to sleep for 1 second because otherwise signing fails
> "%Temp%.\sleep.vbs" ECHO WScript.Sleep 5000
CSCRIPT //NoLogo "%Temp%.\sleep.vbs"
DEL "%Temp%.\sleep.vbs"

REM thank you internet for this "variable" hack
set /p passphrase= <../../../../../codesign/passphrase.txt

"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Bin\signtool.exe" sign /f ../../../../../codesign/zigcert.pfx /p %passphrase% /t http://timestamp.digicert.com/ %1\ZigJSKinectSDK.exe
goto quit
:error
echo Failed creating KinectSDK bundle :(
:quit
