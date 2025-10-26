@echo off
echo Converting mob_proto
dump_proto.exe --action server-mob-old --input mob_proto.txt --output ../data/mob_proto_server
xcopy /s/h/e/k/f/c/y ../data/mob_proto_server ../../client/data/source/root/data/mob_proto
echo Adding to git
git add ../data/mob_proto_server