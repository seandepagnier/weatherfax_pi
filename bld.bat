cd build
cmake -T v141 ..
cmake --build . --target package --config release >output.txt
bash ./cloudsmith-upload.sh
