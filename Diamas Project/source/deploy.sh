#!/bin/sh
echo "Starting deployment\n"
echo "Killing test server"

killall -9 game_server db vrunner auth game
kill `ps | grep auto.sh | awk '{print $1}'`

if [ ! $? -eq 0 ]; then
	echo "Could not kill test server"
fi

echo "Packing" 

cd bin/server/

if [ ! $? -eq 0 ]; then
	echo "Could find binary folder"
fi

tar cfz ../bin.tgz *

if [ ! $? -eq 0 ]; then
	echo "Could pack binaries"
	exit 1
fi


cp ../bin.tgz $SERVER_DIR/server/share/bin/bin.tgz

if [ ! $? -eq 0 ]; then
	echo "Could not copy binary archive to $SERVER_DIR/server/share/bin"
fi


cd $SERVER_DIR/server/share/bin/
tar xvfz bin.tgz 

if [ ! $? -eq 0 ]; then
	echo "Could not extract binaries to $SERVER_DIR/server/share/bin"
fi

mv game game_server

if [ ! $? -eq 0 ]; then
	echo "Could not rename the binary game to game_server"
fi
rm -rf bin.tgz 
if [ ! $? -eq 0 ]; then
	echo "Could remove bin.tgz"
fi

cd ../../

sh clear.sh

echo "Deployed"
curl "https://api.telegram.org/bot1019998125:AAFdDFWWGjHTOGT93VRGUxfEa9Ru600Q9QA/sendMessage?chat_id=-1001408529110&text=Server%20Deploy%20done"
