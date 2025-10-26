if [ ! $? -eq 0 ]; then
 echo "Build folder does exist"
fi

curl "https://api.telegram.org/bot1019998125:AAFdDFWWGjHTOGT93VRGUxfEa9Ru600Q9QA/sendMessage?chat_id=-1001408529110&text=Server%20AsanBuild%20started"

CXX=clang++11 
CC=clang11

export CXX
export CC

cmake -DCMAKE_BUILD_TYPE=asan -DMETIN2_VENDOR_PATH=$VENDOR_DIR -G Ninja

if [ ! $? -eq 0 ]; then
curl "https://api.telegram.org/bot1019998125:AAFdDFWWGjHTOGT93VRGUxfEa9Ru600Q9QA/sendMessage?chat_id=-1001408529110&text=CMake%20Build%20failed"
 echo "Cmake was not successfull"
 exit 1
fi
ninja
if [ ! $? -eq 0 ]; then
curl "https://api.telegram.org/bot1019998125:AAFdDFWWGjHTOGT93VRGUxfEa9Ru600Q9QA/sendMessage?chat_id=-1001408529110&text=Server%20Build%20failed"
 echo "Could not fully build"
 exit 2
fi

curl "https://api.telegram.org/bot1019998125:AAFdDFWWGjHTOGT93VRGUxfEa9Ru600Q9QA/sendMessage?chat_id=-1001408529110&text=Server%20Build%20done"
