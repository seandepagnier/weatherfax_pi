REM FE2  Testplugin

REM  Use "bldw c" to run cmake.
REM  Use "bldw c pi" to run cmake and copy a dll for testing. 
REM  Adjust this command for your setup and Plugin.
REM  Requires wxWidgets setup
REM  C:\Users\fcgle\source\wxWidgets-3.1.2
REM  C:\Users\fcgle\source\wxWidgets-3.2.2
REM  C:\Users\fcgle\source\ is where all the plugins
REM        and OpenCPN repos are kept.
REM  Visual Studio 15 2017  installed
REM  Visual Studio 17 2022  installed
REM --------------------------------------
REM  For Opencpn 5.8 and wxWidgets-3.2.2 using Visual Studio 15 2017
REM --------------------------------------
REM Used for local builds and testing.
REM Create an empty "[plugin]/build" directory 
REM Use MSVC Command Prompt from [plugin]root directory   ".\bld.bat"
REM Find the errors in the build\output.txt file
REM Then use bash prompt to run cloudsmith-upload.sh command  "BASH ./bld.bat"
REM This adds the metadata file to the tarball gz file.
REM Set local environment to find and use wxWidgets-3.2.2

set "wxDIR=%WXWIN%"
set "wxWidgets_ROOT_DIR=%WXWIN%"
set "wxWidgets_LIB_DIR=%WXWIN%\lib\vc14x_dll" 
set "WXWIN=C:\Users\fcgle\source\wxWidgets-3.2.2"

REM  For Opencpn 5.8 and wxWidgets-3.2.2
cd build
if %1%==c cmake -T v143 -A Win32 -DOCPN_TARGET=MSVC ..
if %1%==c cmake --build . --target package --config relwithdebinfo >output.txt

REM  Alternative lines do the same thing.
REM  if %1%==c cmake -A Win32 -G "Visual Studio 17 2022" -DCMAKE_GENERATOR_PLATFORM=Win32 ..
REM  if %1%==c cmake --build . --config Release

REM Bash script completes tarball prep adding metadata into it.
bash ./cloudsmith-upload.sh

REM Example used to copy a plugin dll for testing. Adjust the paths and plugin name.
if %1%==pi copy C:\radar\AutoTrackRaymarine_pi\build\Release\autotrackraymarine_pi.dll C:\Users\"Douwe Fokkema"\AppData\Local\opencpn\plugins

REM
REM Find build/output.txt file if the build is not successful.
REM Other examples below.
REM -------------------------------------
REM  For Opencpn 5.6.2 and wxWidgets-3.1.2  release
REM --------------------------------------
REM  Using built C:\Users\fcgle\source\wxWidgets-3.1.2 and local settings
REM set "wxDIR=%WXWIN%"
REM set "wxWidgets_ROOT_DIR=%WXWIN%" 
REM set "wxWidgets_LIB_DIR=%WXWIN%\lib\vc14x_dll" 
REM set "WXWIN=C:\Users\fcgle\source\wxWidgets-3.1.2"
REM cd build
REM cmake -T v141_xp -DOCPN_TARGET=MSVC ..
REM cmake --build . --target package --config release >output.txt
REM bash ./cloudsmith-upload.sh

REM --------------------------------------
REM  For Opencpn 5.8 and wxWidgets-3.2.2 using Visual Studio 15 2017
REM --------------------------------------
REM  Using built C:\Users\fcgle\source\wxWidgets-3.2.2 and local settings
REM set "wxDIR=%WXWIN%"
REM set "wxWidgets_LIB_DIR=%WXWIN%\lib\vc14x_dll" 
REM set "WXWIN=C:\Users\fcgle\source\wxWidgets-3.2.2"
REM cd build
REM cmake -T v141_xp -G -DOCPN_TARGET=MSVC "Visual Studio 15 2017"
REM       -DCMAKE_BUILD_TYPE=RelWithDebInfo  .. 
REM cmake --build . --target tarball --config RelWithDebInfo >output.txt
REM bash ./cloudsmith-upload.sh

REM --------------------------------------
REM  For Opencpn 5.8.2 and wxWidgets-3.2.2 using Visual Studio 12 2022
REM  TransmitterDAN full Build in repos2 which executes win_deps.bat
REM --------------------------------------
REM .\buildwin\win_deps.bat
REM cd build
REM cmake -T v143 -A Win32 -DOCPN_TARGET=MSVC "Visual Studio 17 2022" ^
REM      -DCMAKE_BUILD_TYPE=RelWithDebInfo  ..
REM cmake --build . --target tarball --config RelWithDebInfo >output.txt

REM  Setup for wxWidgets 3.2.2
REM  wxWidgets_INCLUDE_DIRS   %WXWIN%\include
REM  wxWidgets_LIB_DIR   %WXWIN%\lib\vc_dll
REM  wxWidgets_LIBRARIES    %WXWIN%\lib\vc_dll
REM  wxWidgets_ROOT_DIR   %WXWIN%
REM  WXWIN   C:\Users\fcgle\source\repos2\OpenCPN\cache\buildwxWidgets\

REM --------------------------------------
REM configdev58.bat   by Rick for local builds.
REM --------------------------------------
REM Can use  "call configdev58.bat" in another batch file to set up local wxWidgets environment
REM set "wxDIR=C:\Users\fcgle\source\repos2\OpenCPN\buildwin\..\cache\buildwxWidgets" 
REM set "wxWidgets_ROOT_DIR=C:\Users\fcgle\source\repos2\OpenCPN\buildwin\..\cache\buildwxWidgets" 
REM set "wxWidgets_LIB_DIR=C:\Users\fcgle\source\repos2\OpenCPN\buildwin\..\cache\buildwxWidgets\lib\vc_dll" 
