
REM install chocolatey
REM @powershell -NoProfile -ExecutionPolicy Bypass -Command "iex ((new-object net.webclient).DownloadString('https://chocolatey.org/install.ps1'))" && SET PATH=%PATH%;%ALLUSERSPROFILE%\chocolatey\bin
REM choco install nuget.commandline -y
REM choco install cmake -y
REM choco install visualstudio2015community -y
REM choco install devbox-sed -y

REM Get dependancies
pushd VS2015
nuget restore get_dependencies.sln
devenv get_dependencies.sln /build 
popd

REM Build curlcpp
mkdir curlcpp\build\release
pushd curlcpp\build\release
del CMakeCache.txt
cmake ../.. -DCMAKE_LIBRARY_PATH="../VS2015/deps/Release/lib" -DCMAKE_PREFIX_PATH="../VS2015/deps/" -DCMAKE_INSTALL_PREFIX="../../../VS2015/deps/Release"
devenv CURLCPP.sln /Rebuild Release
devenv CURLCPP.sln /Build Release /Project INSTALL.vcxproj
popd

mkdir curlcpp\build\debug
pushd curlcpp\build\debug
del CMakeCache.txt
cmake ../.. -DCMAKE_SYSTEM_INCLUDE_PATH="../VS2015/deps/include" -DCMAKE_PREFIX_PATH="../VS2015/deps/Debug" -DCMAKE_INSTALL_PREFIX="../../../VS2015/deps/Debug"
devenv CURLCPP.sln /Rebuild Debug
devenv CURLCPP.sln /Build Debug /Project INSTALL.vcxproj
popd

REM Build crypto++
pushd cryptopp\crypto++
devenv cryptest.sln /Upgrade
sed -i "s/>MultiThreaded</>MultiThreadedDLL</g" cryptlib.vcxproj
sed -i "s/>MultiThreaded</>MultiThreadedDLL</g" cryptdll.vcxproj
sed -i "s/>MultiThreaded</>MultiThreadedDLL</g" cryptest.vcxproj
sed -i "s/>MultiThreaded</>MultiThreadedDLL</g" dlltest.vcxproj
sed -i "s/>MultiThreadedDebug</>MultiThreadedDebugDLL</g" cryptlib.vcxproj
sed -i "s/>MultiThreadedDebug</>MultiThreadedDebugDLL</g" cryptdll.vcxproj
sed -i "s/>MultiThreadedDebug</>MultiThreadedDebugDLL</g" cryptest.vcxproj
sed -i "s/>MultiThreadedDebug</>MultiThreadedDebugDLL</g" dlltest.vcxproj
devenv cryptest.sln /Rebuild Debug
devenv cryptest.sln /Rebuild Release
popd

REM Build GiGaSdk
cd ..
mkdir build\release
pushd build\release
del CMakeCache.txt
cmake ../.. -DUSE_BOOST_LOG="1" -DCMAKE_BUILD_TYPE="Release"
devenv GiGaSdk.sln /Rebuild Release
devenv GiGaSdk.sln /Build Release /Project INSTALL.vcxproj
popd

mkdir build\debug
pushd build\debug
del CMakeCache.txt
cmake ../.. -DUSE_BOOST_LOG="1" -DCMAKE_BUILD_TYPE="Debug" -DUSE_DEV_GG=1
devenv GiGaSdk.sln /Rebuild Debug
devenv GiGaSdk.sln /Build Debug /Project INSTALL.vcxproj
popd