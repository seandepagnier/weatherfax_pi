REM "tp-copy.bat"  Local TP Frontend2 Template to local Plugin
REM Source Folder:        ..src\testplugin_pi
REM DestinationFolder:    ..src\[plugin_pi]
REM Batch File Location:  ..src\[plugin_pi] - Execute from here
REM Put this batch file in the Plugin Folder and execute it.
REM
@echo off
echo  Start script to copy Template Folders
pause   
echo .
xcopy ..\testplugin_pi\.circleci\* .\.circleci\ /s /v /y
xcopy ..\testplugin_pi\build-deps\* .\build-deps\ /s /v /y
xcopy ..\testplugin_pi\buildosx\* .\buildosx\ /s /v /y
xcopy ..\testplugin_pi\ci\* .\ci\ /v /y
xcopy ..\testplugin_pi\ci\extras\circleci-build-flatpak_extra_libs.txt .\ci\extras\ /v /y
xcopy ..\testplugin_pi\cmake\* .\cmake\ /s /v /y
xcopy ..\testplugin_pi\mingw\* .\mingw\ /s /v /y
xcopy ..\testplugin_pi\msvc\* .\msvc\ /s /v /y
echo Template Folders are copied
echo .
echo Start script to copy Template files. 
pause
echo .
xcopy ..\testplugin_pi\.clang-format .\ /v /y
xcopy ..\testplugin_pi\.cmake-format.yaml .\ /v /y
xcopy ..\testplugin_pi\.editorconfig .\ /v /y
xcopy ..\testplugin_pi\.gitignore .\ /v /y
xcopy ..\testplugin_pi\.travis.yml .\ /v /y
xcopy ..\testplugin_pi\appveyor.yml .\ /v /y
xcopy ..\testplugin_pi\bld.bat .\ /v /y
xcopy ..\testplugin_pi\build-local-package-example.sh .\ /v /y
xcopy ..\testplugin_pi\compile.bat .\ /v /y
xcopy ..\testplugin_pi\make-new-plugin.sh .\ /v /y
xcopy ..\testplugin_pi\run-circleci-local.txt .\ /v /y
xcopy ..\testplugin_pi\submod-up.bat .\ /v /y
xcopy ..\testplugin_pi\update_submodule_branch.sh .\ /v /y
echo Template Folders are copied
echo .
echo Template files are copied. 
echo .
echo Git status lists the files changed.
echo If it looks ok, execute add-files.bat
pause
git status
echo .
echo .
echo -----------------------------------------
echo Check and Compare CMakeLists.txt
echo -----------------------------------------
echo CMakeLists.txt needs to be compared and updated.
pause
echo Update the submodule if needed "submod-up.bat" 
echo Use "tp-add.bat"  to git add template files
echo .
echo  git add 
echo  gToit commit -am "[version] + TP[tpversion]"
echo  git tag v[version]
echo  git push --tags origin master
echo

Rem  XCOPY.EXE is built into Windows.
Rem  https://docs.microsoft.com/en-us/windows-server/administration/windows-commands/xcopy
Rem
Rem  xcopy /s c:\source d:\target
Rem
Rem  Copies files and directories, including subdirectories.
Rem
Rem  <Source> 	Required. Specifies the location and names of the files you want to copy. 
Rem    This parameter must include either a drive or a path.
Rem
Rem  [<Destination>] 	Specifies the destination of the files you want to copy. 
Rem    This parameter can include a drive letter and colon, a directory name, a file name, or a combination of these.
Rem
Rem    /s/e - recursive copy, including copying empty directories.
Rem    /s - Copies directories and subdirectories, unless they are empty. If you omit /s, xcopy works within a single directory.
Rem    /e -	Copy all subdirectories, even if they are empty. Use /e with the /s and /t options.
Rem    /v - verify the copy against the original.
Rem    /h - copy system and hidden files.
Rem    /k - copy read-only attributes, otherwise, all files become read-write.
Rem    /x - if you care about permissions, if you worry about permissions /o or /x.
Rem    /y - don't prompt before overwriting existing files.
Rem    /z - if you think the copy might fail and you want to restart it, use this. 
Rem      It places a marker on each file as it copies, 
Rem      so you can rerun the xcopy command to pick up from where it left off.
Rem
Rem  If you think the xcopy might fail partway through 
Rem  (like when you are copying over a flaky network connection), 
Rem  or that you have to stop it and want to continue it later, 
Rem  you can use xcopy /s/z c:\source d:\target.