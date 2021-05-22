

CFLAGS := -Wall `sdl2-config --cflags` -O3
CFILES := mandelbrot.c
LIBS := -lm `sdl2-config --libs`
TARGET := mandelbrot


all: $(CFILES)
	$(CC) $(CFLAGS) $(CFILES) -o $(TARGET) $(LIBS)


.PHONY: clean
clean:
	$(RM) $(TARGET)
