#!/bin/sh

SRC_BIN_FILE="../Debug/HUB.bin"
APP_BIN_FILE="APP.bin"
APP_CRC_FILE="APP.crc"

cp $SRC_BIN_FILE $APP_BIN_FILE
#objcopy -I binary -O binary --reverse-bytes=4 $SRC_BIN_FILE "tmp.bin"
java -jar ~/jacksum.jar -a crc32_mpeg2 -X -F "#CHECKSUM" -O $APP_CRC_FILE $APP_BIN_FILE

echo "Done."
