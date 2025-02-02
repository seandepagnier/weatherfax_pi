#!/bin/bash

# "tp-copy.sh" Local TP Frontend2 Template to local Plugin
# Source Folder:        ../src/testplugin_pi
# Destination Folder:   ../src/[plugin_pi]
# Script Location:      ../src/[plugin_pi] - Execute from here
# Put this script in the Plugin Folder and execute it.

echo "Start script to copy Template Folders"
read -p "Press [Enter] to continue..."
echo "."
cp -r ../testplugin_pi/.circleci/* ./.circleci/
cp -r ../testplugin_pi/build-deps/* ./build-deps/
cp -r ../testplugin_pi/buildosx/* ./buildosx/
cp ../testplugin_pi/ci/* ./ci/
cp ../testplugin_pi/ci/extras/circleci-build-flatpak_extra_libs.txt ./ci/extras/
cp -r ../testplugin_pi/cmake/* ./cmake/
cp -r ../testplugin_pi/mingw/* ./mingw/
cp -r ../testplugin_pi/msvc/* ./msvc/
echo "Template Folders are copied"
echo "."
echo "Start script to copy Template files."
read -p "Press [Enter] to continue..."
echo "."
cp ../testplugin_pi/.clang-format ./
cp ../testplugin_pi/.cmake-format.yaml ./
cp ../testplugin_pi/.editorconfig ./
cp ../testplugin_pi/.gitignore ./
cp ../testplugin_pi/.travis.yml ./
cp ../testplugin_pi/appveyor.yml ./
cp ../testplugin_pi/bld.bat ./
cp ../testplugin_pi/build-local-package-example.sh ./
cp ../testplugin_pi/compile.bat ./
cp ../testplugin_pi/make-new-plugin.sh ./
cp ../testplugin_pi/run-circleci-local.txt ./
cp ../testplugin_pi/submod-up.bat ./
cp ../testplugin_pi/update_submodule_branch.sh ./
echo "Template Files are copied"
echo "."
echo "Template files are copied."
echo "."
echo "Git status lists the files changed."
echo "If it looks ok, execute add-files.sh"
read -p "Press [Enter] to continue..."
git status
echo "."
echo "."
echo "-----------------------------------------"
echo "Check and Compare CMakeLists.txt"
echo "-----------------------------------------"
echo "CMakeLists.txt needs to be compared and updated."
read -p "Press [Enter] to continue..."
echo "Update the submodule if needed 'submod-up.sh'"
echo "."
echo "Then Commit the changes"
echo "$ git add"
echo "$ git commit -am (version) + TP (version)"
echo "$ git tag v[version]"
echo "$ git push --tags origin master"
