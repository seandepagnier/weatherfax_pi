cd build
cmake -T v143 -A Win32 -DOCPN_TARGET=MSVC ..
cmake --build . --target package --config relwithdebinfo >output.txt
bash ./cloudsmith-upload.sh
