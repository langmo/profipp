mkdir -p debug
cd debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
sudo cmake --install .