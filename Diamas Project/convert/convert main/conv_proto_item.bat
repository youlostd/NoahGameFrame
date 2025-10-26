@echo off
echo Converting item proto
dump_proto.exe --action server-item-old --input item_proto.txt --output ../data/item_proto_server
xcopy /s/h/e/k/f/c/y ../data/item_proto_server ../../client/data/source/root/data/item_proto
echo Adding to git
git add ../data/item_proto_server