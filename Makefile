TARGET = pong
TARGET2 = pong2
SRCS = pong.c
SRCS2 = pong2.c
CFLAGS = -Wall -Werror
LDFLAGS = -L. -lraylib -lm -lX11

$(TARGET): $(SRCS)
	cc $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

$(TARGET2): $(SRCS2)
	cc $(CFLAGS) -o $(TARGET2) $(SRCS2) $(LDFLAGS)

web: pong2.c
	emcc -o game.html pong2.c -L. -lraylib.web \
    -s USE_GLFW=3 -s ASYNCIFY -DPLATFORM_WEB \
    --shell-file shell.html
