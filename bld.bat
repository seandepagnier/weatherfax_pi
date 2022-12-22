cd build
cmake -T v143 -A Win32 ..
cmake --build . --target package --config relwithdebinfo >output.txt
bash ./cloudsmith-upload.sh
