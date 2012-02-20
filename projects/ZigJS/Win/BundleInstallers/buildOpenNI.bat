candle installerOpenNI.wxs
IF %ERRORLEVEL% GTR 0 goto error
light -ext WixBalExtension -o %1\ZigJSOpenNI.exe installerOpenNI.wixobj
IF %ERRORLEVEL% GTR 0 goto error
set /p passphrase= <../../../../../codesign/passphrase.txt
"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Bin\signtool.exe" sign /f ../../../../../codesign/zigcert.pfx /p %passphrase% /t http://timestamp.digicert.com/ %1\ZigJSOpenNI.exe
goto quit

:error
echo Failed creating OpenNI bundle :(
:quit