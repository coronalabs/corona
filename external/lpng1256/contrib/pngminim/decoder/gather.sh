cp ../../pngminus/png2pnm.c pngm2pnm.c
cp ../../../*.h .
cp ../../../*.c .
rm -f example.c pngtest.c pngpread.c pngw*.c pnggccrd.c pngvcrd.c
# change the following 2 lines if zlib is somewhere else
cp ../../../../zlib/*.h .
cp ../../../../zlib/*.c .
rm minigzip.c example.c compress.c deflate.c gz*
