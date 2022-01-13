#!/usr/bin/bash

device=mychar
rm -f ${device}-{3..6}
rmmod $device
