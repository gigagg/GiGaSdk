mkdir %1deps\Debug\lib
mkdir %1deps\Debug\bin
mkdir %1deps\Debug\include
mkdir %1deps\Release\lib
mkdir %1deps\Release\bin
mkdir %1deps\Release\include
mkdir %1deps\include

REM boost
for /f %%f in ('dir /b %1\packages\boost_*') do xcopy %1packages\%%f\lib\native\address-model-32\lib %1deps\Debug\lib /y /s
for /f %%f in ('dir /b %1\packages\boost_*') do xcopy %1packages\%%f\lib\native\address-model-32\lib %1deps\Release\lib /y /s

REM for /f %%f in ('dir /b %1\packages\boost_*') do xcopy %1packages\%%f\lib\native\address-model-32\lib\*.dll %1deps\Debug\bin /y /s
REM for /f %%f in ('dir /b %1\packages\boost_*') do xcopy %1packages\%%f\lib\native\address-model-32\lib\*.dll %1deps\Release\bin /y /s

for /f %%f in ('dir /b %1\packages\boost*') do xcopy %1packages\%%f\lib\native\include %1deps\include /y /s

REM casablanca
for /f %%f in ('dir /b %1\packages\cpprestsdk.v140.windesktop.msvcstl.dyn.rt-dyn*') do xcopy %1packages\%%f\lib\native\v140\windesktop\msvcstl\dyn\rt-dyn\x86\Debug   %1deps\Debug\lib /y /s
for /f %%f in ('dir /b %1\packages\cpprestsdk.v140.windesktop.msvcstl.dyn.rt-dyn*') do xcopy %1packages\%%f\lib\native\v140\windesktop\msvcstl\dyn\rt-dyn\x86\Release %1deps\Release\lib /y /s

for /f %%f in ('dir /b %1\packages\cpprestsdk.v140.windesktop.msvcstl.dyn.rt-dyn*') do xcopy %1packages\%%f\lib\native\v140\windesktop\msvcstl\dyn\rt-dyn\x86\Debug\*.dll   %1deps\Debug\bin /y /s
for /f %%f in ('dir /b %1\packages\cpprestsdk.v140.windesktop.msvcstl.dyn.rt-dyn*') do xcopy %1packages\%%f\lib\native\v140\windesktop\msvcstl\dyn\rt-dyn\x86\Release\*.dll %1deps\Release\bin /y /s

for /f %%f in ('dir /b %1\packages\cpprestsdk.v140.windesktop.msvcstl.dyn.rt-dyn*') do xcopy %1packages\%%f\build\native\include %1deps\include /y /s

REM cryptopp
REM for /f %%f in ('dir /b %1\packages\cryptopp.v140.windesktop.msvcstl.dyn.rt-dyn-Win32*') do xcopy %1packages\%%f\lib\native\v140\windesktop\msvcstl\Win32\Debug\md   %1deps\Debug\lib /y /s
REM for /f %%f in ('dir /b %1\packages\cryptopp.v140.windesktop.msvcstl.dyn.rt-dyn-Win32*') do xcopy %1packages\%%f\lib\native\v140\windesktop\msvcstl\Win32\Release\md %1deps\Release\lib /y /s
REM for /f %%f in ('dir /b %1\packages\cryptopp.v140.windesktop.msvcstl.dyn.rt-dyn-Win32*') do xcopy %1packages\%%f\lib\native\v140\windesktop\msvcstl\Win32\Debug      %1deps\Debug\lib /y /s
REM for /f %%f in ('dir /b %1\packages\cryptopp.v140.windesktop.msvcstl.dyn.rt-dyn-Win32*') do xcopy %1packages\%%f\lib\native\v140\windesktop\msvcstl\Win32\Release    %1deps\Release\lib /y /s

REM for /f %%f in ('dir /b %1\packages\cryptopp.v140.windesktop.msvcstl.dyn.rt-dyn-Win32*') do xcopy %1packages\%%f\lib\native\v140\windesktop\msvcstl\Win32\Debug\md\*.dll   %1deps\Debug\bin /y /s
REM for /f %%f in ('dir /b %1\packages\cryptopp.v140.windesktop.msvcstl.dyn.rt-dyn-Win32*') do xcopy %1packages\%%f\lib\native\v140\windesktop\msvcstl\Win32\Release\md\*.dll %1deps\Release\bin /y /s

REM for /f %%f in ('dir /b %1\packages\cryptopp.5*') do xcopy %1packages\%%f\build\native\include %1deps\include /y /s

REM openssl
for /f %%f in ('dir /b %1\packages\openssl.v140.windesktop.msvcstl.dyn.rt-dyn.x86*') do xcopy %1packages\%%f\lib\native\v140\windesktop\msvcstl\dyn\rt-dyn\x86\debug   %1deps\Debug\lib /y /s
for /f %%f in ('dir /b %1\packages\openssl.v140.windesktop.msvcstl.dyn.rt-dyn.x86*') do xcopy %1packages\%%f\lib\native\v140\windesktop\msvcstl\dyn\rt-dyn\x86\release %1deps\Release\lib /y /s

for /f %%f in ('dir /b %1\packages\openssl.v140.windesktop.msvcstl.dyn.rt-dyn.x86*') do xcopy %1packages\%%f\lib\native\v140\windesktop\msvcstl\dyn\rt-dyn\x86\debug\*.dll   %1deps\Debug\bin /y /s
for /f %%f in ('dir /b %1\packages\openssl.v140.windesktop.msvcstl.dyn.rt-dyn.x86*') do xcopy %1packages\%%f\lib\native\v140\windesktop\msvcstl\dyn\rt-dyn\x86\release\*.dll %1deps\Release\bin /y /s

for /f %%f in ('dir /b %1\packages\openssl.v140.windesktop.msvcstl.dyn.rt-dyn.x86*') do xcopy %1packages\%%f\build\native\include %1deps\include /y /s

REM curl
for /f %%f in ('dir /b %1\packages\rmt_curl.*') do xcopy %1packages\%%f\build\native\lib\v140\Win32\Debug\dynamic     %1deps\Debug\lib /y /s
for /f %%f in ('dir /b %1\packages\rmt_curl.*') do xcopy %1packages\%%f\build\native\lib\v140\Win32\Release\dynamic   %1deps\Release\lib /y /s

for /f %%f in ('dir /b %1\packages\rmt_curl.*') do xcopy %1packages\%%f\build\native\bin\v140\Win32\Debug\dynamic     %1deps\Debug\bin /y /s
for /f %%f in ('dir /b %1\packages\rmt_curl.*') do xcopy %1packages\%%f\build\native\bin\v140\Win32\Release\dynamic   %1deps\Release\bin /y /s

for /f %%f in ('dir /b %1\packages\rmt_curl.*') do xcopy %1packages\%%f\build\native\include   %1deps\include /y /s

REM zlib
for /f %%f in ('dir /b %1\packages\rmt_zlib.*') do xcopy %1packages\%%f\build\native\lib\v140\Win32\Debug\dynamic   %1deps\Debug\lib /y /s
for /f %%f in ('dir /b %1\packages\rmt_zlib.*') do xcopy %1packages\%%f\build\native\lib\v140\Win32\Release\dynamic   %1deps\Release\lib /y /s

for /f %%f in ('dir /b %1\packages\rmt_zlib.*') do xcopy %1packages\%%f\build\native\bin\v140\Win32\Debug\dynamic   %1deps\Debug\bin /y /s
for /f %%f in ('dir /b %1\packages\rmt_zlib.*') do xcopy %1packages\%%f\build\native\bin\v140\Win32\Release\dynamic   %1deps\Release\bin /y /s

for /f %%f in ('dir /b %1\packages\rmt_zlib.*') do xcopy %1packages\%%f\build\native\include %1deps\include /y /s

REM libssh2
for /f %%f in ('dir /b %1\packages\rmt_libssh2.*') do xcopy %1packages\%%f\build\native\lib\v140\Win32\Debug\dynamic   %1deps\Debug\lib /y /s
for /f %%f in ('dir /b %1\packages\rmt_libssh2.*') do xcopy %1packages\%%f\build\native\lib\v140\Win32\Release\dynamic   %1deps\Release\lib /y /s

for /f %%f in ('dir /b %1\packages\rmt_libssh2.*') do xcopy %1packages\%%f\build\native\bin\v140\Win32\Debug\dynamic   %1deps\Debug\bin /y /s
for /f %%f in ('dir /b %1\packages\rmt_libssh2.*') do xcopy %1packages\%%f\build\native\bin\v140\Win32\Release\dynamic   %1deps\Release\bin /y /s

for /f %%f in ('dir /b %1\packages\rmt_libssh2.*') do xcopy %1packages\%%f\build\native\include %1deps\include /y /s

