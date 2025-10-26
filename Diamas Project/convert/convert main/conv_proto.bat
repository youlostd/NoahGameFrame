@echo off
echo Converting item proto
dump_proto.exe --action server-item-old --input item_proto.txt --output ../converting-server/item_proto_server
xcopy /s/h/e/k/f/c/y "../converting-server/item_proto_server" "../converting-pack-root-data/item_proto"
echo Converting mob_proto
dump_proto.exe --action server-mob-old --input mob_proto.txt --output ../converting-server/mob_proto_server
xcopy /s/h/e/k/f/c/y "../converting-server/mob_proto_server" "../converting-pack-root-data/mob_proto"
echo Converting object proto
dump_proto.exe --action object --input object-proto.xml --output ../converting-server/object_proto_server
xcopy /s/h/e/k/f/c/y "../converting-server/object_proto_server" "../converting-pack-root-data/object_proto"
echo Converting skill proto
dump_proto.exe --action skill --input skill-proto.xml --output ../converting-server/skill_proto_server
xcopy /s/h/e/k/f/c/y "../converting-server/skill_proto_server" "../converting-pack-root-data/skill_proto"
echo Converting item_attr
dump_proto.exe --action item-attr --input item-attr.xml --output ../converting-server/item_attr_proto_server
xcopy /s/h/e/k/f/c/y "../converting-server/item_attr_proto_server" "../converting-pack-root-data/item_attr_proto"
echo Converting item attr rare
dump_proto.exe --action item-attr --input item-attr-rare.xml --output ../converting-server/item_attr_rare_proto_server
xcopy /s/h/e/k/f/c/y "../converting-server/item_attr_rare_proto_server" "../converting-pack-root-data/item_attr_rare_proto"
echo Converting cube
dump_proto.exe --action cube --input cube-proto.xml --output ../converting-server/cube_proto
xcopy /s/h/e/k/f/c/y "../converting-server/cube_proto" "../converting-pack-root-data/cube_proto"
echo Converting refine
dump_proto.exe --action refine --input refine-proto.xml --output ../converting-server/refine_proto


REM echo Adding to git
REM git add ../converting server/item_proto_server
REM git add ../converting server/mob_proto_server
REM git add ../converting server/object_proto_server
REM git add ../converting server/skill_proto_server
REM git add ../converting server/item_attr_proto_server
REM git add ../converting server/item_attr_rare_proto_server
REM git add ../converting server/cube_proto
REM git add ../converting server/refine_proto_server