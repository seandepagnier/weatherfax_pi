cd build
cmake -T v141_xp -DOCPN_TARGET=MSVC ..
cmake --build . --target package --config release >output.txt
bash ./cloudsmith-upload.sh
