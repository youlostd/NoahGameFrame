if [ ! $? -eq 0 ]; then
 echo "Build folder does exist"
fi

curl "Start source building"

CXX=clang++23
CC=clang17

export CXX
export CC

cmake -DCMAKE_BUILD_TYPE=Release -DMETIN2_VENDOR_PATH=$VENDOR_DIR -G Ninja

if [ ! $? -eq 0 ]; then
 echo "Cmake was not successfull"
 exit 1
fi
ninja
if [ ! $? -eq 0 ]; then
 echo "Could not fully build"
 exit 2
fi

 echo "Succesfull fully build"