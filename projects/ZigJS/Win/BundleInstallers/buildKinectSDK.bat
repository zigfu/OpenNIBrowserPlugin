candle installerKinectSDK.wxs
IF %ERRORLEVEL% GTR 0 goto error
light -ext WixBalExtension -o %1\ZigJSKinectSDK_unsigned.exe installerKinectSDK.wixobj
IF %ERRORLEVEL% GTR 0 goto error

REM thank you internet for this "variable" hack
set /p passphrase= <../../../../../codesign/passphrase.txt
REM unpack bootstrapper engine, sign it and then place the signed version in the bundle
insignia -ib %1\ZigJSKinectSDK_unsigned.exe -o engine.exe
"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Bin\signtool.exe" sign /f ../../zigcert.pfx /p %passphrase% /t http://timestamp.digicert.com/ engine.exe
insignia -ab engine.exe %1\ZigJSKinectSDK_unsigned.exe -o %1\ZigJSKinectSDK.exe
REM sleep for 1 sec because insignia doesn't shut down properly
ping 127.0.0.1 -n 2 -w 1000
REM sign the bundle with the signed bootstrapper
"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Bin\signtool.exe" sign /f ../../zigcert.pfx /p %passphrase% /t http://timestamp.digicert.com/ %1\ZigJSKinectSDK.exe
IF %ERRORLEVEL% GTR 0 goto error
goto quit
:error
echo Failed creating KinectSDK bundle :(
:quit
