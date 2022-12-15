cd build
cmake -T v143 ..
cmake --build . --target package --config release >output.txt
bash ./cloudsmith-upload.sh
