#!/bin/bash

if [ -z "$1" ]
then
	echo "No argument supplied."
	exit 1
else
	JAIL=$1
fi

mkdir -p $JAIL/{bin,usr,lib,lib64}
mount --bind -o ro /bin   $JAIL/bin
mount --bind -o ro /usr   $JAIL/usr
mount --bind -o ro /lib   $JAIL/lib
mount --bind -o ro /lib64 $JAIL/lib64

start=$(date +%s%N)
chroot $JAIL /usr/bin/sleep 1
end=$(date +%s%N)
echo "Chroot took $((((($end - $start)/1000)) - 1000000)) us"

umount $JAIL/bin
umount $JAIL/usr
umount $JAIL/lib
umount $JAIL/lib64
rm -r $JAIL
