.PHONY: clean

# Detect OS
OS := $(shell uname -s)

# Default compiler
CC := clang

# Common flags
CFLAGS := -Wall -std=c11 -Iinclude/ -Iinclude_ext/ -Ilib/raylib/src
LFLAGS := -Llib/raylib/src -lraylib -lGL -lGLU -lglut -lm -lpthread -ldl -lrt

# OS-specific settings
ifeq ($(OS), Darwin)  # macOS
    LFLAGS += -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
endif

ifeq ($(OS), Linux)
    LFLAGS += -lGL -lGLU -lglut -lm
endif

ifeq ($(OS), Windows_NT)  # Windows (Windows_NT is used in Make) wait wtf why would bash commands work here then
    CC := gcc  # Use gcc instead of clang on Windows
    LFLAGS += -lopengl32 -lgdi32 -lwinmm
endif

# raylib Build
lib/raylib/src/libraylib.a:
	$(MAKE) -C lib/raylib/src RAYLIB_LIBTYPE=STATIC

# Build main program (depends on Raylib)

build: lib/raylib/src/libraylib.a
	mkdir -p bin
	$(CC) $(CFLAGS) $(LFLAGS) \
		-o bin/quantum \
		src/quantumapp.c \
		src/solver.c \
		lib/hashmap.c \
		src/potential.c \
		src/guiconfig.c \
		src/simconfig.c \
		-lGL -lGLU -lglut -lm -lpthread -ldl -lrt -lraylib


run: build
	bin/quantum

scratch:
	clang -g -O0 -Wall src/solver.c src/hashmap.c tests/scratch.c -o bin/scratch -lm

test:
	clang -Wall src/solver.c tests/test.c src/hashmap.c -o bin/test -lm -lcriterion

clean:
	rm -rf bin lib/raylib/src/libraylib.a

debug: src/quantumapp.c src/solver.c src/potential.c src/guiconfig.c src/simconfig.c
	clang \
	-framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL \
	-Wall -std=c11 -Iinclude/ -L lib/ -lraylib -o bin/quantum -g \
	src/quantumapp.c src/solver.c lib/hashmap.c src/potential.c src/guiconfig.c src/simconfig.c
