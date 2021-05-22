

CFLAGS := -Wall -Wextra -O3 `sdl2-config --cflags` `pkg-config sdl2_gfx --cflags`
CFILES := mandelbrot.c
LIBS := -lm `sdl2-config --libs` `pkg-config sdl2_gfx --libs`
TARGET := mandelbrot


all: $(CFILES)
	$(CC) $(CFLAGS) $(CFILES) -o $(TARGET) $(LIBS)


.PHONY: clean
clean:
	$(RM) $(TARGET)
