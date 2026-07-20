.PHONY: web clean

TARGET = pong
SRCS = src/pong.c
CC = cc
EMCC = emcc

CFLAGS = -Wall -I./raylib
LDFLAGS = -L./raylib -lraylib.linux -lm -lpthread -ldl -lrt -lX11
WINLDFLAGS = -Llib -L./raylib -lraylib.win -lgdi32 -lwinmm

WEBLDFLAGS = -L./raylib -lraylib.web \
             -s USE_GLFW=3 \
             -s ASYNCIFY \
             -DPLATFORM_WEB \
             --shell-file web/shell.html

linux: $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

win: $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(WINLDFLAGS)

web: $(SRCS)
	$(EMCC) $(CFLAGS) -o web/game.html $(SRCS) $(WEBLDFLAGS)

clean:
	rm -f $(TARGET) web/game.html web/game.js web/game.wasm
