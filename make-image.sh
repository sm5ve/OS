#! /bin/sh

if [ "$(id -u)" != 0]; then
	echo "requires root"
	exit 1
fi

qemu-img create hd.img 100m
mke2fs -q -I 128 hd.img

mkdir mnt
mount hd.img mnt

cleanup(){
	umount mnt
	rm -rf mnt
	echo "done"
}

trap cleanup EXIT
