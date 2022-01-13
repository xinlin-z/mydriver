#!/usr/bin/bash

device=mychar

insmod ${device}.ko init_content_len=1024 || exit 1
rm -f ${device}-{3..6}
major=$(awk "\$2==\"${device}\" {print \$1}" /proc/devices)

for i in {3..6}; do
    mknod -m 666 ${device}-$i c ${major} $i 
done

