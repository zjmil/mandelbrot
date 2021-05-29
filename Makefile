

CFLAGS := -Wall -Wextra -O3 `sdl2-config --cflags`
CFILES := mandelbrot.c
LIBS := -lm `sdl2-config --libs`
TARGET := mandelbrot

EMCC := emcc
EMFLAGS := -O3
EMOPTIONS := -s USE_SDL=2 -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1 -s LEGACY_GL_EMULATION=1
EMTARGETDIR := wasm
EMTARGET := $(EMTARGETDIR)/index.html


all: $(CFILES)
	$(CC) $(CFLAGS) $(CFILES) -o $(TARGET) $(LIBS)

emscripten-serve: emscripten
	python3 -m http.server --bind 127.0.0.1 -d $(EMTARGETDIR)

emscripten: $(EMTARGET)

$(EMTARGET): $(CFILES)
	mkdir -p $(EMTARGETDIR)
	emcc $(EMFLAGS) $(CFILES) $(EMOPTIONS) -o $(EMTARGET)

.PHONY: clean
clean:
	$(RM) -r $(TARGET) $(EMTARGETDIR)
