rm -Rf build
mkdir build
cd build &&
cmake ../src/dsp_server_c -DCMAKE_BUILD_TYPE=Debug
make
cd ..
