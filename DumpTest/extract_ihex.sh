#!/bin/bash

grep ^: $1 > readback.hex
objcopy -I ihex -O binary readback.hex readback.bin

offset=`grep -Ubo --binary-file=text "TAR file starts here" readback.bin | grep -o "^[0-9]*"`

tail -c +$((offset+22)) readback.bin > readback.tar.xz

echo $((offset+22))


