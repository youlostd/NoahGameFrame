echo "Converting item proto"
dump_proto --action server-item-old --input item_proto.txt --output ../data/item_proto_server
echo "Converting mob_proto"
dump_proto --action server-mob-old --input mob_proto.txt --output ../data/mob_proto_server
echo "Adding to git"
git add ../data/item_proto_server
git add ../data/mob_proto_server