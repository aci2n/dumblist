CC = gcc
CFLAGS = -ggdb -Og
OUT = out/dumblist
LOG_LEVEL = 1000

headers = \
	src/log.h \
	src/server.h

sources = \
	src/main.c \
	src/server.c

defines = \
	-DLOG_LEVEL='$(LOG_LEVEL)'

.PHONY: build
build: $(sources) $(headers)
	$(CC) $(CFLAGS) $(defines) $(sources) $(headers) -o'$(OUT)'
