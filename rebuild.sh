cd ~/Projects/SpeedConverter
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ninja
cd ~/Projects/SpeedConverter
ln -sf build/compile_commands.json .