#!/bin/bash

# FE2 Testplugin

# Use ${bold}"./build-win.sh c"${normal} to run cmake.
# Use "./build.sh c pi" to run cmake and copy a DLL for testing.
# Adjust this command for your setup and Plugin.
# Requires wxWidgets setup
# - /home/fcgle/source/wxWidgets-3.1.2
# - /home/fcgle/source/wxWidgets-3.2.2
# - /home/fcgle/source/ is where all the plugins and OpenCPN repos are kept.
# Visual Studio 15 2017 installed
# Visual Studio 17 2022 installed
# --------------------------------------
# For Opencpn 5.8 and wxWidgets-3.2.2 using Visual Studio 15 2017
# --------------------------------------
# Used for local builds and testing.
# Create an empty "[plugin]/build" directory
# Use a terminal from the [plugin] root directory: "./bld.sh"
# Find the errors in the build/output.txt file
# Then use bash prompt to run cloudsmith-upload.sh command: "bash ./bld.sh"
# This adds the metadata file to the tarball gz file.
# Set local environment to find and use wxWidgets-3.2.2

set -x  # Enable command tracing

wxDIR=$WXWIN
wxWidgets_ROOT_DIR=$WXWIN
wxWidgets_LIB_DIR="$WXWIN/lib/vc14x_dll"
WXWIN="/home/fcgle/source/wxWidgets-3.2.2"

# For Opencpn 5.8 and wxWidgets-3.2.2

# cd build || exit
# find the build directory whereever it is.
BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/build"
cd "$BUILD_DIR"

if [ "$1" == "c" ]; then
    cmake -T v143 -A Win32 -DOCPN_TARGET=MSVC ..
    cmake --build . --target package --config relwithdebinfo >output.txt
fi

# Alternative lines do the same thing.
# if [ "$1" == "c" ]; then
#     cmake -A Win32 -G "Visual Studio 17 2022" -DCMAKE_GENERATOR_PLATFORM=Win32 ..
#     cmake --build . --config Release
# fi

# Bash script completes tarball prep adding metadata into it.
bash ./cloudsmith-upload.sh

# Example used to copy a plugin DLL for testing. Adjust the paths and plugin name.
if [ "$1" == "pi" ]; then
    cp /home/radar/AutoTrackRaymarine_pi/build/Release/autotrackraymarine_pi.dll "/home/$USER/.local/share/opencpn/plugins"
fi

# Find ${bold}"build/output.txt"${normal} file if the build is not successful.
# Other examples below.
