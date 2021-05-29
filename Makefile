

CFLAGS := -Wall -Wextra -O3 `sdl2-config --cflags`
CFILES := mandelbrot.c
LIBS := -lm `sdl2-config --libs`
TARGET := mandelbrot


all: $(CFILES)
	$(CC) $(CFLAGS) $(CFILES) -o $(TARGET) $(LIBS)

emscripten: $(CFILES)
	mkdir -p wasm
	emcc -O3 $(CFILES) -s USE_SDL=2 -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1 -s LEGACY_GL_EMULATION=1 -o wasm/mandelbrot.html

.PHONY: clean
clean:
	$(RM) $(TARGET)
