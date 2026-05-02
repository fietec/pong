.PHONY: web clean

TARGET = pong
SRCS = src/pong.c
CC = cc
EMCC = emcc

CFLAGS = -Wall -I./raylib
LDFLAGS = -L./raylib -lraylib -lm -lpthread -ldl -lrt -lX11

WEBLDFLAGS = -L./raylib -lraylib.web \
             -s USE_GLFW=3 \
             -s ASYNCIFY \
             -DPLATFORM_WEB \
             --shell-file web/shell.html

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

web: $(SRCS)
	$(EMCC) $(CFLAGS) -o web/game.html $(SRCS) $(WEBLDFLAGS)

clean:
	rm -f $(TARGET) web/game.html web/game.js web/game.wasm
