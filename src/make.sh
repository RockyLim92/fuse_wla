
gcc -W wlafs.c log.c params.h -o wlafs -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -lfuse
