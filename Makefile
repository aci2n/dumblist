CC = gcc
CFLAGS = -g -Og -Wall
OUT = out/dumblist
LOG_LEVEL = LEVEL_TRACE

headers = \
	src/log.h \
	src/server.h \
	src/handler.h \
	src/http.h \
	src/dumb.h \
	src/str.h

sources = \
	src/main.c \
	src/server.c \
	src/handler.c \
	src/http.c \
	src/dumb.c \
	src/str.c

defines = \
	-DLOG_LEVEL='$(LOG_LEVEL)'

.PHONY: build
build: $(sources) $(headers)
	$(CC) $(CFLAGS) $(defines) $(sources) $(headers) -o'$(OUT)'
