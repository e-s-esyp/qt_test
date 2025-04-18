mkdir build
cp -r icons build
cd build
cmake .. -DCMAKE_PREFIX_PATH=~/Qt/6.6.1/gcc_64/lib/cmake
cmake --build .
./test
